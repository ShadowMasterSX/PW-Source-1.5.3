#!/usr/bin/perl -w

BEGIN {
    # If you need to change anything in this script, it should only
    # be here, in this BEGIN block. See the README for more info.

    my $programdir = (($0 =~ m:^(.*/):)[0] || "./") . ".";
    eval "require '$programdir/cricket-conf.pl'";
    if (!$Common::global::gInstallRoot && -l $0) {
        eval {
            my $link = readlink($0);
            my $dir = (($link =~ m:^(.*/):)[0] || "./") . ".";
            require "$dir/cricket-conf.pl";
        }
    }
    eval "require '/usr/local/etc/cricket-conf.pl'"
        unless $Common::global::gInstallRoot;
    $Common::global::gInstallRoot ||= $programdir;
    $Common::global::gConfigRoot ||= 'cricket-config';
    $Common::global::isGrapher = 1;
}

use strict;
use lib "$Common::global::gInstallRoot/lib";
use CGI qw(fatalsToBrowser);
use Time::Local 'timelocal_nocheck';
use RRDs 1.000101;

use ConfigTree::Cache;

use Common::Version;
use Common::global;
use Common::Log;
use Common::Options;
use Common::Util;
use Common::Map;
use Common::HandleTarget;

	my $gQ = new CGI;
	fixHome($gQ);

	my %g_dsnamemap;
	my %g_dstypemap;

	my $target = $gQ->param("target");
	my $datasource = $gQ->param("datasource");
	my $resolution = $gQ->param("resolution");
	my $start = $gQ->param("start");
	my $end = $gQ->param("end");
	my $dsappend = $gQ->param("dsappend");
	my $suberea = $gQ->param("suberea");

	$target = $target . '/' . $dsappend if( length($dsappend) > 0 );
	$datasource = 'ds-' . $datasource if( substr($datasource,0,3) ne 'ds-' );
	$datasource = $datasource . '-' . $dsappend if( length($dsappend) > 0 );
	$target =~ s/suberea/$suberea/ if( length($suberea) > 0 );

	# const parameters
	my $datadir = '/home/cricket/cricket-data';

	# implied paramters
	my @temp = split( /\//, $target );
	my $object = $temp[@temp-1];

	CheckParameter();

	PrintSort();


# same as grapher.cgi fixHome
sub fixHome
{
    # brute force:
    # $Common::global::gCricketHome = '/path/to/cricket/home';
    # return;

    return if (defined($Common::global::gCricketHome) &&
               $Common::global::gCricketHome =~ /\//);

    my($sname) = $gQ->script_name();
    if ($sname =~ /\/~([^\/]*)\//) {
        my($username) = $1;
        my($home) = (getpwnam($username))[7];
        if ($home) {
            $Common::global::gCricketHome = $home;
            return;
        } else {
            Info("Could not find a home directory for user $username." .
                 "gCricketHome is probably not set right.");
        }
    } else {
        Info("Could not find a username in SCRIPT_NAME. " .
             "gCricketHome is probably not set right.");
    }
    # Last ditch effort... If all else fails, assume Cricket's home
    # is one directory up from grapher.cgi.
    $Common::global::gCricketHome ||= $Common::global::gInstallRoot . "/..";
}

sub initDSNameMap
{
	my $onefile = shift;
	my @temp = split( /[\/\.]/, $onefile );
	my $name = $gQ->param( 'target' );
	$name .= "/";
	$name .= $temp[@temp-2] if( @temp >= 2 );

	$Common::global::gCT = new ConfigTree::Cache;
	my $ct = $Common::global::gCT;
	$ct->Base($Common::global::gConfigRoot);
	$ct->Warn(\&Warn);

	if (! $ct->init()) {
	    Die("Failed to open compiled config tree from " .
	        "$Common::global::gConfigRoot/config.db: $!");
	}

    my($targRef) = $ct->configHash($name, 'target');

	my($ttype) = lc($targRef->{'target-type'});
	my($ttRef) = $ct->configHash($name, 'targettype',
								$ttype, $targRef);

	%g_dsnamemap = makeDSNameMap($ttRef->{'ds'});

	# makeDSTypeMap	
	my $dsname;
    foreach $dsname (split(/\s*,\s*/, $ttRef->{'ds'})) {
		my($dsRef) = $ct->configHash($name, 'datasource',
									lc($dsname), $targRef);
		$g_dstypemap{lc($dsname)} = $dsRef->{'rrd-ds-type'};
	}
}

# same as grapher.cgi makeDSNameMap
sub makeDSNameMap
{
    my($dslist) = @_;
    my($i) = 0;
    my($dsname, %dsnamemap);

    if ($Common::global::gLongDSName) {
        foreach $dsname (split(/\s*,\s*/, $dslist)) {
            $dsnamemap{lc($dsname)} = Common::Util::mungeDsName($dsname);
            $i++;
        }
    } else {
        foreach $dsname (split(/\s*,\s*/, $dslist)) {
            $dsnamemap{lc($dsname)} = "ds$i";
            $i++;
        }
    }

    return %dsnamemap;
}

sub CheckParameter
{
	if( not $target or not $object )
	{
		PrintError( "no target defined.(target=$target)" );
		exit(0);
	}
	if( not $datasource )
	{
		PrintError( "no datasource defined.(datasource=$datasource)" );
		exit(0);
	}
	if( $resolution ne 'now'
		and $resolution ne '5min' and $resolution ne '30min'
		and $resolution ne '1hour' and $resolution ne '2hour'
		and $resolution ne 'day' and $resolution ne 'week' )
	{
		PrintError( "no resolution defined or invalid resolution.(resolution=$resolution)" );
		exit(0);
	}
}

sub PrintError
{
	my $msg = shift;
	print $gQ->header('text/html');
	print "<html><head><title>Error.</title></head><body>Error: $msg </body></html>";
}

sub PrintSort
{
	print $gQ->header('text/html');

	print "<html><head><title>Query Result</title></head><body bgcolor=white><img src=/images/company-logo.jpg align=left><h1>&nbsp;</h1><br clear=left>";

	print "<h3>$object 's $datasource, the last $resolution</h3>";
	print "<PRE>Conditions:\n target = $target, object = $object, datasource = $datasource,\n resolution = $resolution, start = $start, end = $end</PRE><br>";
	print "<table border cellpadding=2 align=center width=600>";

	print "<tr><td align=center valign=center width=30%><h3>$object</h3></td><td align=center valign=center width=30%><h3>Value</h3></td><td align=center valign=center width=40%><h3>Update Time</h3></td></tr>";

	my $rrdres = 1;
	if( $resolution eq 'now' )		{	$rrdres = 1;		}
	elsif( $resolution eq '5min' )	{	$rrdres = 300;		}
	elsif( $resolution eq '30min' )	{	$rrdres = 1800;		}
	elsif( $resolution eq '1hour' )	{	$rrdres = 3600;		}
	elsif( $resolution eq '2hour' )	{	$rrdres = 7200;		}
	elsif( $resolution eq 'day' )	{	$rrdres = 86400;	}
	elsif( $resolution eq 'week' )	{	$rrdres = 7*86400;	}

	my @args = ('AVERAGE',"--resolution=$rrdres","--start=$start","--end=$end");

	my %values;
	my %times;

	my @rrdfiles = glob( $datadir . "$target/*.rrd" );
	initDSNameMap( $rrdfiles[0] ) if( @rrdfiles > 0 );
	my $dsname = $g_dsnamemap{lc($datasource)};
	my $dstype = $g_dstypemap{lc($datasource)};
	foreach( @rrdfiles )
	{
		my @temp = split( /[\/\.]/, $_ );
		my $thistarget = $temp[@temp-2] if( @temp >= 2 );

		my ($start_real,$step,$names,$data) = RRDs::fetch( $_, @args );

		my $dsidx = 0;
		for( my $i=0; $i<@$names; $i ++ )	{	$dsidx = $i if( $dsname eq $$names[$i] );	}

		$times{$thistarget} = $start_real;
		$values{$thistarget} = 0;
		for( my $k=0; $k<@$data; $k ++ )
		{
			my $line = $$data[$k];

			if( defined $$line[$dsidx] )
			{
				if( uc($dstype) eq 'COUNTER' )	{	$values{$thistarget} += $$line[$dsidx] * $step;	}
				else							{	$values{$thistarget} = $$line[$dsidx];	}

				$times{$thistarget} = $start_real + $k*$step;
			}
		}
	}

	my @keys_sort = sort { $values{$b} <=> $values{$a} } keys(%values);

	foreach( @keys_sort )
	{
		my $t = scalar localtime($times{$_});
		print "<tr><td>$_</td><td>";
		printf "%12.0f ", $values{$_};
		print "</td><td>$t</td></tr>";
	}

	print "</table>&nbsp;<br>";
	print "</body></html>";
}


