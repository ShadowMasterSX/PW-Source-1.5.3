private:    
	GFactionServer::Player player;
	upgrade_param_st params;
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
		//printf("opupgrade: player(%d)'s faction role is %d\n",oper_roleid,player.froleid);           
		return PrivilegePolicy((GNET::Roles)player.froleid);
	}
public:
	OpUpgrade() : Operation(_O_FACTION_UPGRADE) { }
	~OpUpgrade() {} 
	OpUpgrade* Clone() { return new OpUpgrade(); }
	
	bool NeedSync() { return true; }
	bool NeedAddInfo() { return false; }

	int oper_roleid;
