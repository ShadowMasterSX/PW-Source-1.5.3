#!/usr/bin/perl

my $levelto = 1;
my $levelfrom = 1;

$levelto = $ARGV[0] if(@ARGV>0);
$levelfrom = $ARGV[1] if(@ARGV>1);

my $exp = 0;
my $i;
for( $i=$levelfrom+1; $i<= $levelto; $i++ )
{
	$exp += 500*$i*$i;
}

print "The exp needed from $levelfrom to $levelto: $exp\n";

