
package com.wanmei.db.cache;

public interface AbstractCacheMBean
{
	public int getSize();

	public int getMaxSize();
	public void setMaxSize(int max);

	public long getCountPut();
	public long getCountGet();
	public long getCountHit();
	public long getCountRemovePassive();
	public long getCountRemoveActive();
	public int  getPartition();

	public String getHitPercent();
}

