#ifndef __ONLINEGAME_GS_FACTION_H__
#define __ONLINEGAME_GS_FACTION_H__

//����Ķ�������elements.data�������һ��

enum
{
	FACTION_HUMAN		= 0x0002,	//����
	FACTION_ORC			= 0x0004,	//����
	FACTION_YUMAO		= 0x0008,	//����
	FACTION_GHOST		= 0x0010,	//ϫ��
	FACTION_SPIRIT		= 0x0100,	//����
	FACTION_OBORO		= 0x0400,	//����
	
	FACTION_PARIAH		= 0x00200,		//��ɫ��
	FACTION_BATTLEOFFENSE	= 0x00800,		//��ս����
	FACTION_BATTLEDEFENCE	= 0x01000,		//��ս�ط�
	FACTION_OFFENSE_FRIEND	= 0x02000,		//�����ѷ�
	FACTION_DEFENCE_FRIEND	= 0x04000,		//�ط��ѷ�

	FACTION_MPVP_MINE_CAR	= 0x080000,	// ����ս��
	FACTION_MPVP_MINE_BASE	= 0x100000, // ����ս����
};
#endif
