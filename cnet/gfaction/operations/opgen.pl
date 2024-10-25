#!/usr/bin/perl -w
use strict;
use IO::File;
use XML::DOM;

my $namespace = 'GNET';

parse ( @ARGV > 0 ? $ARGV[0] : 'opgen.xml' );

sub parse
{
	my $parser = new XML::DOM::Parser;
	my $doc = $parser->parsefile( shift );
	my $app = $doc->getElementsByTagName('application')->item(0);
	
	my @context_nodes;
	my @operation_nodes;
	my @role_nodes;

	for ($app->getChildNodes)
	{
		next unless $_->getNodeType == ELEMENT_NODE;
		my $name = $_->getNodeName;
		push(@context_nodes,  $_) if $name eq 'contextobj';
		push(@operation_nodes,$_) if $name eq 'operation';
		push(@role_nodes,     $_) if $name eq 'role';
	}
	
	generate_context_obj(\@operation_nodes);
	generate_operation($_)		for (@operation_nodes);
	generate_operation_stubs(\@operation_nodes);
	generate_role(\@role_nodes);
	generate_ids(\@operation_nodes,\@role_nodes);
	$doc->dispose;
}

sub generate_context_obj
{
	my ($context_objs)=@_;
	my $file = new IO::File('factiondata.hxx', O_TRUNC|O_WRONLY|O_CREAT);
	my $flag = '__' . $namespace . '_' . uc("factiondata"). '_H';
	$file->print(<<EOF);
#ifndef $flag
#define $flag
#include "marshal.h"
namespace $namespace
{	
EOF
	for (@$context_objs)
	{
		my $name=$_->getAttribute('name').'_param';
		my $vars=$_->getElementsByTagName('variable');
		generate_a_contextobj_client($file,$name,$vars);
		generate_a_contextobj_server($file,$name,$vars);
	}
	$file->print(<<EOF);
}; //end of namespace
#endif	
EOF
}

sub generate_a_contextobj_client
{
	my ($file,$ctxname,$vars) = @_;
	$ctxname = $ctxname.'_ct';
	my $blfirst=1;
	my $blNoParam=1;
	$file->print(<<EOF);
	struct $ctxname
	{
EOF
	#generate member variables
	my $n = $vars->getLength;
	my $i=0;
	for ($i=0; $i<$n ; $i++)
	{
		my $var = $vars->item ($i);
		my $name= $var->getAttribute('name');
		my $type= $var->getAttribute('type');
		my $serversupply = $var->getAttribute('serversupply');
		if ($serversupply !~ m/^true/)
		{
		$file->print(<<EOF);
		$type $name;
EOF
		}
	}
	###############################
	# STEP1. generate constructors
	###############################
	$file->print("\t\t$ctxname(");
	$blfirst=1;
	$blNoParam=1;
	for ($i=0; $i<$n; $i++)
	{
		my $var = $vars->item ($i);
		my $name= $var->getAttribute('name');
		my $type= $var->getAttribute('type');
		my $default= $var->getAttribute('default');
		my $attr= $var->getAttribute('attr');
		my $serversupply = $var->getAttribute('serversupply');
		if ($serversupply !~ m/^true/)
		{
			$file->print(',') if($blfirst!=1);
			$blfirst=0 if ($blfirst==1);

			$file->print("const $type& _$name=$default") if ($attr =~ m/^ref/);
			$file->print("$type _$name=$default") unless ($attr =~ m/^ref/);
			$blNoParam=0;
		}
	}
	$file->print(") : ") if ($blNoParam==0);
	$blfirst=1;
	for ($i=0; $i<$n; $i++)
	{
		my $var = $vars->item ($i);
		my $name= $var->getAttribute('name');
		my $type= $var->getAttribute('type');
		my $serversupply = $var->getAttribute('serversupply');
		if ($serversupply !~ m/^true/)
		{
			$file->print(',') if($blfirst!=1);
			$blfirst=0 if ($blfirst==1);
			$file->print("$name(_$name)");
		}
	}
	$file->print(" { }\n");
	###################################
	# STEP2. generate create function
	###################################
	$file->print(<<EOF);
		$ctxname& Create(const Marshal::OctetsStream& os) 
		{
EOF
	for ($i=0; $i<$n; $i++)
	{
		my $var = $vars->item ($i);
		my $name= $var->getAttribute('name');
		my $serversupply = $var->getAttribute('serversupply');
		if ($serversupply !~ m/^true/)
		{
		$file->print(<<EOF);
			os >> $name;
EOF
		}
	}
	$file->print(<<EOF);
			return *this;
		}
EOF

	###################################
	# STEP3. generate marshal function
	###################################
	$file->print(<<EOF);
		Marshal::OctetsStream marshal()
		{
EOF
	$blfirst=1;
	$blNoParam=1;
	for ($i=0; $i<$n; $i++)
	{
		my $var = $vars->item ($i);
		my $name= $var->getAttribute('name');
		my $serversupply = $var->getAttribute('serversupply');
		$file->print("\t\t\treturn Marshal::OctetsStream()") if ($blfirst==1);
		$blfirst=0;
		if ($serversupply !~ m/^true/)
		{
		$file->print("<<$name");
		$file->print(";\n") if ($i==$n-1);
		$blNoParam=0;
		}
	}
	$file->print(';\n') if ($blNoParam==1);

	$file->print(<<EOF);
		}
	};
EOF
}
sub generate_a_contextobj_server
{
	my ($file,$ctxname,$vars) = @_;
	my $ctxserver = $ctxname.'_st';
	my $ctxclient = $ctxname.'_ct';
	my $blfirst=1;
	my $blNoParam=1;
	##########################
	# 1. create server struct
	##########################
	$file->print(<<EOF);
	struct $ctxserver
	{
EOF
	#step1. generate member variables
	my $n = $vars->getLength;
	my $i=0;
	for ($i=0; $i<$n ; $i++)
	{
		my $var = $vars->item ($i);
		my $name= $var->getAttribute('name');
		my $type= $var->getAttribute('type');
		my $serversupply = $var->getAttribute('serversupply');
		$file->print(<<EOF);
		$type $name;
EOF
	}
	#step2. generate constructors, using all parameters
	$file->print("\t\t$ctxserver(");
	for ($i=0; $i<$n; $i++)
	{
		my $var = $vars->item ($i);
		my $name= $var->getAttribute('name');
		my $type= $var->getAttribute('type');
		my $default= $var->getAttribute('default');
		my $attr= $var->getAttribute('attr');
		my $serversupply = $var->getAttribute('serversupply');
		$file->print("const $type& _$name=$default") if ($attr =~ m/^ref/);
		$file->print("$type _$name=$default") unless ($attr =~ m/^ref/);
		
		$file->print(",") if ($i!=$n-1);
		$file->print(") : ") if ($i==$n-1);
	}
	for ($i=0; $i<$n; $i++)
	{
		my $var = $vars->item ($i);
		my $name= $var->getAttribute('name');
		my $type= $var->getAttribute('type');
		my $serversupply = $var->getAttribute('serversupply');
		$file->print("$name(_$name)");
		
		$file->print(",") if ($i!=$n-1);
		$file->print(" { }\n") if ($i==$n-1);
	}
	#step3. generate the constructor, using client_struct
	$file->print("\t\t$ctxserver(");
	$blNoParam=1;
	for ($i=0; $i<$n; $i++)
	{
		my $var = $vars->item ($i);
		my $name= $var->getAttribute('name');
		my $type= $var->getAttribute('type');
		my $default= $var->getAttribute('default');
		my $attr= $var->getAttribute('attr');
		my $serversupply = $var->getAttribute('serversupply');
		if ($serversupply =~ m/^true/)
		{
			$file->print("const $type& _$name") if ($attr =~ m/^ref/);
			$file->print("$type _$name") unless ($attr =~ m/^ref/);
			$file->print(",");
			$blNoParam=0;
		}
	}
	$file->print('const '.$ctxclient.'& _ct)');
	$file->print(' : ') if ($blNoParam==0);
	
	$blfirst=1;
	for ($i=0; $i<$n; $i++)
	{
		my $var = $vars->item ($i);
		my $name= $var->getAttribute('name');
		my $type= $var->getAttribute('type');
		my $serversupply = $var->getAttribute('serversupply');
		if ($serversupply =~ m/^true/)
		{
			$file->print(",") if ($blfirst!=1);
			$blfirst=0 if ($blfirst==1);
			$file->print("$name(_$name)");
		}
	}
	$file->print("{\n");
	for ($i=0; $i<$n; $i++)
	{
		my $var = $vars->item ($i);
		my $name= $var->getAttribute('name');
		my $serversupply = $var->getAttribute('serversupply');
		if ($serversupply !~ m/^true/)
		{
			$file->print(<<EOF);
			$name=_ct.$name;
EOF
		}
	}
	
	$file->print(<<EOF);
		}
EOF
	#step4. generate create function
	$file->print(<<EOF);
		void Create(const Marshal::OctetsStream& os) 
		{
EOF
	for ($i=0; $i<$n; $i++)
	{
		my $var = $vars->item ($i);
		my $name= $var->getAttribute('name');
		$file->print(<<EOF);
			os >> $name;
EOF
	}
	$file->print(<<EOF);
		}
EOF
	#step5. generate marshal function
	$file->print(<<EOF);
		Marshal::OctetsStream marshal()
		{
EOF
	$blfirst=1;
	$blNoParam=1;
	for ($i=0; $i<$n; $i++)
	{
		my $var = $vars->item ($i);
		my $name= $var->getAttribute('name');
		$file->print("\t\t\treturn Marshal::OctetsStream()") if ($blfirst==1);
		$blfirst=0;
		$file->print("<<$name");
		$file->print(";\n") if ($i==$n-1);
		$blNoParam=0;
	}
	$file->print(';\n') if ($blNoParam==1);
	$file->print(<<EOF);
		}
	};
EOF
}


sub generate_operation
{
	generate_oper_inl(@_);
	generate_oper_h(@_);
}
sub generate_oper_inl
{
	my ($operation)=@_;
	my $opname = $operation->getAttribute('name');
	my $optype = '_O_FACTION_'.uc($opname);
	my $param_name=$opname.'_param_st';
	my $classname = 'Op'.ucfirst($opname);
	my $needsync = $operation->getAttribute('needsync');
	my $needaddinfo = $operation->getAttribute('needaddinfo');
	my $ctxobjs = $operation->getElementsByTagName('context');
	my $file = new IO::File('op'.$opname.'.inl', O_TRUNC|O_WRONLY|O_CREAT);
	
	$needsync = 'false' if (! $needsync=~m/^true/);
	$needaddinfo = 'false' if (! $needaddinfo=~m/^true/);
	
	$file->print(<<EOF);
private:    
	GFactionServer::Player player;
	$param_name params;
	bool PrepareContext()
	{ 
		if (pWrapper==NULL) return false;
		oper_roleid=pWrapper->GetOperator();
		params.Create(pWrapper->GetParams());
		return true;
	}
	bool PrivilegeCheck() 
	{
		if (!GFactionServer::GetInstance()->GetPlayer(oper_roleid,player)) player.froleid=_R_UNMEMBER;
		//printf("op$opname: player(%d)'s faction role is %d\\n",oper_roleid,player.froleid);           
		return PrivilegePolicy((GNET::Roles)player.froleid);
	}
EOF
	$file->print(<<EOF);
public:
	$classname() : Operation($optype) { }
	~$classname() {} 
	$classname* Clone() { return new $classname(); }
	
	bool NeedSync() { return $needsync; }
	bool NeedAddInfo() { return $needaddinfo; }

	int oper_roleid;
EOF
}

sub generate_oper_h
{
	my ($operation)=@_;
	my $opname = $operation->getAttribute('name');
	my $optype = '_O_FACTION_'.uc($opname);
	my $classname = 'Op'.ucfirst($opname);
	my $flag = '__'.$namespace.'_OP'.uc($opname).'_H';
	
	my $inl_filename = 'op'.$opname.'.inl';
	my $file = new IO::File('op'.$opname.'.h',  O_WRONLY|O_CREAT|O_EXCL) or return;
	$file->print(<<EOF);
#ifndef $flag
#define $flag
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"

namespace $namespace
{

class $classname : public Operation
{
#include "$inl_filename"
private:
	bool ConditionCheck() { return true; }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
			return retcode;
		//TODO: do operation here
		return retcode;
	}
	void SetResult(void* pResult)
	{
	}
};

};//end of namespace
#endif
EOF
}

sub generate_operation_stubs
{
	my ($operation_nodes) = @_;
	my $file = new IO::File('operation.cxx', O_TRUNC|O_WRONLY|O_CREAT);
	#generate include files
	for (@$operation_nodes)
	{
		my $name=$_->getAttribute('name');
		my $includefile='op'.$name.'.h';
		$file->print(<<EOF);
#include "$includefile"
EOF
	}
	#generate stubs
	$file->print(<<EOF);
namespace $namespace
{
EOF
	for (@$operation_nodes)
	{
		my $name=$_->getAttribute('name');
		my $classname='Op'.ucfirst($name);
		my $stubname='__op'.$name.'_stub';
		$file->print(<<EOF);
	static $classname $stubname;
EOF
	}
	$file->print(<<EOF);
};//end of namespace	
EOF
}

sub	generate_role
{
	my ($role_nodes)=@_;
	my $file=new IO::File('privilege.cxx', O_TRUNC|O_WRONLY|O_CREAT);
	$file->print(<<EOF);
#include "privilege.h"
namespace $namespace
{	
EOF
	for (@$role_nodes)
	{
		my $rolename= $_->getAttribute('name');
		my $opers= $_->getElementsByTagName('operation');
		generate_privilege($file,$rolename,$opers);
	}
	$file->print(<<EOF);
}; //end of namespace	
EOF
}

sub generate_privilege
{
	my ($file,$rolename,$opers) = @_;
	my $opers_array_name = '__'.uc($rolename).'_OPERATIONS';
	my $n=$opers->getLength;
	my $i=0;
	#generate operation array
	$file->print(<<EOF);
	static Operations $opers_array_name []={
EOF
	for ($i=0;$i<$n;$i++)
	{
		my $op=$opers->item($i);
		my $opname=$op->getAttribute('ref');
		my $optype='_O_FACTION_'.uc($opname);
		$file->print(<<EOF);
		$optype,
EOF
	}
	$file->print(<<EOF);
	}; 
EOF
	#generate privilege obj
	my $roletype='_R_'.uc($rolename);
	my $privilege_name='__'.$rolename.'_privilege';
	$file->print(<<EOF);
	static Privilege $privilege_name($roletype,$opers_array_name,sizeof($opers_array_name)/sizeof(Operations));
EOF
}

sub generate_ids
{
	my($operation_nodes,$role_nodes) = @_;
	my $file=new IO::File('ids.hxx', O_TRUNC|O_WRONLY|O_CREAT);
	my $flag = '__'.$namespace.'_IDS_H';
	$file->print(<<EOF);
#ifndef $flag
#define	$flag

namespace $namespace
{
	
EOF
	#generate role types
	$file->print(<<EOF);
	enum Roles {
EOF
	for (@$role_nodes)
	{
		my $rolename= $_->getAttribute('name');
		my $typeid=$_->getAttribute('type');
		my $roletype= '_R_'.uc($rolename);
		$file->print(<<EOF);
		$roletype = $typeid,
EOF
	}
	$file->print(<<EOF);
	}; //end of Roles
	
EOF
	#generate operation types
	$file->print(<<EOF);
	enum Operations {
EOF
	for (@$operation_nodes)
	{
		my $opname= $_->getAttribute('name');
		my $typeid= $_->getAttribute('type');
		my $optype= '_O_FACTION_'.uc($opname);
		$file->print(<<EOF);
		$optype = $typeid,
EOF
	}
	$file->print(<<EOF);
	}; //end of Operations
	
EOF
	
	$file->print(<<EOF);
}; //end of namespace	

#endif
EOF
}
