private:    
	GFactionServer::Player player;
	allianceapply_param_st params;
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
		//printf("opallianceapply: player(%d)'s faction role is %d\n",oper_roleid,player.froleid);           
		return PrivilegePolicy((GNET::Roles)player.froleid);
	}
public:
	OpAllianceapply() : Operation(_O_FACTION_ALLIANCEAPPLY) { }
	~OpAllianceapply() {} 
	OpAllianceapply* Clone() { return new OpAllianceapply(); }
	
	bool NeedSync() { return true; }
	bool NeedAddInfo() { return false; }

	int oper_roleid;
