#!/usr/bin/perl -w

use strict;
use CGI qw(fatalsToBrowser);

my $gQ = new CGI;

my $filename;
$filename = $gQ->param("file");
$filename = "messages" if not $filename;

if( $filename eq 'run_max.log')
{
	print $gQ->header( -type=>'text/plain', -charset=>'gbk');
	print "编号   名称          职业 总时间 冷却 执行    类型    吟唱距离 有效距离 半径  物理伤害 魔法伤害 消耗MP\n";
	my $cmd = ("export LC_ALL=zh_CN && cat logs/$filename | sort -k 3");
	system( $cmd );
}
elsif( $filename eq 'run_min.log')
{
	print $gQ->header( -type=>'text/plain', -charset=>'gbk');
	print "编号   名称          职业 总时间 冷却 执行    类型    吟唱距离 有效距离 半径  物理伤害 魔法伤害 消耗MP\n";
	my $cmd = ("export LC_ALL=zh_CN && cat logs/$filename | sort -k 3");
	system( $cmd );
}
elsif( $filename eq 'learn.log')
{
	print $gQ->header( -type=>'text/plain', -charset=>'gbk');
	print "编号   名称             职业  开始  结束  最大级别  SP总计  前提技能\n";
	my $cmd = ("export LC_ALL=zh_CN && cat logs/$filename | sort -k 3");
	system( $cmd );
}
elsif( $filename eq 'error.log')
{
	#print $gQ->header;
	print $gQ->header( -type=>'text/html', -charset=>'gbk');
	print $gQ->start_html("Warning");
	open ERROR, "logs/$filename";
	while(<ERROR>)
	{
		if($_ =~ m/!!!/)
		{
			print "<font color=red>";
			print $_;
			print "</font>";
		}
		elsif($_ =~ m/!!/)
		{
			print "<font color=brown>";
			print $_;
			print "</font>";
		}	
		else
		{
			print $_;
		}
		print "<BR>";
	}
	close ERROR;
	print $gQ->end_html;
}
else
{
	print $gQ->header( -type=>'text/plain', -charset=>'gbk');
	my $cmd = ("cat logs/$filename");
	system( $cmd );
}

