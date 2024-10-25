private:    
	GFactionServer::Player player;
	resign_param_st params;
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
		//printf("opresign: player(%d)'s faction role is %d\n",oper_roleid,player.froleid);           
		return PrivilegePolicy((GNET::Roles)player.froleid);
	}
public:
	OpResign() : Operation(_O_FACTION_RESIGN) { }
	~OpResign() {} 
	OpResign* Clone() { return new OpResign(); }
	
	bool NeedSync() { return false; }
	bool NeedAddInfo() { return false; }

	int oper_roleid;
