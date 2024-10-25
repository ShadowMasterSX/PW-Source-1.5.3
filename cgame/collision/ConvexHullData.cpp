#include "ConvexHullData.h"
#include "Face.h"
#include "A3DMatrix.h"

#ifdef BMAN_EDITOR
#include "A3DGDI.h"
#endif

namespace SvrCD
{

// 文件输出的版本控制！字符串的长度必须相同！
// old versions
#define CHDATA_HEAD_TAG_V00	"==========Convex_Hull_Data_Start========="
#define CHDATA_HEAD_TAG_V01	"V0.1======Convex_Hull_Data_Start========="

// current version
#define CHDATA_HEAD_TAG	       "V0.2======Convex_Hull_Data_Start========="



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConvexHullData::CConvexHullData()
{
	m_dwFlags=0;
}

CConvexHullData::~CConvexHullData()
{
	for(int i=0;i<(int)m_arrFaces.size();i++)
		delete m_arrFaces[i];
}

CConvexHullData::CConvexHullData(const CConvexHullData& CHData)
{
	int i;
	for(i=0;i<CHData.GetVertexNum();i++)
		AddVertex(CHData.m_arrVertices[i]);
	for(i=0;i<CHData.GetFaceNum();i++)
		AddFace(*(CHData.m_arrFaces[i]));
	
	m_dwFlags=CHData.m_dwFlags;
}


void CConvexHullData::AddFace(const CFace& f)
{	
		CFace* pFace = new CFace(f);
		m_arrFaces.push_back(pFace);
}


bool CConvexHullData::LoadBinaryData(FILE* InFile)
{
	if(!InFile)
	{
		return false;
	}

	//读出标志，从而检验！
	char *szCHDataFlag=CHDATA_HEAD_TAG;
	char szRead[60];
	memset(szRead,0,60);
	int iFlagLen=strlen(szCHDataFlag);
	
	fread(szRead, iFlagLen, sizeof(char), InFile);


	//判断标志
	if(!strcmp(szRead,szCHDataFlag))	
	{
		//为最新版本：CHDATA_HEAD_TAG，版本2
		m_iVer=2;
	}
	else
	{
		szCHDataFlag=CHDATA_HEAD_TAG_V01;		
		if(!strcmp(szRead,szCHDataFlag))
		{
			//为版本：CHDATA_HEAD_TAG_V01
			m_iVer=1;
		}
		else
		{
			szCHDataFlag=CHDATA_HEAD_TAG_V00;		
			if(!strcmp(szRead,szCHDataFlag))
			{
				//为版本：CHDATA_HEAD_TAG_V00
				m_iVer=0;
			}
			else
				m_iVer=-1;
		}
	}

	switch(m_iVer) 
	{

	case 2:
	case 1:
		//读出标志位
		fread(&m_dwFlags, sizeof(DWORD), 1, InFile);
		break;
	case 0:
		m_dwFlags=0;
		break;
	case -1:
		return false;
		break;
	}

	//读出缓冲区大小
	int BufSize;
	fread(&BufSize, sizeof(int), 1, InFile);
	
	//开始读出数据
	char * buf= new char[BufSize];
	fread(buf, BufSize, sizeof(char), InFile);

	_ReadFromBuf(buf);
	delete [] buf;

	// build the convex hull's aabb
	_BuildAABB();


	return true;
}

bool CConvexHullData::_ReadFromBuf(char* buf)
{
	const int IntSize = sizeof(int);
	const int FloatSize = sizeof(float);

	if(!buf) return false;

	int iRead;
	int cur=0;
	float fRead;
	int i, j;
	//读出顶点数量
	iRead=* (int *) (buf+cur);
	cur+=IntSize;
	int vNum=iRead;

	//读出顶点坐标信息
	for(i=0;i<vNum;i++)
	{
		A3DVECTOR3 v;
		fRead=* (float *) (buf+cur);
		cur+=FloatSize;
		v.x=fRead;

		//memcpy(&fRead,buf+cur,FloatSize);
		fRead=* (float *) (buf+cur);
		cur+=FloatSize;
		v.y=fRead;

		//memcpy(&fRead,buf+cur,FloatSize);
		fRead=* (float *) (buf+cur);
		cur+=FloatSize;
		v.z=fRead;
		m_arrVertices.push_back(v);
	}

	//读出面片数量
	iRead=* (int *) (buf+cur);
	cur+=IntSize;
	int fNum=iRead;
	for(i=0;i<fNum;i++)
	{
		CFace *pFace = new CFace;
		
		//读出面片的平面方程参数
		A3DVECTOR3 n;
		float d;
		fRead=* (float *) (buf+cur);
		cur+=FloatSize;
		n.x=fRead;

		//memcpy(&fRead,buf+cur,FloatSize);
		fRead=* (float *) (buf+cur);
		cur+=FloatSize;
		n.y=fRead;

		//memcpy(&fRead,buf+cur,FloatSize);
		fRead=* (float *) (buf+cur);
		cur+=FloatSize;
		n.z=fRead;

		//memcpy(&fRead,buf+cur,FloatSize);
		fRead=* (float *) (buf+cur);
		cur+=FloatSize;
		d=fRead;

		pFace->SetNormal(n);
		pFace->SetD(d);

		//读出顶点的数量
		//memcpy(&iRead,buf+cur,IntSize);
		iRead=* (int *) (buf+cur);
		cur+=IntSize;
		int fvNum=iRead;
		
		for(j=0;j<fvNum;j++)
		{
			int id;
			CHalfSpace hs;
			A3DVECTOR3 n;
			float d;
			
			//读出顶点id
			//memcpy(&iRead,buf+cur,IntSize);
			iRead=* (int *) (buf+cur);
			cur+=IntSize;
			id=iRead;
			
			//读出相应的边平面方程
			//memcpy(&fRead,buf+cur,FloatSize);
			fRead=* (float *) (buf+cur);
			cur+=FloatSize;
			n.x=fRead;

			//memcpy(&fRead,buf+cur,FloatSize);
			fRead=* (float *) (buf+cur);
			cur+=FloatSize;
			n.y=fRead;

			//memcpy(&fRead,buf+cur,FloatSize);
			fRead=* (float *) (buf+cur);
			cur+=FloatSize;
			n.z=fRead;

			//memcpy(&fRead,buf+cur,FloatSize);
			fRead=* (float *) (buf+cur);
			cur+=FloatSize;
			d=fRead;

			hs.SetNormal(n);
			hs.SetD(d);
			
			pFace->AddElement(id,hs);
		}
		
		if(m_iVer>1)
		{
			// 版本2以上的，将包含额外面片信息
			
			// 读出额外面片的数量
			iRead=* (int *) (buf+cur);
			cur+=IntSize;
			int iExtraFaceNum=iRead;
			
			// 读出额外面片信息
			for(j=0; j<iExtraFaceNum; j++)
			{
				CHalfSpace hs;
				A3DVECTOR3 n;
				float d;
				
				//读出相应的边平面方程
				//memcpy(&fRead,buf+cur,FloatSize);
				fRead=* (float *) (buf+cur);
				cur+=FloatSize;
				n.x=fRead;

				//memcpy(&fRead,buf+cur,FloatSize);
				fRead=* (float *) (buf+cur);
				cur+=FloatSize;
				n.y=fRead;

				//memcpy(&fRead,buf+cur,FloatSize);
				fRead=* (float *) (buf+cur);
				cur+=FloatSize;
				n.z=fRead;

				//memcpy(&fRead,buf+cur,FloatSize);
				fRead=* (float *) (buf+cur);
				cur+=FloatSize;
				d=fRead;

				hs.SetNormal(n);
				hs.SetD(d);
				
				pFace->AddExtraHS(hs);
			}
			
		}

		m_arrFaces.push_back(pFace);
	}


	return true;
}

void CConvexHullData::_BuildAABB()
{
	m_aabb.Build( m_arrVertices.begin(), (int)m_arrVertices.size());	
}


void CConvexHullData::Transform(const A3DMATRIX4& mtxTrans)
{
	// 变换顶点
	int i;
	for(i=0;i<(int)m_arrVertices.size();i++)
	{
		m_arrVertices[i]=m_arrVertices[i]*mtxTrans;
	}

	// 变换面片
	for(i=0;i<(int)m_arrFaces.size();i++)
	{
		m_arrFaces[i]->Transform(mtxTrans);
	}

	// after transformation, we should rebuild the convex hull's aabb.
	_BuildAABB();
}

#ifdef BMAN_EDITOR
void CConvexHullData::Render(A3DFlatCollector* pFC,  bool bRenderV,CFace* pSpecialFace, DWORD dwVColor,DWORD dwFColor, const A3DVECTOR3& vRayDir, const A3DMATRIX4* pTransMatrix)const
{
	if(!pFC) return;

	if( m_arrVertices.size() == 0 ) return;

	int i;
	//绘制顶点
	if(bRenderV)
	{
		A3DVECTOR3 v;
		for(i=0;i<(int)m_arrVertices.size();i++)
		{
			if(pTransMatrix)
				v=m_arrVertices[i]*(*pTransMatrix);
			else
				v=m_arrVertices[i];
			g_pA3DGDI->Draw3DPoint(v,dwVColor,3.0f);
		}
	}

	//绘制面片
	A3DVECTOR3 RayDir;
	//保留面片色彩dwFColor的alpha值
	DWORD ALPHA=dwFColor & 0xff000000;

	Normalize(vRayDir,RayDir);

	for(i=0;i< (int)m_arrFaces.size();i++)
	{
		int VNum=m_arrFaces[i]->GetVNum();
		A3DVECTOR3* Vs=new A3DVECTOR3[VNum];
		int IDNum=(VNum-2)*3;
		WORD* IDs=new WORD[IDNum];
		int cur=0;
		//构造顶点和三角形索引
		for(int j=0;j<VNum;j++)
		{
			if(pTransMatrix)
				Vs[j]=m_arrVertices[m_arrFaces[i]->GetVID(j)]*(*pTransMatrix);
			else
				Vs[j]=m_arrVertices[m_arrFaces[i]->GetVID(j)];
			if(j<VNum-2)
			{
				IDs[cur]=0;
				IDs[cur+1]=j+1;
				IDs[cur+2]=j+2;
				cur+=3;
			}
		}
		
		//利用光照计算颜色
		A3DCOLORVALUE faceColorV(dwFColor);
		//双向光源，为了区分不同的面片！
		float fdot = DotProduct(m_arrFaces[i]->GetNormal(),RayDir);
		if(fdot < 0.0f) fdot = 0.0f;
		float fWeight=fdot*0.8f;
		faceColorV*=0.2f+fWeight;
		DWORD faceColor;
		if(pSpecialFace==m_arrFaces[i])
		{
			faceColor=faceColorV.ToRGBAColor();
			faceColor=0xffffffff-faceColor;
			faceColor=faceColor & 0x00ffffff  | ALPHA;
		}
		else
			faceColor=faceColorV.ToRGBAColor() & 0x00ffffff  | ALPHA;

		//绘制
		pFC->AddRenderData_3D(Vs,VNum,IDs,IDNum,faceColor);

		delete [] IDs;
		delete [] Vs;
	}
}
#endif


}
