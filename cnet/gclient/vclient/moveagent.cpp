#include "moveagent.h"
#include "dbgprt.h"

unsigned char DirConvert(const A3DVECTOR & dir)
{
	if(dir.x < 1e-6 && dir.x > -1e-6)
	{
		if(dir.z > 0.f) return 64;
		else return 192;
	}

	float rad = atanf(dir.z/dir.x);
	if(dir.x < 0) rad += 3.1416f;
	else if(dir.z < 0) rad += 2*3.1416f;
	return (unsigned char)(rad/(2*3.1416f)*256);
}

void MoveAgent::Init()
{
	FILE * file = fopen("move.data","r");	
	if(!file) return;

	int cnt=0;
	char buf[256];
	while(fgets(buf,256,file))
	{
		A3DVECTOR pos(0.f,0.f,0.f);
		if(sscanf(buf,"%f:%f:%f",&pos.x,&pos.y,&pos.z) <= 0) continue;
		Learn(pos);	
		++cnt;
	}
	__PRINTINFO("\tmove.data read cnt=%d\n",cnt);
	
	fclose(file);
}

void MoveAgent::Quit()
{
	FILE * file = fopen("move.data","w");
	if(!file) return;
	
	SpinLock::Scoped l(lock);
	int cnt=0;
	for(int i=0; i<MOVEAGENT_ROW; i++)
		for(int j=0; j<MOVEAGENT_COL; j++)
		{
			if(reachable_table[i][j] == NULL) continue;
			A3DVECTOR & pos = *(reachable_table[i][j]);
			fprintf(file, "%f:%f:%f\n", pos.x,pos.y,pos.z);
			++cnt;
		}
	__PRINTINFO("\tmove.data wirte cnt=%d\n",cnt);

	fclose(file);
}

void MoveAgent::Learn(const A3DVECTOR & rpos)
{
	int row, col;
	if(!Locate(rpos, row, col)) return;

	SpinLock::Scoped l(lock);
	if(reachable_table[row][col] != NULL)
	{
	}
	else
	{
		reachable_table[row][col] = new A3DVECTOR(rpos.x, rpos.y, rpos.z);
	}
}

bool MoveAgent::GetMovePos(const A3DVECTOR & cur_pos, const A3DVECTOR & dest_pos, float range, A3DVECTOR & next_pos)
{
	if(cur_pos.squared_distance(dest_pos) <= range*range)
	{
		next_pos = dest_pos;	
		return true;
	}
	A3DVECTOR dir = dest_pos;
	dir -= cur_pos;
	return GetMovePos(cur_pos,DirConvert(dir),range,next_pos);
}

bool MoveAgent::GetMovePos(const A3DVECTOR & cur_pos, unsigned char dir, float range, A3DVECTOR & next_pos)
{
	int row, col;
	if(!Locate(cur_pos, row, col)) return false;
	range *= range;
	
	SpinLock::Scoped l(lock);
	bool ret = false;
	int tmp_row, tmp_col;
	A3DVECTOR tmp_pos;
	std::vector<off_node_t> & off_list = off_node_list[dir/32];
	for(int i=off_list.size()-1; i>=0; i--)
	{
		tmp_row = row + off_list[i].z_off;
		tmp_col = col + off_list[i].x_off;
		if(tmp_row < 0 || tmp_row > MOVEAGENT_ROW-1 || tmp_col < 0 || tmp_col > MOVEAGENT_COL-1)
			continue;
		if(!reachable_table[tmp_row][tmp_col])
			continue;
		tmp_pos = *(reachable_table[tmp_row][tmp_col]);	
		if(cur_pos.squared_distance(tmp_pos) > range)
			continue;

		ret = true;
		next_pos = tmp_pos;
		break;
	}
	
	return ret;
}

bool MoveAgent::Locate(const A3DVECTOR & pos, int & row, int & col)
{
	row = int((pos.z - MOVEAGENT_Z_START)/MOVEAGENT_STEP);
	col = int((pos.x - MOVEAGENT_X_START)/MOVEAGENT_STEP);
	if(row < 0 || row > MOVEAGENT_ROW-1 || col < 0 || col > MOVEAGENT_COL-1)
	{
		row = col = 0;
		return false;
	}
	return true;
}

