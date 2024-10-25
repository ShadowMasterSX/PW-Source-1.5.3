#!/usr/bin/perl -w

use IO::File;
use XML::DOM;
use Getopt::Std;

sub usage
{
	print <<END
Usage: xmlversion.pl [-f file_name] [-o output] [-h] 
	-f use file_name as input file instead of rpcalls.xml
	-o generate code to file output instead of xmlversion.h
	-h show help information
END
	;
	exit;
}

getopts("hf:o:", \%opts);
&usage if($opts{h});
$rpcallxml = "rpcalls.xml";
$output = "xmlversion.h";
$rpcallxml = $opts{f} if $opts{f};
$output = $opts{o} if $opts{o};

die "xmlversion: cannot open $rpcallxml for reading" if !-e $rpcallxml;
my $parser = new XML::DOM::Parser;
my $doc = $parser->parsefile( $rpcallxml );
my $app = $doc->getElementsByTagName('application')->item(0);
my $project = $app->getAttribute('project');
$doc->dispose;

my $version = "0.0";
open STAT, "cvs status $rpcallxml |";
while(<STAT>)
{
	if($_ =~ m/Working revision.*(\d+\.\d+)/)
	{
		$version = $1;
	}
}
close STAT;

my $file  = new IO::File($output, O_TRUNC|O_WRONLY|O_CREAT);

$file->print(<<EOF);
#ifndef __XMLVERSION_H
#define __XMLVERSION_H

#define XMLVERSION "$project $version"

#endif
EOF

undef $file;
