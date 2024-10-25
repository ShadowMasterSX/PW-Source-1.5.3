#!/usr/bin/perl -w

use strict;
use CGI qw(fatalsToBrowser);

my $gQ = new CGI;

my $count = 0;
$count = $gQ->param("count");
$count = 0 if not $count;
my $filename;
$filename = $gQ->param("file");
$filename = "messages" if not $filename;

if( $filename ne 'messages' and $filename ne 'world2.log'
	and $filename ne 'statinfom' and $filename ne 'statinfoh'
	and $filename ne 'statinfod' and $filename ne 'secure'
	and $filename ne 'maillog' and $filename ne 'cron'
	and $filename ne 'spooler' and $filename ne 'boot.log' )
{
	print $gQ->header('text/html');
	print "<html><head><title>Error.</title></head><body>Error: no this log file.</body></html>";
}
elsif( $count < 1 or $count > 1000 )
{
	print $gQ->header('text/html');
	print "<html><head><title>Error.</title></head><body>Error: count is too large.</body></html>";
}
else
{
	print $gQ->header('text/plain');
	my @tail = ("/usr/bin/tail", "-n", "$count", "/var/log/$filename");
	system( @tail );
}

