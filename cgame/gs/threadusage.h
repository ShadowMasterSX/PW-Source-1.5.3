#ifndef __THREAD_USAGE_H__
#define __THREAD_USAGE_H__

#include <sstream>
#include <stdio.h>

/*
 *   ��õ�ǰ�����и��̵߳�CPUռ���ʣ�Ϊ���������ܵ����ṩ����( �ɼ����Ϊ1�롢5�롢1���Ӻ�5����)
 *   Ŀǰ�ɼ���ONET�̳߳ء���ʱ���̡߳��ɼ��̱߳���������Լ������̵�����
 *
 *   �������󣬻ᶨ�����׼���, ��ѡ�񲻽��б�׼�����ת��LogServer
 *   ��������:
 *   	CPU_USAGE_FILE     ������ã������׼���, ����������ļ�
 *   	CPU_USAGE_INTERVAL ������ã����ӡ�ļ�����޸�
 *   	CPU_USAGE_DELAY	   ������ã����������������ʼ�ɼ��ļ�����޸�
 *
 *    			yangyanhzao00972@wanmei.com    2012.5.17
 */

namespace ThreadUsage {

	//���ڲ��ɼ����� ���ڲɼ�����ʹ�����̳߳غͶ�ʱ��������ʱ��Щ��һ����ʼ������������delayһ��ʱ�䣬
	//���޸Ķ��ڴ�ӡ�ļ����Ϊ0ʱ��ʾ����ӡ
	void Start(int delaysecond, int dump_interval);

	//�رղɼ���
	void Stop();

	//���������Զ���ӡ�����0�ǲ���ӡ
	void ChangeDumpInterval(int dump_interval);

	//�ֶ��Ե�ǰ�����߳̽���һ�βɼ�. ���ڵ��߳���Ӧ�ó������ֶ����������ģ������ڲ��ɼ����Ĺ���֮�£�
	//���Խ��˺����ӵ��̹߳����߳�ѭ���У����һ�����һ�Ρ��ڲ��Զ�������1�롢5�������ļ������
	//����Ҫ������һ����ʶ�������ں����Ĵ�ӡ�С�
	//���������ã� Pool  --�ѱ�������ʶ�̳߳��е��߳�
	//	       Timer --�ѱ�������ʱ���߳�
	//	       Stat  --�ڲ��ɼ�������
	//	       Total --������
	void StatSelf(const char *thread_identification);

	//���ֶ�����Ϣ��ӡ��fp��, ��stdout
	void Dump(FILE *fp);

	//���ÿ5���ӵ���Ϣ������log���������dump������������лس���, ���Ҿ������
	void GetLogString(std::stringstream & os);
}
#endif
