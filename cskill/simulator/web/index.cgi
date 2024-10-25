#!/usr/bin/perl -w
     
use CGI;
use CGI::Carp qw/fatalsToBrowser/;
 
$query = new CGI;

print $query->header( -type=>'text/html', -charset=>'gb2312');
print $query->start_html("技能测试");
print "<H2>技能测试</H2>\n";
open CONFIG, "logs/config";
my ($level, $hp, $mp, $attack, $magic, $range, $target) = split(/ /,<CONFIG>);
close CONFIG;

&print_prompt($query);
&print_table;
&do_work($query);
my @types = ('count lines','count words','count characters');

print $query->end_html;
 
sub print_prompt {
   my($query) = @_;
   print $query->start_form;
   print "玩家级别";
   print $query->textfield( -name=>'level',-default=>$level, -size=>10);
   print "&nbsp;&nbsp;玩家生命";
   print $query->textfield( -name=>'hp',-default=>$hp, -size=>10);
   print "&nbsp;&nbsp;玩家魔法";
   print $query->textfield( -name=>'mp',-default=>$mp, -size=>10);
   print "<BR>物理攻击";
   print $query->textfield( -name=>'attack',-default=>$attack, -size=>10);
   print "&nbsp;&nbsp;魔法攻击";
   print $query->textfield( -name=>'magic',-default=>$magic, -size=>10);
   print "&nbsp;&nbsp;默认射程";
   print $query->textfield( -name=>'range',-default=>$range, -size=>10);
   print "<BR><BR>目标级别";
   print $query->textfield( -name=>'target',-default=>$target, -size=>10);
   print "<BR><BR>";
   print $query->submit('Action','Update');
#   print "<BR><BR><BR><H3>重新生成代码:<BR></H3>";
#   print $query->submit('Action','Generate');
#   print "<font color=red>(别乱点，很慢的说)</font>";
#   if(-e "ElementSkill.dll")
#   {
#	   print "<a href=\"http://172.16.2.2/skill/ElementSkill.dll\">Download</a>"
#   }
   print $query->endform;
   print "<HR>\n";
 	}
 
sub do_work {
    my($query) = @_;
    my($value,$key);
	
	if($query->param("Action") eq "Update")
	{
		open OUTPUT, ">logs/config";
		print OUTPUT join(' ',$query->param("level"), $query->param("hp"), $query->param("mp"), 
			$query->param("attack"), $query->param("magic"), $query->param("range"), $query->param("target"));
		close OUTPUT;
		my $exe = "/home/cricket/cricket-html/skill/simulator ./logs/config"; 
		print qx/$exe/;
	}elsif($query->param("Action") eq "Generate")
	{
		print "别急，别急，喝口水......";
	}
}
 
sub print_table {
	print <<END;
<script language=javascript>
function showlog( logfile )
{
	document.showlogs.file.value = logfile;
	document.showlogs.submit();
}
</script>

<form method="post" action="./patch.cgi" name="showlogs">
<input type="hidden" name="file" value="messages">
<table border cellpadding=2 width=100%>
<tr><th align=left width=30%>Name</th>    <th align=left>Description</th></tr>
<tr><td><a href="javascript:showlog('run_min.log')">最低级执行效果</a></td><td>&nbsp;</td></tr>
<tr><td><a href="javascript:showlog('run_max.log')">最高级执行效果</a></td><td>&nbsp;</td></tr>
<tr><td><a href="javascript:showlog('learn.log')">技能学习统计信息</a></td><td>&nbsp;</td></tr>
<tr><td><a href="javascript:showlog('depend.log')">相关技能</a></td><td>&nbsp;</td></tr>
<tr><td><a href="javascript:showlog('attack_max.log')">最高级攻击效果</a></td><td>&nbsp;</td></tr>
<tr><td><a href="javascript:showlog('attack_min.log')">最低级攻击效果</a></td><td>&nbsp;</td></tr>
<tr><td><a href="javascript:showlog('error.log')">警告信息</a></td><td>&nbsp;</td></tr>
</table>
</form>
END
    ;
}
