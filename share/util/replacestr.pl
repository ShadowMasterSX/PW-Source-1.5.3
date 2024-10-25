#!/usr/bin/perl

use strict;
use FileHandle;

if( @ARGV != 3 )
{
	print "usage: replacestr.pl filename_exp string_old string_new\n\n";
	exit(0);
}

my ($filename_exp, $src, $dest);
$filename_exp = $ARGV[0];
$src = $ARGV[1];
$dest = $ARGV[2];

my @files = glob( "$filename_exp" );
foreach( @files )
{
	my $filename = $_;
	my $filename_tmp = "$filename.tmp";

	my $file = new FileHandle;
	my $file_tmp = new FileHandle;
	my $success;
	my $count = 0;
	if( $file_tmp->open( ">$filename_tmp" ) )
	{
		if( $file->open( "<$filename" ) )
		{
			my $line;
			while( $line =  $file->getline() )
			{
				$count += ($line =~ s/$src/$dest/g);
				$file_tmp->print( $line );
			}
			$file->close();
			$success = 1;
		}
		$file_tmp->close();
	}

	if( $count > 0 )
	{
		my @args = ("mv", "-f", "$filename_tmp", "$filename");
		system(@args);
		print "replace file: $filename\n";
	}
	else
	{
		my @args = ("rm", "-f", "$filename_tmp");
		system(@args);
		print "error replace file: $filename\n" if not $success;
	}
}




