#!/usr/bin/perl -w
use strict;
use IO::File;
use Data::Dumper;

my $namespace = 'GNET';

parse( @ARGV > 0 ? $ARGV[0] : 'skill.og' );

sub parse
{
	open (F,"<" . shift);
	my @objects;
	my @lines;
	while (<F>) {
		if ($_ !~ m/^(bool=|char=|float=|int=|double=)/)
		{
			push( @lines ,$_ );
		}
	}
	close(F);
	#print @lines;
	for (@lines)
	{
		if ($_=~ m/(^@?[\w][_\d\w]+)=(.*)/)
		{
			my $objectname = $1;
			my $members=$2;
			
			push(@objects,$objectname);
		   	my @members= $members =~ m/<@?[\w][_\d\w]+,[\w][_\d\w]+>/g;
			generate_wrapper($objectname,@members);
#print '$object='.$objectname."\n";
#print '@members='. "@members" . "\n";
		}
		else
		{
			print "Invalid object definition: " . $_ ;
		}
	}
	generate_visitors(@objects);
	generate_stubs(@objects);
	print "Done!\n"
}

sub generate_wrapper
{
	my ($objectname,@members) = @_;
	my $array_obj=$objectname=~s/^@//g;
	my $includes = lc($objectname).'.h';
	my $classname=$objectname.'CO';
	my $flag = '__'.$namespace.'_'.uc($objectname).'_H';
	my $file = new IO::File('./' . 'enc_'. lc($objectname).'.h', O_TRUNC|O_WRONLY|O_CREAT);

	$file->print(<<EOF);
#ifndef $flag
#define $flag
#include <string>
#include "bridge.h"
	
#include "$includes"	
EOF
	print_include_files($file,@members);
	$file->print(<<EOF);
namespace $namespace
{
	class $classname : public CommonObject
	{
	private:
		$objectname* pObj;
EOF
	print_member_co($file,@members);
	$file->print(<<EOF);

		static Method_Map method_map_get,method_map_set;
EOF
	print_member_access_func($file,@members);
	
	$file->print(<<EOF);	
	public:
		$objectname* GetContent() { return pObj; }
		void SetContent($objectname* _pObj) { pObj=_pObj; }
		static $classname* Create()
		{
			$objectname* p=new $objectname();
			return new $classname(p);
		}	
		static void Destroy($classname* p)
		{
			delete p->GetContent();
			delete p;
		}
EOF
	print_constructor($file,$objectname,$classname,@members);

	$file->print(<<EOF);	
		virtual ~$classname() 
		{
EOF
	print_destructor($file,@members);
	$file->print(<<EOF);
		}

		template <typename _t>
		bool GetProperty(const std::string& p_name,_t & value)
		{
			Method_Map::iterator it=method_map_get.find(p_name);
			if (it == method_map_get.end()) return false;
			unary_function_t<_t,$classname>* pFunc = dynamic_cast< unary_function_t<_t,$classname>* >((*it).second);
			if (pFunc == NULL) return false;
			value = (*pFunc)(this);
			return true;
		}
		template <typename _t>
		bool SetProperty(const std::string& p_name,const _t& value)   
		{
			Method_Map::iterator it=method_map_set.find(p_name);
			if (it == method_map_set.end()) return false;
			binary_function_t<void,_t,$classname>* pFunc = dynamic_cast< binary_function_t<void,_t,$classname>* >((*it).second);
			if (pFunc == NULL) return false;
			(*pFunc)(this,value);
			return true;
		}	
		
		template <typename _t>
		bool GetArrayValue(const int index,_t& value)
		{
			Method_Map::iterator it=method_map_get.find("value");
			if (it == method_map_get.end()) return false;
			const_binary_function_t<_t,int,$classname>* pFunc2 = dynamic_cast< const_binary_function_t<_t,int,$classname>* >((*it).second);
			if (pFunc2 == NULL) return false;
			value = (*pFunc2)(this,index);
			return true;
		}
		template <typename _t>	
		bool SetArrayValue(const int index,const _t& value)
		{
			Method_Map::iterator it=method_map_set.find("value");
			if (it == method_map_set.end()) return false;
			binary_function_t<void,const pair_t<_t>&,$classname>* pFunc = dynamic_cast< binary_function_t<void,const pair_t<_t>&,$classname>* >((*it).second);
			if (pFunc == NULL) return false;
			(*pFunc)(this,pair_t<_t>(index,value));
			return true;
		}	
	
	}; //end of class $classname
}; // end of namespace	

#endif
EOF
}
sub print_include_files
{
	my ($file,@members) = @_;
	my $member_type;
	my $include_file;
	foreach(@members)
	{
		$_ =~ m/<(@?[\w][_\d\w]+),([\w][_\d\w]+)>/;
		$member_type=$2;
		if ($member_type !~ m/^(bool|char|float|int|double)/) #自定义类
		{
			$include_file='enc_'.lc($member_type).'.h';
			$file->print(<<EOF);
#include "$include_file"
EOF
		}	
	}
}

sub print_member_co
{
	my($file,@members) = @_;
	my $type;
	my $name;
	foreach(@members)
	{
		$_ =~ m/<(@?[\w][_\d\w]+),([\w][_\d\w]+)>/;
		$type=$2.'CO';
		$name=lc($1);
		if ($type !~ m/^(bool|char|float|int|double)/) #自定义类
		{
			$file->print(<<EOF);
		mutable $type*	$name;
EOF
		}	
	}
}

sub print_member_access_func
{
	my ($file,@members) = @_;
	foreach(@members)
	{
		$_ =~ m/<(@?[\w][_\d\w]+),([\w][_\d\w]+)>/;
		#get member name and type
		my $member_name = $1;
		my $type = $2;  
		if ($type !~ m/^(bool|char|float|int|double)/) #自定义类
		{
			my $wrapper_type=$type.'CO';
			if ($member_name =~ m/^@/)
			{	
				$member_name=~ s/@//g;
				$member_name=ucfirst($member_name);
				my $member_instance=lc($member_name);
				$file->print(<<EOF);	
		CommonObject* Get$member_name(int index) const 
		{ 
			if ($member_instance)
			{
				$member_instance->SetContent(pObj->Get$member_name(index));
				return $member_instance;
			}
			return $member_instance=new $wrapper_type(pObj->Get$member_name(index));
		}
		void  Set$member_name(const pair_t<CommonObject*>& p) const 
		{ 
			$wrapper_type* wrapper=dynamic_cast<$wrapper_type*>(p.value);
			if (wrapper != NULL) pObj->Set$member_name(p.index,wrapper->GetContent());
		}

EOF
			}
			else
			{
				$member_name=ucfirst($member_name);
				my $member_instance=lc($member_name);
				$file->print(<<EOF);
		CommonObject* Get$member_name() const 
		{
			if ($member_instance)
			{
				$member_instance->SetContent(pObj->Get$member_name());
				return $member_instance;
			}
			return $member_instance=new $wrapper_type(pObj->Get$member_name());
		}
		void Set$member_name(CommonObject* co)
		{
			$wrapper_type* wrapper=dynamic_cast<$wrapper_type*>(co);
			if (wrapper != NULL) pObj->Set$member_name(wrapper->GetContent());
		}
EOF
			}
		}
		else	#基本类
		{
			if ($member_name =~ m/^@/)
			{
				$member_name=~ s/@//g;
				$member_name=ucfirst($member_name);
				$file->print(<<EOF);
		$type Get$member_name(int index) const { return pObj->Get$member_name(index); }
		void  Set$member_name(const pair_t<$type>& p) { pObj->Set$member_name(p.index,p.value); }
EOF
			}
			else
			{
				$member_name=ucfirst($member_name);
				$file->print(<<EOF);
		$type Get$member_name() const { return pObj->Get$member_name(); }
		void  Set$member_name($type value) { pObj->Set$member_name(value); }
EOF
			}
		}	
	}
}
sub print_destructor
{
	my ($file,@members) = @_;
	foreach (@members)
	{
		$_ =~ m/<(@?[\w][_\d\w]+),([\w][_\d\w]+)>/;
		my $type = $2;
		my $name = lc($1);
		if ($type !~ m/^(bool|char|float|int|double)/) #自定义类
		{
#my $member_instance = lc($type);
			$file->print(<<EOF);
			delete $name;
EOF
		}
	}
}
sub print_constructor
{
	my ($file,$objectname,$classname,@members) = @_;
	$file->print("		$classname($objectname* _pObj) : pObj(_pObj)");
	foreach (@members)
	{
        $_ =~ m/<(@?[\w][_\d\w]+),([\w][_\d\w]+)>/;
        my $type=$2;
		my $name=lc($1);
        if ($type !~ m/^(bool|char|float|int|double)/) #自定义类
		{
			$file->print(",$name(NULL)");
		}
	}		
	$file->print(<<EOF);
	
		{
EOF
	print_method_map($file,"get",$classname,@members);
	print_method_map($file,"set",$classname,@members);
	$file->print(<<EOF);
		}	
EOF
}
sub print_method_map
{
	my($file,$method,$classname,@members) = @_;
	$file->print(<<EOF);
			if (method_map_$method.size() == 0)
			{
EOF
	foreach (@members)
	{
		my ($member_name, $type) = $_ =~ m/<(@?[\w][_\d\w]+),([\w][_\d\w]+)>/;
		$member_name =~ s/@//g;
		my $content='method_map_'."$method".'["'."$member_name".'"]=(memfunc_pointer_t)mem_fun_p(&'."$classname".'::'.ucfirst($method).ucfirst($member_name).');';
		$file->print(<<EOF);
				$content
EOF
#$file->print("			\n"."method_map_"."$method"."[\"$member_name\"]=(memfunc_pointer_t)mem_fun_p(&$classname::".ucfirst($method).ucfirst($member_name).");"."\n");
	}
	$file->print(<<EOF);
			}
EOF
}

sub generate_visitors
{
	my $flag = '__'.$namespace.'_VISITORS_H';
	my $file = new IO::File('./visitors.h', O_TRUNC|O_WRONLY|O_CREAT);
	$file->print(<<EOF);
#ifndef $flag
#define $flag
#include "bridge.h"
#include "array.h"
#include "objectgraph.hpp"
EOF
	print_visitor_include_files($file,@_);
	$file->print(<<EOF);
namespace $namespace
{	
	class OperationVisitor : public Visitor
	{
		CommonObject* co;
	public:	
		void visit(CommonObject* _co)
		{
			co=_co;
		}
		static CommonObject* CreateArray(int size,unsigned int type)
		{
			switch(type)
			{
				case BOOL_T: { return ArrayCO<bool>::Create(size); } 
				case CHAR_T: { return ArrayCO<char>::Create(size); }
				case INT_T: { return ArrayCO<int>::Create(size); } 
				case FLOAT_T: { return ArrayCO<float>::Create(size); } 
				case DOUBLE_T: { return ArrayCO<double>::Create(size); } 
				default:
					return NULL;			   
			}
		}
		static void DestroyArray(CommonObject* co,unsigned int type)
		{
			switch(type)
			{
				case BOOL_T: 
				{ 
					ArrayCO<bool>* p=dynamic_cast<ArrayCO<bool>*>(co);
					if (p!=NULL) ArrayCO<bool>::Destroy(p);
					break;
				}
				case CHAR_T: 
				{ 
					ArrayCO<char>* p=dynamic_cast<ArrayCO<char>*>(co);
					if (p!=NULL) ArrayCO<char>::Destroy(p);
					break;
				}
				case INT_T: 
				{ 
					ArrayCO<int>* p=dynamic_cast<ArrayCO<int>*>(co);
					if (p!=NULL) ArrayCO<int>::Destroy(p);
					break;
				}
				case FLOAT_T: 
				{ 
					ArrayCO<float>* p=dynamic_cast<ArrayCO<float>*>(co);
					if (p!=NULL) ArrayCO<float>::Destroy(p);
					break;
				}
				case DOUBLE_T: 
				{ 
					ArrayCO<double>* p=dynamic_cast<ArrayCO<double>*>(co);
					if (p!=NULL) ArrayCO<double>::Destroy(p);
					break;
				}
				default:
					return;			   
			}
		}

EOF
	print_visitor_obj_factory($file,@_);
	print_visitor_access_property($file,@_);
	print_visitor_access_array($file,@_);
	$file->print(<<EOF);
	};//end of class
}; //end of namespace

#endif
EOF
}

sub print_visitor_include_files
{
	my ($file,@objects) = @_;
	foreach (@objects)
	{
		my $include_file = 'enc_'.lc($_).'.h';
		$file->print(<<EOF);
#include "$include_file"
EOF
	}
}
sub print_visitor_obj_factory
{
	my ($file,@objects) = @_;
	###############
	#create object#
	###############
	$file->print(<<EOF);
		static CommonObject* CreateCO(unsigned int type,ObjectGraph& og)
		{
EOF
	foreach (@objects)
	{	
		my $classname = $_.'CO';
		$file->print(<<EOF);
			if (type==og.GetIdentifier("$_")) { return $classname\::Create(); }
EOF
	}
	$file->print(<<EOF);
			return NULL;
		}
EOF
	################
	#destroy object#
	################
	$file->print(<<EOF);
		static void DestroyCO(CommonObject* co)
		{
			if (co == NULL) return;
EOF
	foreach (@objects)
	{	
		my $classname = $_.'CO';
		$file->print(<<EOF);
			{
				$classname* pco=dynamic_cast<$classname*>(co);
				if (pco!=NULL) $classname ::Destroy(pco);
				return;
			}	
EOF
	}
	$file->print(<<EOF);
		}
EOF
}
sub print_visitor_access_property
{
	my ($file,@objects) = @_;
	my @method=('Get','Set');
	foreach (@method)
	{
		my $method=$_;
		$file->print(<<EOF);
		template<typename _t>
  		bool $method(const std::string& p_name,_t& value)
		{
			if (co == NULL) return false;
EOF
		foreach (@objects)
		{
			my $classname = $_.'CO';
			$file->print(<<EOF);
			{
				$classname* pco=dynamic_cast<$classname*>(co);
				if (pco!=NULL) return pco->$method\Property(p_name,value);
			}	
EOF
		}
		$file->print(<<EOF);
			{
				ArrayCO<int>* pco=dynamic_cast<ArrayCO<int>*>(co);
				if (pco!=NULL) return pco->$method\Property(p_name,value);
			}
			{
				ArrayCO<float>* pco=dynamic_cast<ArrayCO<float>*>(co);
				if (pco!=NULL) return pco->$method\Property(p_name,value);
			}
			return false;
		}
EOF
	}
}
sub print_visitor_access_array
{
	my ($file,@objects) = @_;
	my @method=('Get','Set');
	foreach (@method)
	{
		my $method=$_;
		$file->print(<<EOF);
		template<typename _t>
  		bool $method(const int index,_t& value)
		{
			if (co == NULL) return false;
EOF
		foreach (@objects)
		{
			my $classname = $_.'CO';
			$file->print(<<EOF);
			{
				$classname* pco=dynamic_cast<$classname*>(co);
				if (pco!=NULL) return pco->$method\ArrayValue(index,value);
			}	
EOF
		}
		$file->print(<<EOF);
			{
				ArrayCO<int>* pco=dynamic_cast<ArrayCO<int>*>(co);
				if (pco!=NULL) return pco->$method\ArrayValue(index,value);
			}
			{
				ArrayCO<float>* pco=dynamic_cast<ArrayCO<float>*>(co);
				if (pco!=NULL) return pco->$method\ArrayValue(index,value);
			}
			return false;
		}
EOF
	}
}
sub generate_stubs
{
	my @objects = @_;
	my $file = new IO::File('./stubs.cpp', O_TRUNC|O_WRONLY|O_CREAT);
	foreach (@objects)
	{
		my $include_file = 'enc_'.lc($_).'.h';
		$file->print(<<EOF);
#include "$include_file"
EOF
	}
	$file->print(<<EOF);
namespace $namespace
{
EOF
	foreach (@objects)
	{
		my $classname = $_.'CO';
		$file->print(<<EOF);
	CommonObject::Method_Map $classname\::method_map_get;
	CommonObject::Method_Map $classname\::method_map_set;
EOF
	}
	$file->print(<<EOF);
}; // end of namespace
EOF
}
