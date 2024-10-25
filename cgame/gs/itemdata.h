#ifndef __ONLINEGAME_GS_ITEM_DATA_H__
#define __ONLINEGAME_GS_ITEM_DATA_H__
struct item_data 
{
	int type;		//����
	size_t count;		//����
	size_t pile_limit;	//�ѵ�����
	int equip_mask;		//װ����־	��ʾ����װ�����ĸ��ط� 0x80000000��ʾ�Ƿ�64λ��չ 0x40000000��ʾ�Ƿ��и�������
	int proc_type;		//��Ʒ�Ĵ���ʽ
	int classid;		//��Ӧitem�����GUID �������-1����ʾ����Ҫitem����
	struct 
	{
		int guid1;
		int guid2;
	}guid;			//GUID
	size_t price;		//���� ���������Ϊһ���ο�ֵ,ʵ��ֵ��ģ���е�Ϊ׼
	int expire_date; 	//��Ʒ����
	size_t content_length;	//�������ݵĴ�С
	char * item_content;	//�������� ��item����ʹ��

};

#endif

