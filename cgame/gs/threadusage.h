#ifndef __THREAD_USAGE_H__
#define __THREAD_USAGE_H__

#include <sstream>
#include <stdio.h>

/*
 *   获得当前进程中各线程的CPU占用率，为后续的性能调优提供依据( 采集间隔为1秒、5秒、1分钟和5分钟)
 *   目前采集了ONET线程池、定时器线程、采集线程本身的数据以及本进程的数据
 *
 *   当启动后，会定期向标准输出, 可选择不进行标准输出，转到LogServer
 *   环境变量:
 *   	CPU_USAGE_FILE     如果设置，则不向标准输出, 而是输出到文件
 *   	CPU_USAGE_INTERVAL 如果设置，则打印的间隔被修改
 *   	CPU_USAGE_DELAY	   如果设置，则从启动到真正开始采集的间隔被修改
 *
 *    			yangyanhzao00972@wanmei.com    2012.5.17
 */

namespace ThreadUsage {

	//起动内部采集器， 由于采集本身使用了线程池和定时器，而此时这些不一定初始化就绪，可以delay一段时间，
	//可修改定期打印的间隔，为0时表示不打印
	void Start(int delaysecond, int dump_interval);

	//关闭采集器
	void Stop();

	//重新设置自动打印间隔。0是不打印
	void ChangeDumpInterval(int dump_interval);

	//手动对当前调用线程进行一次采集. 用于当线程是应用程序中手动创建出来的，不在内部采集器的管理之下，
	//可以将此函数加到线程工作线程循环中，最好一秒调用一次。内部自动进行了1秒、5秒这样的间隔处理
	//参数要求输入一个标识符，用于后续的打印中。
	//保留字勿用： Pool  --已被用作标识线程池中的线程
	//	       Timer --已被用作定时器线程
	//	       Stat  --内部采集器本身
	//	       Total --本进程
	void StatSelf(const char *thread_identification);

	//可手动将信息打印至fp中, 如stdout
	void Dump(FILE *fp);

	//获得每5分钟的信息，用于log输出，不像dump输出的那样带有回车符, 而且尽量简短
	void GetLogString(std::stringstream & os);
}
#endif
