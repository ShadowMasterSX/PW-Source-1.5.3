
package com.goldhuman.Common;

import java.util.*;
import java.util.regex.*;

public class ThreadPoolRelax implements java.lang.Runnable
{
	private static SortedMap tasks = new TreeMap();
	private static SortedMap count = new TreeMap();
	private static int task_count  = 0;
	private static long time_lastadd  = 0;
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

	private ThreadPoolRelax(Integer priority)
	{
		this.priority = priority;
		synchronized(count)
		{
			Integer c = (Integer)count.get(priority);
			count.put(priority, new Integer(c == null ? 1 : (c.intValue() + 1)));
		}
	}

	private Runnable GetTask(SortedMap map)
	{
		for (Iterator it = map.entrySet().iterator(); it.hasNext(); )
		{
			LinkedList ll = (LinkedList)((Map.Entry)it.next()).getValue();
			if (!ll.isEmpty())
				return (Runnable)ll.removeLast(); 
		}
		return null;
	}

	public void run()
	{
		for (;;)
		{
			try
			{
				Runnable r = null;
				synchronized(tasks)
				{
					while (task_count == 0)
						tasks.wait();
					if ((r = GetTask(tasks.tailMap(priority))) == null)
						r = GetTask(tasks);
					task_count --;
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
		synchronized(tasks)
		{
			Integer priority = new Integer(r.GetPriority());
			LinkedList ll = (LinkedList)tasks.get(priority);
			if (ll == null)
				tasks.put(priority, ll = new LinkedList());
			ll.addFirst(r);
			task_count ++;
			time_lastadd = System.currentTimeMillis();
			tasks.notify();
		}
	}

	public static int TaskCount()
	{
		return task_count;
	}

	public static long TimeLastAdd()
	{
		return time_lastadd;
	}

	public static void AddThread(int priority)
	{
		new Thread(new ThreadPoolRelax(new Integer(priority))).start();
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
