private:    
	GFactionServer::Player player;
	create_param_st params;
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
		//printf("opcreate: player(%d)'s faction role is %d\n",oper_roleid,player.froleid);           
		return PrivilegePolicy((GNET::Roles)player.froleid);
	}
public:
	OpCreate() : Operation(_O_FACTION_CREATE) { }
	~OpCreate() {} 
	OpCreate* Clone() { return new OpCreate(); }
	
	bool NeedSync() { return true; }
	bool NeedAddInfo() { return true; }

	int oper_roleid;
