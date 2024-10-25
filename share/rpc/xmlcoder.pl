#!/usr/bin/perl -w

use IO::File;
use XML::DOM;
use Getopt::Std;

sub usage
{
	print <<END
Usage: xmlcoder.pl [-f file_name] [-o xmlcoderoutput] [-v xmlversionoutput] [-h] 
	-f use file_name as input file instead of rpcalls.xml
	-o generate code to file output instead of xmlcoder.h
	-h show help information
END
	;
	exit;
}

sub walkdata
{
	my ($name) = @_;
	die "xmlcoder: cannot find rpcdata $name" if !$rpcdatas{$name};
	my $rpcdata = $rpcdatas{$name};
	$used{$name} = $rpcdata;
	$types{$name} = $name;
	my $vars = $rpcdata->getElementsByTagName("variable");
	my $n = $vars->getLength;
	for (my $i = 0; $i < $n; $i++)
	{
		my $var = $vars->item($i);
		my $type = $var->getAttribute("type");
		$type =~ s/Vector$//;
		$type =~ s/std::vector<(.*)>/$1/;
		if($rpcdatas{$type})
		{
			walkdata($type);
		}
	}
}

getopts("hf:o:v:", \%opts);
&usage if($opts{h});
$rpcallxml = "rpcalls.xml";
$output = "xmlcoder.h";
$rpcallxml = $opts{f} if $opts{f};
$output = $opts{o} if $opts{o};

die "xmlcoder: cannot open $rpcallxml for reading" if !-e $rpcallxml;
%types = ("char" => "char", "short" => "short", "int" => "int", "int64_t" => "int64_t", 
	"unsigned char" => "char", "unsigned short" => "short", "unsigned int" => "int", 
	"float" => "float", "Octets" => "Octets"); 

my $parser = new XML::DOM::Parser;
my $doc = $parser->parsefile( $rpcallxml );
%rpcdatas = ();
%used = ();
my $app = $doc->getElementsByTagName('application')->item(0);
for ($app->getChildNodes)
{
	if($_->getNodeType==ELEMENT_NODE and $_->getNodeName eq 'rpcdata')
	{
		$rpcdatas{$_->getAttribute('name')} = $_;
	}
}
walkdata("GRoleData", \%rpcdatas, \@used);
walkdata("GTableDefinition", \%rpcdatas, \@used);

my $file  = new IO::File($output, O_TRUNC|O_WRONLY|O_CREAT);
print_header($file);
print_start($file);
while (my ($rpcname, $rpcdata) = each(%used)) 
{
	print_method($file, $rpcname, $rpcdata);
}
print_end($file);
undef $file;

#generate xmlversion.h
$output = "include/xmlversion.h";
$output = $opts{v} if $opts{v};
my $project = $app->getAttribute('project');
my $version = "0.0";
#open STAT, "cvs status $rpcallxml |";
#while(<STAT>)
#{
#	if($_ =~ m/Working revision.*(\d+\.\d+)/)
#	{
#		$version = $1;
#	}
#}
#close STAT;

$file  = new IO::File($output, O_TRUNC|O_WRONLY|O_CREAT);

$file->print(<<EOF);
#ifndef __XMLVERSION_H
#define __XMLVERSION_H

#define XMLVERSION "$project $version"

#endif
EOF

undef $file;
$doc->dispose;
undef $file;


sub print_method
{
	my ($output,$rpcname,$rpcdata) =  @_;
	$output->print("\tvoid append(const char* name, const $rpcname& x)\n\t{\n");
	$output->print("\t\tdata = data + \"<\" + name + \">\\n\";\n");

	my $vars = $rpcdata->getElementsByTagName('variable');
	my $n = $vars->getLength;
	for (my $i = 0; $i < $n; $i++)
	{
		my $var = $vars->item ($i);
		my $name = $var->getAttribute("name");
		my $type = $var->getAttribute("type");
		$type =~ s/Vector$//;
		$type =~ s/std::vector<(.*)>/$1/;
		$type =~ s/std::set<(.*)>/$1/;
		die "xmlcoder: cannot find type $type" if !$types{$type};
		my $cast = "";
		my $func= "append";
		$cast = "($types{$type})" if $types{$type} ne $type;
		$func = "append_string" if $name eq "name";
		$output->print("\t\t$func(\"$name\", $cast x.$name);\n");
	}
	$output->print("\t\tdata = data + \"</\" + name + \">\\n\";\n");
	$output->print("\t}\n");
}
sub print_header
{
	my ($output) =  @_;
	$output->print(<<EOF);
#ifndef __XMLCODER_H
#define __XMLCODER_H

#include <string>
#include <vector>
#include <set>
#include "rpcdefs.h"
EOF
	foreach my $key (keys %used)
	{
		$output->print("#include \"" . lc($key) . "\"\n");
	}
}

sub print_start
{
	my ($output) =  @_;
	$output->print(<<EOF);

namespace GNET
{

class XmlCoder
{
protected:
	std::string  data;
	Octets       buffer;
	std::set<unsigned short> entities; 
public:
	XmlCoder() : buffer(32) 
	{ 
		entities.insert(0);
		entities.insert(34);
		entities.insert(38);
		entities.insert(39);
		entities.insert(60);
		entities.insert(62);
	}
	const char * c_str() { return data.c_str(); }

	void append_header()
	{
		data = "<?xml version=\\"1.0\\"?>\\n";
	}
	void append_variable(const char* name, const char* type, const std::string& value)
	{
		data = data + "<variable name=\\"" + name + "\\" type=\\"" + type + "\\">" + value + "</variable>\\n";
	}
	const std::string toString(char x)
	{
		sprintf((char*)buffer.begin(), "%d", (int)x);
		return (char*)buffer.begin();
	}
	const std::string toString(short x)
	{
		sprintf((char*)buffer.begin(), "%d", (int)x);
		return (char*)buffer.begin();
	}
	const std::string toString(int x)
	{
		sprintf((char*)buffer.begin(), "%d", x);
		return (char*)buffer.begin();
	}
	const std::string toString(float x)
	{
		sprintf((char*)buffer.begin(), "%.9g", x);
		return (char*)buffer.begin();
	}
	const std::string toString(int64_t x)
	{
		sprintf((char*)buffer.begin(), "%lld", x);
		return (char*)buffer.begin();
	}
	const std::string toString(const Octets& x)
	{
		buffer.resize(x.size()*2+1);
		unsigned char* p = (unsigned char*)x.begin();
		char* out = (char*)buffer.begin();
		*out = 0;
		for(size_t i=0;i<x.size();++i,out+=2)
		{
			sprintf(out,"%02x", p[i]);
		}
		return (char*)buffer.begin();
	}
	void append_string(const char* name, const Octets& x)
	{
		std::string result;
		const unsigned short *p = (const unsigned short*)x.begin();
		for(size_t len = x.size()/2;len>0;len--,p++)
		{
			unsigned short c = *p;
			if(c>0x7F || entities.find(c)!=entities.end()) 
				result.append("&#" + toString(c) + ";");
			else
				result += (char)c;
		}
		append_variable(name,"Octets",result);
	}
	void append(const char* name, char x)
	{
		append_variable(name, "byte", toString(x));
	}
	void append(const char* name, short x)
	{
		append_variable(name, "short", toString(x));
	}
	void append(const char* name, int x)
	{
		append_variable(name, "int", toString(x));
	}
	void append(const char* name, float x)
	{
		append_variable(name, "float", toString(x));
	}
	void append(const char* name, int64_t x)
	{
		append_variable(name, "long", toString(x));
	}
	void append(const char* name, const Octets& x)
	{
		append_variable(name, "Octets", toString(x));
	}
	template<typename T>
	void append(const char* name, const std::vector<T> &x) 
	{
		typedef const std::vector<T> VECTOR;
		for( typename VECTOR::const_iterator i=x.begin(),e=x.end();i!=e;++i)
			append(name, *i);
	}
	template<typename T>
	void append(const char* name, const GNET::RpcDataVector<T> &x) 
	{
		typedef const GNET::RpcDataVector<T> VECTOR;
		for(typename VECTOR::const_iterator i=x.begin(),e=x.end();i!=e;++i)
			append(name, *i);
	}
	template<typename T>
	void append(const char* name, const std::set<T> &x)
	{
		for (typename std::set<T>::const_iterator i=x.begin(),e=x.end();i!=e;++i)
			append(name, *i);
	}
EOF
}

sub print_end
{
	my ($output) =  @_;
	$output->print(<<EOF);

};

};

#endif
EOF
}
