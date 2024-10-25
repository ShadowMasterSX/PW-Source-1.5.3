package com.goldhuman.Common;

import java.util.Observer;
import java.util.Observable;
import java.util.Iterator;
import java.util.LinkedList;

public class TimerTask implements Observer
{
    private class TaskPair
    {
        TaskPair(long _w,Runnable _t)
        {
            waitsecds = _w;
            task = _t;
        }
        long waitsecds;
        Runnable task;
    }

    private static TimerTask instance = new TimerTask();
    private LinkedList tasks = new LinkedList();
    private long elapse =0;

    private TimerTask()
    {
        TimerObserver.GetInstance().addObserver(this);
    }
    public synchronized void update(Observable o, Object arg)
    {
        ++elapse;
        Iterator iter = tasks.iterator();
        while( iter.hasNext() ){
            TaskPair tp = (TaskPair)iter.next();
            if( tp.waitsecds > elapse )
                break;
            ThreadPool.AddTask(tp.task);
            iter.remove();
        }
    }
    public synchronized void AddTask(Runnable task,long waitsecds)
    {
        tasks.add( new TaskPair(waitsecds+elapse, task) );
    }
    public static void AddTimerTask(Runnable task,long waitsecds)
    {
        instance.AddTask(task,waitsecds);
    }
}

