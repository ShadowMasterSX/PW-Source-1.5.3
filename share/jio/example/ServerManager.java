import protocol.*;
import com.goldhuman.IO.*;
import com.goldhuman.Common.*;
import com.goldhuman.IO.Protocol.*;
import com.goldhuman.Common.Security.*;

public final class ServerManager extends Manager
{
	protected void OnAddSession(Session session)
	{
		System.out.println("OnAddSession " + session + " " + session.getPeerAddress());
	}

	protected void OnDelSession(Session session)
	{
		System.out.println("OnDelSession " + session + " " + session.getPeerAddress());
	}

	protected State GetInitState()
	{
		return State.Get("rpc");
	}

	protected String Identification()
	{
		return "Server";
	}

}
