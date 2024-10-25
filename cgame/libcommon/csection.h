/*
	csection.h
	���ߣ�����
	ע�ͣ��ٽ����ķ�װ��Ŀ����ʵ�ֿ�����ͬһ�߳�������Ļ���
*/

#ifndef __CRITICALSECTION_H__
#define __CRITICALSECTION_H__

#ifdef __cplusplus
extern "C"{
#endif

typedef struct critical_section_t critical_section;

critical_section * initialize_critical_section();
void delete_critical_section(critical_section *__cs);

void  enter_critical_section(critical_section *__cs);
void  leave_critical_section(critical_section *__cs);

#ifdef __cplusplus
};
#endif

#endif  //EOF critical section 

