
import com.goldhuman.IO.*;
import com.goldhuman.Common.*;
import com.goldhuman.IO.Protocol.*;
import com.goldhuman.Common.Security.*;

public class Client
{
	public static void main(String[] args)
	{
		try
		{
			// Conf.GetInstance(Server.class.getResource("/io.conf"));
			Conf.GetInstance(args[0]);
			Manager manager = new ClientManager();
			Protocol.Client(manager);
			ThreadPool.AddTask(PollIO.GetTask());

			for (;;)
			{
				try
				{
					Thread.sleep(1000);
				}
				catch (Exception e)
				{
				}
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
