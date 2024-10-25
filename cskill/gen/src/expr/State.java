package expr;
import java.lang.*;
public final class State
{
	public final static int START=0;
	public final static int INASSIGN=1;
	public final static int INNUM=2;
	public final static int INFLOAT=3;
	public final static int INID=4;
	public final static int DONE=5;
	public final static int INLT=6;
	public final static int INGT=7;
	public final static int INNE=8;
	
	public final static int RDYINCMNT=20;
	public final static int INCOMMENT=21;
	public final static int RDYOUTCMNT=22;
}

