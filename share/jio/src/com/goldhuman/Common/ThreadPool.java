
package com.goldhuman.Common;

import java.util.*;
import java.util.regex.*;

public class ThreadPool implements java.lang.Runnable
{
	private static SortedMap tasks = Collections.synchronizedSortedMap(new TreeMap());
	private static SortedMap count = new TreeMap();
	private static int task_count  = 0;
	private static Object task_count_locker = new Object();
	private static long time_lastadd  = 0;
	private static long task_total = 0;
	private static LinkedList remove = new LinkedList();
	public Integer priority;

	static 
	{
		try
		{
			String config = Conf.GetInstance().find("ThreadPool", "config");
			if (config != null)
			{
				Matcher matcher = Pattern.compile("\\(\\s*(\\d+)\\s*,\\s*(\\d+)\\s*\\)").matcher(config);
				while (matcher.find())
				{
					int priority = Integer.parseInt(matcher.group(1));
					for (int count = Integer.parseInt(matcher.group(2)); count > 0; count --)
						AddThread(priority);
				}
			}	
		}
		catch (Exception e)
		{
		}
	}

	private ThreadPool(Integer priority)
	{
		this.priority = priority;
		synchronized(count)
		{
			Integer c = (Integer)count.get(priority);
			count.put(priority, new Integer(c == null ? 1 : (c.intValue() + 1)));
		}
	}

	public void run()
	{
		for (;;)
		{
			try
			{
				Runnable r = null;
				LinkedList ll = (LinkedList)tasks.get(priority);
				synchronized(ll)
				{
					while ( ll.isEmpty() )
						ll.wait();
					r = (Runnable)ll.removeLast();
					synchronized( task_count_locker ) { task_count --; }
				}
				r.run();
				synchronized(remove)
				{
					if (!remove.isEmpty() && priority.equals(remove.getLast()))
					{
						remove.removeLast();
						return;
					}
				}
			}
			catch (Exception e) { }
		}
	}

	public static void AddTask(Runnable r)
	{
		LinkedList ll = (LinkedList)tasks.get(new Integer(r.GetPriority()));
		if (ll == null)
		{
			System.out.println("ThreadPool thread LinkedList == null: no Match priority("+r.GetPriority()+")");
			return;
		}
		synchronized( ll )
		{
			ll.addFirst(r);
			synchronized( task_count_locker )
			{
				task_count ++;
				task_total ++;
			}
			time_lastadd = System.currentTimeMillis();
			ll.notify();
		}
	}

	public static int TaskCount()
	{
		synchronized( task_count_locker )
		{
			return task_count;
		}
	}

	public static long TaskTotal()
	{
		synchronized( task_count_locker )
		{
			return task_total;
		}
	}

	public static long TimeLastAdd()
	{
		return time_lastadd;
	}

	public static void AddThread(int priority)
	{
		new Thread(new ThreadPool(new Integer(priority))).start();
		if ( tasks.get(priority) == null )
			tasks.put(priority, new LinkedList());
	}

	public static int ThreadCount()
	{
		int sum = 0;
		synchronized(count)
		{
			for (Iterator it = count.entrySet().iterator(); it.hasNext(); )
				sum += ((Integer)(((Map.Entry)it.next()).getValue())).intValue();
		}
		return sum;
	}


	public static int ThreadCount(int priority)
	{
		int sum = 0;
		synchronized(count)
		{
			Integer i = (Integer)count.get(new Integer(priority));
			if (i != null)
				sum = i.intValue();
		}
		return sum;
	}

	public static void RemoveThread(int prior)
	{
		Integer priority = new Integer(prior);
		synchronized(count)
		{
			Integer c = (Integer)count.get(priority);
			if (c != null)
			{
				int n = c.intValue() - 1;
				if (n > 0)
				{
					count.put(priority, new Integer(n));
					synchronized(remove)
					{
						remove.addFirst(priority);
					}
				}
				else
					count.remove(priority);
			}
		}
	}
}

