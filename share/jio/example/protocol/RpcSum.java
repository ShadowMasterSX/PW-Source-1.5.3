package protocol;

import com.goldhuman.Common.*;
import com.goldhuman.Common.Marshal.*;
import com.goldhuman.Common.Security.*;
import com.goldhuman.IO.Protocol.*;

public final class RpcSum extends Rpc
{
	public void Server(Data argument, Data result) throws ProtocolException
	{
		TwoInteger arg = (TwoInteger)argument;
		OneInteger res = (OneInteger)result;
	}

	public void Client(Data argument, Data result) throws ProtocolException
	{
		TwoInteger arg = (TwoInteger)argument;
		OneInteger res = (OneInteger)result;
	}

	public void OnTimeout()
	{
	}

}
