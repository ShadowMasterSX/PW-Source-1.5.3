#!/usr/bin/perl -w

use strict;
use CGI qw(fatalsToBrowser);

my $gQ = new CGI;

my $rebuild = $gQ->param("rebuild");
my $rep = $gQ->param("rep");
if( not $rep )
{
	print $gQ->header('text/html');
	print "<html><head><title>Error.</title></head><body>Error: no repository is defined.</body></html>";
}
elsif( $rebuild eq 'true' )
{
	print $gQ->header('text/html');
	print "<html><body><pre>";

	my $curdir = qx/pwd/;
	chop($curdir);

	my @cmd;
	my $exe;

	print qx/mkdir tmp/;
	chdir( "tmp" );
	$exe = "cvs -d :pserver:sunzhenyu\@172.16.2.1:/usr/cvsroot co $rep";
	print qx/$exe/;
	chdir( $rep );
	$exe = "cvs -d :pserver:sunzhenyu\@172.16.2.1:/usr/cvsroot log > $rep.cvslog";
	print qx/$exe/;
	chdir( $curdir );
	print qx/mkdir $rep/;
	chdir( $rep );
	$exe = "/usr/java/j2sdk1.4.2_05/bin/java -jar ../statcvs.jar ../tmp/$rep/$rep.cvslog ../tmp/$rep";
	print qx/$exe/;
	chdir( $curdir );

	print "</pre>&nbsp;<br>&nbsp;&nbsp;<a href=\"./$rep\">View</a><br>&nbsp;<br></body></html>";
}
else
{
	print $gQ->redirect($rep);
}

