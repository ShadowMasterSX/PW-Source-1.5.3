private:    
	GFactionServer::Player player;
	removerelationapply_param_st params;
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
		//printf("opremoverelationapply: player(%d)'s faction role is %d\n",oper_roleid,player.froleid);           
		return PrivilegePolicy((GNET::Roles)player.froleid);
	}
public:
	OpRemoverelationapply() : Operation(_O_FACTION_REMOVERELATIONAPPLY) { }
	~OpRemoverelationapply() {} 
	OpRemoverelationapply* Clone() { return new OpRemoverelationapply(); }
	
	bool NeedSync() { return true; }
	bool NeedAddInfo() { return false; }

	int oper_roleid;
