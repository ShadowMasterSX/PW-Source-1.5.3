package Game::MPROCD;

use strict;
use POSIX qw (:sys_wait_h :unistd_h);
use IO::Pipe;
use IO::Socket;
use constant SCOREBOARD_MAINTENANCE_INTERVAL => 1;

my $Config;
my $ScoreBoard;
my $MainSock;

sub import
{
	my ($pkg, %param) = @_;
	$Config = \%param;

	exit unless (exists $Config->{ListenPort});
	$Config->{MinServer} = 5		unless (exists $Config->{MinServer});
	$Config->{MaxServer} = 10		unless (exists $Config->{MaxServer});
	$Config->{SpareServer} = 5		unless (exists $Config->{SpareServer});
	$Config->{MaxService} = 10		unless (exists $Config->{MaxService});
	$Config->{InitServer} = sub {}		unless (exists $Config->{InitServer});
	$Config->{DestroyServer} = sub {}	unless (exists $Config->{DestroyServer});
	$Config->{ListenBackLog} = 5 		unless (exists $Config->{ListenBackLog});
	$Config->{ServiceTimeOut} = 60 		unless (exists $Config->{ServiceTimeOut});
	$Config->{MaxServer} = $Config->{MinServer} + 1 if ($Config->{MinServer} > $Config->{MaxServer});
}

my ($reader, $writer);
sub Server
{
	($reader, $writer) = @_;
	my $Broken_Pipe = 0;
	$Config->{InitServer}->();
	for(my $i = 0; $i < $Config->{MaxService}; $i++)
	{
		my $sock = $MainSock->accept;
		redo unless defined $sock;
		$writer->syswrite("R", 1);
		eval
		{
			local ($SIG{ALRM}, $SIG{PIPE}) = (sub { die "TimeOut" }, sub { $Broken_Pipe = 1; die "Pipe"; });
			alarm ($Config->{ServiceTimeOut});
			$Config->{Server}->($sock, $Config->{ServiceTimeOut});
			alarm (0);
			$@ && die $@;
		};
		$writer->syswrite("S", 1);
		last if $Broken_Pipe;
	}	
	$Config->{DestroyServer}->();
	exit;
}

sub CreateServer
{
	my @pipe = (new IO::Pipe, new IO::Pipe);
	my $pid = fork;

	if (defined $pid)
	{
		if ($pid == 0)
		{
			$ScoreBoard = undef;
			$pipe[0]->reader;
			$pipe[1]->writer;
			$pipe[1]->autoflush;
			Server(@pipe);
		} else {
			$pipe[0]->writer;
			$pipe[1]->reader;
			$pipe[0]->autoflush;
			$ScoreBoard->{$pid}->{status} = 'S';
			$ScoreBoard->{$pid}->{writer} = $pipe[0];
			$ScoreBoard->{$pid}->{reader} = $pipe[1];
		}
		return $pid;
	} 
	return undef;
}

sub Daemonize
{
	chdir '/';
	open STDIN , "/dev/null";
	open STDOUT, "/dev/null";
	open STDERR, "/dev/null";
	exit if fork;
	setsid;
	exit if fork;
}

sub Run
{
	Daemonize;
	local $SIG{PIPE} = 'IGNORE';

	$MainSock = new IO::Socket::INET (LocalPort => $Config->{ListenPort}, Listen => $Config->{ListenBackLog}, Proto => 'tcp', Reuse => 1);
	for(;;)
	{
		my ($rset, $wset, $eset) = ('', '', '');
		foreach (keys %$ScoreBoard)
		{
			vec($rset, fileno($ScoreBoard->{$_}->{reader}), 1) = 1;
		}
		my ($nset, undef) = select ($rset, $wset, $eset, SCOREBOARD_MAINTENANCE_INTERVAL);
		while ((my $pid = waitpid(-1, WNOHANG)) > 0)
		{
			if (exists $ScoreBoard->{$pid})
			{
				$ScoreBoard->{$pid}->{reader}->close;
				$ScoreBoard->{$pid}->{writer}->close;
				delete $ScoreBoard->{$pid};
			}
		}
		foreach (keys %$ScoreBoard)
		{
			my $handle = $ScoreBoard->{$_}->{reader};
			if (vec($rset, fileno($handle), 1))
			{
				my $status;
				$handle->sysread($status, 1);
				if ($status eq 'S')
				{
					$ScoreBoard->{$_}->{status} = 'S';
				} elsif ($status eq 'R')
				{
					$ScoreBoard->{$_}->{status} = 'R';
				} 
			}
		}
		my ($idle_server, $busy_server, $total_server) = (0, 0, 0);
		foreach (keys %$ScoreBoard)
		{
			my $status = $ScoreBoard->{$_}->{status};
			if ($status eq 'S')
			{
				$idle_server++;
			} elsif ($status eq 'R')
			{
				$busy_server++;
			}
		}
		$total_server = $idle_server + $busy_server;
		if ($total_server < $Config->{MinServer})
		{
			CreateServer;
		} elsif ($total_server < $Config->{MaxServer})
		{
			if ($idle_server < $Config->{SpareServer})
			{
				CreateServer;
			}
		}
	}
}

1;
