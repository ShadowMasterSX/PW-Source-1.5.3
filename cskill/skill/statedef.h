#ifndef __SKILL_STATEDEF_H__
#define __SKILL_STATEDEF_H__

namespace GNET
{
enum visible_state{
	// ͨ��״̬
	VSTATE_BLESSED     = 1,  // ������ǿ
	VSTATE_CURSED      = 2,  // �����½�
	VSTATE_RETORT      = 3,  // ����
	VSTATE_SLOW        = 4,  // ����
	VSTATE_DIZZY       = 5,  // ����
	VSTATE_SLEEP       = 6,  // ˯��
	VSTATE_FIX         = 7,  // ����
	VSTATE_SEALED      = 8,  // ��ӡ, ���ܹ���
	VSTATE_INFAUST     = 9,  // ����, ���ⲻ��״̬
	VSTATE_THUNDER     = 10, // �׻�, ������ϵ����
	VSTATE_TOXIC       = 11, // �ж�
	VSTATE_BURNING     = 12, // ȼ��, ������ϵ����
	VSTATE_FALLEN      = 13, // ��ʯ
	VSTATE_BLEEDING    = 14, // ��Ѫ
	VSTATE_MAGICLEAK   = 15, // ħ��ȼ��
	VSTATE_ICEBLADE    = 16, // ˪��
	VSTATE_POWERUP     = 17, // ����
	VSTATE_FIREBLADE   = 18, // ���ӻ��˺�
	VSTATE_FLOWER1     = 19, // �ʻ�1
	VSTATE_FLOWER2     = 20, // �ʻ�2
	VSTATE_FLOWER3     = 21, // �ʻ�3
	VSTATE_FLOWER4     = 22, // �ʻ�4
	VSTATE_NOREGAIN    = 23, // ���꣬�Զ��ָ�ֹͣ
	// ��ʦ����
	VSTATE_FIRESHIELD  = 24, // ���滤��
	VSTATE_ICESHIELD   = 25, // ��������
	VSTATE_SOILSHIELD  = 26, // ��ʯ����
	VSTATE_ANTIWATER   = 27, // ��ˮ��
	// ��������
	VSTATE_FEATHERSHIELD = 28, // ���
	// ��â����
	VSTATE_WINGSHIELD  = 29, // ���
	VSTATE_FIREARROW   = 30, // ����֮ʸ
	// ��������
	VSTATE_JINGJI      = 31, // ������
	VSTATE_FOXFORM     = 32, // ������
	// ��������
	VSTATE_BLIND         = 33, // Ŀä

	VSTATE_IMMUNESEALED = 34,	//���߷�ӡ//lgc
	VSTATE_IMMUNESLEEP 	= 35,	//����˯��
	VSTATE_IMMUNEFIRE 	= 36,	//������
	VSTATE_IMMUNEWATER 	= 37,	//ˮ����
	VSTATE_IMMUNEMETAL 	= 38,	//������
	VSTATE_IMMUNEWOOD 	= 39,	//ľ����
	
	VSTATE_IMMUNESOIL 	= 40,	//������
	VSTATE_WINDSHIELD  	= 41,	//�����
	VSTATE_AIRSTREAMLOCK= 42,	//������
	VSTATE_WEAKELEMENT 	= 43,	//Ԫ������
	VSTATE_IMMUNEWEAK	= 44,	//�˺���������
	VSTATE_BEFROZEN 	= 45,	//����
	VSTATE_VACUUM 		= 46,	//���
	VSTATE_WATERFIREARMOR = 47,	//ˮ�𻤼�
	
	VSTATE_FSSTAR 		= 48,	//��ʦ֮��
	VSTATE_INVINCIBLE 	= 49,	//�޵� //��ֵ���ı䣬ͬʱӦ���� cgame/gs/invincible_filter.cpp
	VSTATE_APGENCONT 	= 50,	//��������Ԫ��
	VSTATE_APLEAKCONT 	= 51,	//��������Ԫ��
	VSTATE_ADDATTACKDEGREE 		= 52,	//�����ȼ�����
	VSTATE_ADDDEFENCEDEGREE 	= 53,	//�����ȼ�����
	VSTATE_ABSORBPHYSICDAMAGE 	= 54,	//���������˺�
	VSTATE_ABSORBMAGICDAMAGE 	= 55,	//���շ����˺�
	
	VSTATE_IMMUNEBLOODING		= 56,	//��Ѫ����
	VSTATE_HARDENSKIN			= 57,	//���
	VSTATE_INVISIBLE			= 58,	//����//��ֵ���ı䣬ͬʱӦ���� cgame/gs/invisible_filter.cpp
	VSTATE_SOULRETORT			= 59,	//����֮��
	VSTATE_SOULSEALED			= 60,	//��ӡ֮��
	VSTATE_SOULBEATBACK			= 61,	//����֮��
	VSTATE_SOULSTUN				= 62,	//����֮��
	VSTATE_HURTWHENUSESKILL		= 63,	//ʹ�ü���ʱ���˺�

	VSTATE_INCDEBUFFDODGE		= 64,	//״̬����
	VSTATE_INCDAMAGEDODGE		= 65,	//�˺�����
	VSTATE_ADJUSTATTACKDEFEND1	= 66,	//�ӹ�����
	VSTATE_ADJUSTATTACKDEFEND2	= 67,	//�����ӷ�
	VSTATE_DEEPICETHRUST		= 68,	//��ȱ���
	VSTATE_ATTACKATTACHSTATE1	= 69,	//����ʱ����״̬����1
	VSTATE_ATTACKATTACHSTATE2	= 70,	//����ʱ����״̬����2
	VSTATE_ATTACKATTACHSTATE3	= 71,	//����ʱ����״̬����3

	VSTATE_BLADESHADOW			= 72,	//�˷���Ӱ
	VSTATE_CONSECRATION 		= 73,	//�׼�
	VSTATE_POISIONSEED			= 74,	//����
	VSTATE_HPGENSEED			= 75,	//��Ѫ��
	VSTATE_INCWOODWATERDEFENSE 	= 76,	//����ľˮ��� 
	VSTATE_SPECIALSLOW			= 77,	//����ļ���
	VSTATE_TRANSPORTMPTOPET 	= 78,	//���Լ��ĳ��ﴫ��mp
	VSTATE_FASTPRAYINCMAGIC 	= 79,	//�ӿ��������ӷ���
	
	VSTATE_INCDEFENCESMITE 		= 80,	//�����������
	VSTATE_INCRESISTMAGIC 		= 81,	//���ӷ�������
	VSTATE_TRANSPORTDAMAGETOPET = 82,	//��ʩ����(����)�����˺�
	VSTATE_ABSORBDAMAGEINCDEFENSE = 83,	//���˺����﷨��
	VSTATE_CHANCEOFREBIRTH		= 84,	//��ľ�괺
	VSTATE_INCCOUNTEDSMITE		= 85,	//��������(����Ч����)
	VSTATE_ATTACKATTACHSTATE4	= 86,	//����ʱ����״̬����4
	VSTATE_SHURA1				= 87,	//���޵�
	VSTATE_SHURA2				= 88,	//���޵�
	VSTATE_SHURA3				= 89,	//���޵�
	VSTATE_SHURA4				= 90,	//���޵�
	VSTATE_POSITIONROLLBACK		= 91,
	VSTATE_HPROLLBACK			= 92,
	VSTATE_NOCHANGESELECT		= 93,
	VSTATE_HEALABSORB			= 94,
	VSTATE_REPELONNORMALATTACK	= 95,
	VSTATE_DECCRITRESISTANCE	= 96,
	VSTATE_TRANSMITSKILLATTACK	= 97,
	VSTATE_ADDITIONALATTACK		= 98,
	VSTATE_FORBIDBESELECTED		= 99,
	VSTATE_DELAYEARTHHURT		= 100,
	VSTATE_DIZZYINCHURT			= 101,
	VSTATE_SOULRETORT2			= 102,
	VSTATE_FIXONDAMAGE			= 103,
	VSTATE_INCATTACKONDAMAGE	= 104,
	VSTATE_REBIRTH2				= 105,
	VSTATE_HEALSTEAL			= 106,
	VSTATE_DROPMONEYONDEATH		= 107,
	VSTATE_INCATTACKRANGE		= 108,
	VSTATE_INVINCIBLE5			= 109,
	VSTATE_THUNDERFORM			= 110,
	VSTATE_BLADEAURA			= 111,
	VSTATE_YJDANCE				= 112,
	VSTATE_FSICEWORLD			= 113,
	VSTATE_FLAGER				= 114,	//��ս����
	VSTATE_DOG					= 115,
	VSTATE_SNAKE				= 116,
	VSTATE_PALSY				= 117,	//̱���������
	VSTATE_MODIFYSPECSKILLPRAY	= 118,	
	VSTATE_INCSPECSKILLDAMAGE	= 119,	
	VSTATE_HEALSHIELD			= 120,
	VSTATE_INCFLYSPEED			= 121,
	VSTATE_MOVEPUNISH			= 122,
	VSTATE_STANDPUNISH			= 123,
	VSTATE_STANDPUNISH2			= 124,
	VSTATE_CHANTSHIELD			= 125,
	VSTATE_INTERVALPALSY		= 126,
	VSTATE_HUNTERSOUL			= 127,
	VSTATE_WHITE1				= 128,
	VSTATE_WHITE2				= 129,
	VSTATE_WHITE3				= 130,
	VSTATE_BLACK1				= 131,
	VSTATE_BLACK2				= 132,
	VSTATE_BLACK3				= 133,
	VSTATE_WHITE1BLACK1			= 134,
	VSTATE_WHITE1BLACK2			= 135,
	VSTATE_WHITE2BLACK1			= 136,
	VSTATE_INTERNALINJURY		= 137,
	VSTATE_DEATHRESETCD			= 138,
	VSTATE_APPENDDAMAGE			= 139,
	VSTATE_COOLDOWNPUNISH		= 140,
	VSTATE_SKILL2772			= 141,

	VSTATE_MAX = 192,
};

enum hidden_state{
	HSTATE_DIZZY         = 1,    // ����
	HSTATE_SLEEP         = 2,    // ˯��
	HSTATE_SLOW          = 3,    // �ƶ��ٶȼ���
	HSTATE_RETORT        = 4,    // ���������˺�
	HSTATE_FEATHERSHIELD = 5,    // ���

	HSTATE_FIRESHIELD    = 6,    // ���滤��
	HSTATE_ICESHIELD     = 7,    // ��������
	HSTATE_FIX           = 8,    // ����
	HSTATE_SEALED        = 9,    // ���ܹ���
	HSTATE_BLIND         = 10,   // �䱻��

	HSTATE_SOILSHIELD    = 11,   // ��ʯ����
	HSTATE_THUNDER       = 12,   // �������˺�
	HSTATE_TOXIC         = 13,   // ����ľ�˺�(�ж�)
	HSTATE_BURNING       = 14,   // �������˺�
	HSTATE_FALLEN        = 15,   // �������˺�

	HSTATE_MAGICLEAK     = 16,   // ��������MP
	HSTATE_BLEEDING      = 17,   // ���������˺�(��Ѫ)
	HSTATE_DECDEFENCE    = 18,   // �������������
	HSTATE_DECRESIST     = 19,   // �������з�����
	HSTATE_DECATTACK     = 20,   // ������������

	HSTATE_INCHURT       = 21,   // �����˺��ӱ�
	HSTATE_SLOWATTACK    = 22,   // �����ٶȼ���
	HSTATE_SLOWPRAY      = 23,   // ��������
	HSTATE_DECACCURACY   = 24,   // ׼ȷ���½�
	HSTATE_DECDODGE      = 25,   // ���Ͷ����

	HSTATE_FASTHPGEN     = 26,   // HP�ָ��ٶȼӿ�(����)
	HSTATE_FASTMPGEN     = 27,   // MP�ָ��ٶȼӿ�(����)
	HSTATE_INCHP         = 28,   // ����HP����
	HSTATE_INCMP         = 29,   // ����MP����
	HSTATE_INCDEFENCE    = 30,   // ��ǿ���������

	HSTATE_INCRESIST     = 31,   // ��ǿ���з�����
	HSTATE_INCATTACK     = 32,   // �����������
	HSTATE_FASTATTACK    = 33,   // ��������
	HSTATE_INCDODGE      = 34,   // ���������
	HSTATE_DECHURT       = 35,   // �����˺�����

	HSTATE_INCACCURACY   = 36,   // ׼ȷ������
	HSTATE_FASTPRAY      = 37,   // �ӿ�����
	HSTATE_ICEBLADE      = 38,   // ���ӱ��˺�
	HSTATE_FIREBLADE     = 39,   // ���ӻ��˺�
	HSTATE_TOXICBLADE    = 40,   // ����

	HSTATE_HPGEN         = 41,   // �����ָ�HP
	HSTATE_MPGEN         = 42,   // �����ָ�MP
	HSTATE_DECMAGIC      = 43,   // ��������������
	HSTATE_SPEEDUP       = 44,   // ����
	HSTATE_DECHP         = 45,   // ����HP����

	HSTATE_INCMAGIC      = 46,   // ��ǿħ��������
	HSTATE_TIGERFORM     = 47,   // �׻���
	HSTATE_ENHANCEGOLD   = 48,   // �������
	HSTATE_ENHANCEWOOD   = 49,   // ľ������
	HSTATE_ENHANCEWATER  = 50,   // ˮ������

	HSTATE_ENHANCEFIRE   = 51,   // �������
	HSTATE_ENHANCESOIL   = 52,   // ��������
	HSTATE_REDUCEGOLD    = 53,   // ����½�
	HSTATE_REDUCEWOOD    = 54,   // ľ���½�
	HSTATE_REDUCEWATER   = 55,   // ˮ���½�

	HSTATE_REDUCEFIRE    = 56,   // ����½�
	HSTATE_REDUCESOIL    = 57,   // �����½�
	HSTATE_ANTIWATER     = 58,   // ��ˮ��
	HSTATE_PANRUO        = 59,   // �����ľ�
	HSTATE_POWERUP       = 60,   // ��Ԫ����

	HSTATE_XISUI         = 61,   // ϴ�辭
	HSTATE_YIJIN         = 62,   // �׽
	HSTATE_APGEN         = 63,   // ����֮ŭ
	HSTATE_STONESKIN     = 64,   // ��׷�
	HSTATE_IRONSHIELD    = 65,   // ������

	HSTATE_GIANT         = 66,   // ��������
	HSTATE_DEVILSTATE    = 67,   // ��Ѫ��ħ
	HSTATE_BLESSMAGIC    = 68,   // ������
	HSTATE_WINGSHIELD    = 69,   // ���
	HSTATE_FIREARROW     = 70,   // ����֮ʸ

	HSTATE_EAGLECURSE    = 71,   // ������ӥ
	HSTATE_FREEMOVE      = 72,   // ���������ж��谭״̬
	HSTATE_FROZEN        = 73,   // �������˺�
	HSTATE_INCSMITE      = 74,   // ����������
	HSTATE_FOXFORM       = 75,   // ������
	HSTATE_INVINCIBLE    = 76,   // �޵�
	HSTATE_JINGJI        = 77,   // ������
	HSTATE_NOREGAIN      = 78,   // ���꣬�Զ��ָ�ֹͣ
	HSTATE_CANTI         = 79,   // ����, �˺�����
	HSTATE_APLEAK        = 80,   // ����, �ܵ�����ʧȥԪ��
	HSTATE_SWIFTFORM     = 81,   // �ɻ���Ӱ
	HSTATE_FASTRIDE      = 82,   // ��˼���
	HSTATE_SHARPBLADE    = 83,   // ���������
	HSTATE_IMMUNESEALED		= 84,	//���߷�ӡ//lgc
	HSTATE_IMMUNESLEEP		= 85,	//����˯��
	HSTATE_IMMUNESLOWDIZZY	= 86,	//�����ƶ����ٺ���
	
	HSTATE_IMMUNEWOUND		= 87,	//���߰�������Ѫ
	HSTATE_IMMUNEALL		= 88,	//�������и���״̬
	HSTATE_IMMUNEPHYSICAL	= 89,	//���������˺�
	HSTATE_IMMUNEFIRE		= 90,	//���߻��˺�
	HSTATE_IMMUNEWATER		= 91,	//����ˮ�˺�
	HSTATE_IMMUNEMETAL		= 92,	//���߽��˺�
	HSTATE_IMMUNEWOOD		= 93,	//����ľ�˺�
	HSTATE_IMMUNESOIL		= 94,	//�������˺�
	HSTATE_IMMUNEMAGICAL	= 95,	//���������˺�
	HSTATE_ARROGANT			= 96,	//����һ��״̬

	HSTATE_SLOWSWIM			= 97,	//��Ӿ��������
	HSTATE_FASTSWIM			= 98,	//��Ӿ��������
	HSTATE_SLOWFLY			= 99,	//���б�������
	HSTATE_FASTFLY			= 100,	//���б�������
	HSTATE_SLOWRIDE			= 101, 	//��˱�������
	HSTATE_APGENCONT		= 102,	//��������Ԫ��
	HSTATE_APLEAKCONT		= 103, 	//��������Ԫ��
	HSTATE_INCELFSTR		= 104,	//����С��������
	HSTATE_INCELFAGI		= 105,	//����С��������
	HSTATE_INCDEFENCE2		= 106,	//�����������
	
	HSTATE_WEAKELEMENT		= 107,	//Ԫ������������������� ���������½�
	HSTATE_DEEPPOISION 		= 108,	//����ܵ��˺�����
	HSTATE_ROOTED			= 109,	//����������ͬʱ�˺�����ͬʱ�����˺�
	HSTATE_EARTHGUARD		= 110,	//����ػ����ظ�hp�˺���������ͷ��������½�
	HSTATE_FURY				= 111,	//�񱩣�Ŀ������ͷ�����������ͬʱ�����ȼ�����
	HSTATE_SANDSTORM		= 112,	//ɳ�������н���ʩ���ٶȱ���
	HSTATE_HOMEFEELING		= 113,	//���飬�����������ظ��ٶ������ƶ��ٶ�����
	HSTATE_REDUCEWATER2		= 114,	//����ˮ������
	HSTATE_INCSMITE2		= 115, 	//���ˣ���������
	HSTATE_DECDEFENCE2		= 116,	//����������������
	
	HSTATE_REDUCEFIRE2		= 117,	//�𻨣����������
	HSTATE_SLOWATTACKPRAY	= 118,	//�Ļ�����ͷ��������ٶ��½�
	HSTATE_BURNING2			= 119,	//���֣��������˺�
	HSTATE_BURNINGFEET		= 120,	//ȼ�㣬��������ͣ��ƶ��ٶȽ���
	HSTATE_HARDENSKIN		= 121,	//��ڣ���������ͷ����˺������������ƶ��ٶ�
	HSTATE_REDUCEGOLD2		= 122,	//���У����������
	HSTATE_LEAFDANCE		= 123,	//Ҷ�裬�ٻ�+��Ѫ
	HSTATE_CHARRED			= 124,	//�ս��������½������½��ƶ��ٶ�����
	HSTATE_VACUUM			= 125,	//��գ�ʩ���ٶȱ��� �ƶ����� �ܵ��˺�����
	HSTATE_IMMUNEBLOODING	= 126,	//������Ѫ
	
	HSTATE_ABSORBPHYSICDAMAGE	= 127,	//���������˺�
	HSTATE_ABSORBMAGICDAMAGE	= 128,	//���շ����˺�
	HSTATE_RETORTMAGIC		= 129,		//���������˺�
	HSTATE_WINDSHIELD		= 130,		//��ܣ��˺������������ٶȺ���������
	HSTATE_AIRSTREAMLOCK	= 131,		//��������Ŀ�궨�����м��ʱ���ӡ
	HSTATE_CLOSED			= 132,		//��գ���ӡ����������
	HSTATE_IMMUNEWEAK		= 133,		//�����˺�����
	HSTATE_BEFROZEN			= 134,		//���ᣬͬѣ��
	HSTATE_ADDATTACKDEGREE  = 135,		//���ӹ����ȼ�
	HSTATE_SUBATTACKDEGREE  = 136,		//���͹����ȼ�
	HSTATE_ADDDEFENCEDEGREE  = 137,		//���ӷ����ȼ�
	HSTATE_SUBDEFENCEDEGREE  = 138,		//���ͷ����ȼ�
	HSTATE_FALLEN2 			= 139, 		//�������˺����ܹ����ȼ�Ӱ��ͬburning2
	HSTATE_SEALED2			= 140,		//���ܹ���2
	HSTATE_FIX2				= 141,		// ����2
	HSTATE_DECHURT2			= 142,		// �����˺�����2
	HSTATE_INCHURT2			= 143,		// �����˺��ӱ�2
	HSTATE_INCHP2			= 144,		// ����HP���ֵ2������
	HSTATE_INCATTACK2		= 145,		// ��ǿ������2, ���ӱ���
	HSTATE_INCMAGIC2		= 146,		// ��ǿ����������2, ���ӱ���
	HSTATE_FASTPRAY2		= 147,		// ��������2
	HSTATE_SPEEDUP2			= 148,		// ����2�� ���ӱ���
	HSTATE_INVISIBLE		= 149,		// ���� //��ֵ���ı䣬ͬʱӦ���� cgame/gs/invisible_filter.cpp
	HSTATE_INCANTIINVISIBLEACTIVE = 150,// ���ӷ����ȼ�
	HSTATE_INCHPSTEAL		= 151,		// ������Ѫ
	HSTATE_INCCRITDAMAGE	= 152,		// ���ӱ����˺�
	HSTATE_INCDAMAGEDODGE	= 153,		// �����˺�����
	HSTATE_INCDEBUFFDODGE	= 154,		// ����״̬����
	HSTATE_REBIRTH			= 155,		// ����
	HSTATE_SOULRETORT		= 156,		// ����֮��
	HSTATE_SOULSEALED		= 157,		// ��ӡ֮��
	HSTATE_SOULBEATBACK		= 158,		// ����֮��
	HSTATE_SOULSTUN			= 159,		// ����֮��
	HSTATE_DEEPENBLESS		= 160,		// ף������
	HSTATE_WEAKENBLESS		= 161,		// ף������
	HSTATE_HURTWHENUSESKILL	= 162,		// ʹ�ü���ʱ���˺�
	HSTATE_INTERRUPTWHENUSESKILL = 163, // ʹ�ü���ʱ���ж�	
	HSTATE_FISHFORM			= 164,		// ���˱�
	HSTATE_DEEPICETHRUST	= 165,		//��ȱ���
	HSTATE_ADJUSTATTACKDEFEND = 166,	//�����ȼ�����
	HSTATE_INCHURTPHYSICGOLD= 167,		//��������Թ����˺�����
	HSTATE_INCHURTWOODWATER = 168,		//ľˮ���Թ����˺�����
	HSTATE_INCHURTFIREEARTH = 169,		//�������Թ����˺�����
	HSTATE_ATTACKATTACHSTATE1 = 170,	//����ʱ����״̬����1
	HSTATE_ATTACKATTACHSTATE2 = 171,	//����ʱ����״̬����2
	HSTATE_ATTACKATTACHSTATE3 = 172,	//����ʱ����״̬����3
	HSTATE_BEATTACKEDATTACHSTATE1 = 173,//������ʱ����״̬����1
	HSTATE_BEATTACKEDATTACHSTATE2 = 174,//������ʱ����״̬����2
	HSTATE_BEATTACKEDATTACHSTATE3 = 175,//������ʱ����״̬����3
	HSTATE_BLADESHADOW		= 176,		//�˷���Ӱ
	HSTATE_POISIONSEED		= 177,		//����
	HSTATE_HPGENSEED		= 178,		//��Ѫ��
	HSTATE_INCWOODWATERDEFENSE = 179,	//����ľˮ��� 
	HSTATE_SPECIALSLOW		= 180,		//����ļ���
	HSTATE_INCDEFENCESMITE 	= 181,		//�����������
	HSTATE_INCRESISTMAGIC 	= 182,		//���ӷ�������
	HSTATE_TRANSPORTMPTOPET = 183,		//���Լ��ĳ��ﴫ��mp
	HSTATE_TRANSPORTDAMAGETOPET = 184,	//��ʩ����(����)�����˺�
	HSTATE_ABSORBDAMAGEINCDEFENSE = 185,	//���˺����﷨��
	HSTATE_FASTPRAYINCMAGIC = 186,		//�ӿ��������ӷ���
	HSTATE_INCREMENTALHPGEN = 187,		//�����Ļ�Ѫ
	HSTATE_CONSECRATION 	= 188,		//�׼�
	HSTATE_CHANCEOFREBIRTH	= 189,		//��ľ�괺
	HSTATE_SPECIALPHYSICHURTTRIGGER	= 190,	//����������˺�������
	HSTATE_INCCOUNTEDSMITE	= 191,		//��������(����Ч����)
	HSTATE_ATTACKATTACHSTATE4 = 192,	//����ʱ����״̬����4
	HSTATE_BEATTACKEDATTACHSTATE4 = 193,//������ʱ����״̬����4
	HSTATE_WEAPONDISABLED	= 194,		//����ʧЧ
	HSTATE_INCAGGROONDAMAGE = 195,		//���������ܵ��˺�ʱ�ĳ��
	HSTATE_ENHANCESKILLDAMAGE = 196,	//���Ӽ��ܶ�npc���˺�
	HSTATE_DETECTINVISIBLE	= 197,		//�����Χ�������
	HSTATE_FASTMPGEN2     	= 198,   	// MP�ָ��ٶȼӿ�(����)
	HSTATE_POSITIONROLLBACK	= 199,
	HSTATE_HPROLLBACK		= 200,
	HSTATE_NOFLY			= 201,
	HSTATE_NOCHANGESELECT	= 202,
	HSTATE_HEALABSORB		= 203,
	HSTATE_REPELONNORMALATTACK	= 204,
	HSTATE_INCCRITRESISTANCE	= 205,
	HSTATE_DECCRITRESISTANCE	= 206,
	HSTATE_TRANSMITSKILLATTACK	= 207,
	HSTATE_ADDITIONALHEAL		= 208,
	HSTATE_ADDITIONALATTACK		= 209,
	HSTATE_TRANSPORTDAMAGETOLEADER = 210,
	HSTATE_FORBIDBESELECTED		= 211,
	HSTATE_ENHANCESKILLDAMAGE2	= 212,
	HSTATE_DELAYEARTHHURT		= 213,
	HSTATE_DIZZYINCHURT			= 214,
	HSTATE_SOULRETORT2			= 215,
	HSTATE_FIXONDAMAGE			= 216,
	HSTATE_APGEN2				= 217,
	HSTATE_INCATTACK3			= 218,
	HSTATE_INCATTACKONDAMAGE	= 219,
	HSTATE_REBIRTH2				= 220,
	HSTATE_HEALSTEAL			= 221,
	HSTATE_DROPMONEYONDEATH		= 222,
	HSTATE_INCATTACKRANGE		= 223,
	HSTATE_INVINCIBLE5			= 224,
	HSTATE_THUNDERFORM			= 225,
	HSTATE_BLADEAURA			= 226,
	HSTATE_YJDANCE				= 227,
	HSTATE_DELAYTRANSMIT		= 228,
	HSTATE_DECNORMALATTACKHURT	= 229,
	HSTATE_INCATKDEFHP			= 230,
	HSTATE_HPMPGENNOTINCOMBAT	= 231,
	HSTATE_INCHPMP				= 232,
	HSTATE_INCHURT3				= 233,
	HSTATE_NONPENALTYPVP		= 234,
	HSTATE_INCRESIST2			= 235,
	HSTATE_FLAGER				= 236,	//��ս����
	HSTATE_SUBDEFENCEDEGREE2 	= 237,	//���ͷ����ȼ�2
	HSTATE_INCATKDEFHP2			= 238,
	HSTATE_INCSMITE3			= 239,
	HSTATE_INCPENRES			= 240,	//������ħ/��ħ�ȼ�
	HSTATE_INCMAXHPATKDFDLEVEL	= 241,	//ͬʱ����HP���޺͹����ȼ�
	HSTATE_GTAWARD1				= 242, 	//gt������buff1
	HSTATE_GTAWARD2				= 243, 	//gt������buff2
	HSTATE_DECHURT3				= 244,	//�˺�����3
	HSTATE_ATTACHSTATETOSELF	= 245,	//���Լ�����״̬��
	HSTATE_ATTACHSTATETOTARGET	= 246,	//��Ŀ�긽��״̬��
	HSTATE_IMMUNEPHYSICAL2		= 247,	//���������˺�2
	HSTATE_IMMUNEMETAL2			= 248,	//���߽��˺�2
	HSTATE_IMMUNEWOOD2			= 249,	//����ľ�˺�2
	HSTATE_IMMUNEWATER2			= 250,	//����ˮ�˺�2
	HSTATE_IMMUNEFIRE2			= 251,	//���߻��˺�2
	HSTATE_IMMUNESOIL2			= 252,	//�������˺�2
	HSTATE_RETORT2				= 253,
	HSTATE_ADDATTACKDEFENDDEGREE= 254,
	HSTATE_PALSY				= 255,	//̱���������
	HSTATE_INCHURT4				= 256,
	HSTATE_INCBECRITRATE		= 257,
	HSTATE_MODIFYSPECSKILLPRAY	= 258,
	HSTATE_INCSPECSKILLDAMAGE	= 259,
	HSTATE_ICESHIELD2			= 260,
	HSTATE_FIRESHIELD2			= 261,
	HSTATE_HEALSHIELD			= 262,
	HSTATE_INCFLYSPEED			= 263,
	HSTATE_INCVIGOUR			= 264,
	HSTATE_INCVIGOUR2			= 265,
	HSTATE_MOVEPUNISH			= 266,
	HSTATE_STANDPUNISH			= 267,
	HSTATE_STANDPUNISH2			= 268,
	HSTATE_CHANTSHIELD			= 269,
	HSTATE_INTERVALPALSY		= 270,
	HSTATE_INTERNALINJURY		= 271,
	HSTATE_ATKDAMAGEREDUCE		= 272,
	HSTATE_DEATHRESETCD			= 273,
	HSTATE_APPENDENCHANT		= 274,
	HSTATE_APPENDDAMAGE			= 275,
	HSTATE_COOLDOWNAWARD		= 276,
	HSTATE_HUNTERSOUL			= 277,
    HSTATE_SHADOWFORM           = 278,
    HSTATE_FAIRYFORM            = 279,
    HSTATE_ADDFROSTEFFECT       = 280,
    HSTATE_MOONGOD              = 281,
    HSTATE_ENHANCESKILLDAMAGE3  = 282,
	HSTATE_CRITDAMAGEREDUCE		= 283,
    HSTATE_INCPHYSICALDAMAGE    = 284,
    HSTATE_INCMAGICALDAMAGE     = 285,
	HSTATE_INCHURT5				= 286,
	HSTATE_BLACKWHITE			= 287,
	HSTATE_COOLDOWNPUNISH		= 288,
	HSTATE_ANTICLEARBUF			= 289,
	HSTATE_INCENCHANTRANGE		= 290,
    HSTATE_INCPHYSICALMAGICALDEFENSE = 291,
    HSTATE_REDUCEGOLD3          = 292,
    HSTATE_REDUCEWATER3         = 293,
    HSTATE_GENHPAP              = 294,
};

#define IMMUNEPHYSICAL 0x00000001   //����������
#define IMMUNEMETAL    0x00000002   //���߽�ϵ
#define IMMUNEWOOD     0x00000004   //����ľϵ
#define IMMUNEWATER    0x00000008   //����ˮϵ
#define IMMUNEFIRE     0x00000010   //���߻�ϵ
#define IMMUNESOIL     0x00000020   //������ϵ
#define IMMUNEDIZZY    0x00000040   //���߻���
#define IMMUNESLEEP    0x00000080   //����˯��
#define IMMUNESLOW     0x00000100   //���߼���
#define IMMUNEFIX      0x00000200   //���߶���
#define IMMUNECURSED   0x00000400   //���߷�ӡ�����ܹ�����
#define IMMUNEWEAK     0x00000800   //�����˺��ӱ�
#define IMMUNEREPEL    0x00001000   //���߻���
#define IMMUNEWOUND    0x00002000   //����HP�����½�����������HP
#define IMMUNEBREAK    0x00004000   //���ߴ�ϵ�ǰsession
#define IMMUNEDIRECT   0x00008000   //����behurt
#define IMMUNECLEAR    0x00010000   //�������filter
#define IMMUNEBLOODING 0x00020000	//������Ѫ			lgc
#define IMMUNEALL	   0x00040000	//�������и���״̬
#define IMMUNEDROP	   0x00080000	//���ߵ�����Ʒ����
#define IMMUNEPALSY	   0x00100000   //����̱��

enum
{
	MULTIOBJ_EFFECT_HEAL,
	MULTIOBJ_EFFECT_ATTACK,
	MULTIOBJ_EFFECT_ATTACK2,
};

enum
{
	STIME_GFX_MOVEPUNISH = 1,
	STIME_GFX_INTERNALINJURY,
	STIME_GFX_HUNTERSOUL,
};

}

#endif
