private:    
	GFactionServer::Player player;
	cancelexpelschedule_param_st params;
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
		//printf("opcancelexpelschedule: player(%d)'s faction role is %d\n",oper_roleid,player.froleid);           
		return PrivilegePolicy((GNET::Roles)player.froleid);
	}
public:
	OpCancelexpelschedule() : Operation(_O_FACTION_CANCELEXPELSCHEDULE) { }
	~OpCancelexpelschedule() {} 
	OpCancelexpelschedule* Clone() { return new OpCancelexpelschedule(); }
	
	bool NeedSync() { return false; }
	bool NeedAddInfo() { return false; }

	int oper_roleid;
