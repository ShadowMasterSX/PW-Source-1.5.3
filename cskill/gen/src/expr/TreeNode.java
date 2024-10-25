package expr;
import java.lang.*;
import java.util.*;

public class TreeNode
{
	//Define NodeKind
	public static final int StmtK=0;
	public static final int ExpK=1;

	//Define Statement kind
	public static final int IfK=1;
	public static final int ReadK=2;
	public static final int WriteK=3;
	public static final int RepeatK=4;
	public static final int AnnounceK=5;
	
	//Define Expression Kind
	public static final int OpK=1;
	public static final int	ConstIntK=2;
	public static final int ConstFloatK=3;
	public static final int ConstArrayK=4;
	public static final int IdK=5;
	public static final int FunctionK=6;
	
	//Define Struct-Type Kind
	public static final int ArrayK=1;
	public static final int VarK=2;
	public static final int PointerK=3;
	public static final int RefK=4;	
	
	//Define maximum children number
	public static final int MAXCHILDREN=3; 

	//Define members
	public ArrayList	child;
	public TreeNode  	sibling;
	public int			lineno;
	public int			nodekind;
	public int			kind;
	public int			op;
	public int			val;
	public float 		fval;

	public String		name;
	public int			type;
	public int 			struct_type;
	
	TreeNode()
	{
		child = new ArrayList();
		child.ensureCapacity(MAXCHILDREN);
	}
	static void DisplayTreeNode(TreeNode node,int indent)
	{
		if (node == null) return;
		int i=indent;
		while (i!=0) { System.out.print(" "); i--; }
		if (node.nodekind == StmtK)
		{
			switch (node.kind)
			{
				case TreeNode.IfK:
					Token.DisplayToken(Token.IF,"if");
					break;
				case TreeNode.RepeatK:
					Token.DisplayToken(Token.REPEAT,"repeat");
					break;
				case TreeNode.AnnounceK:   
					Token.DisplayToken(Token.ANNOUNCE,node.name);
					break;
				case TreeNode.ReadK: 
					Token.DisplayToken(Token.READ,node.name);
					break;
				case TreeNode.WriteK:
					Token.DisplayToken(Token.WRITE,"write");
					break;

			}
		}		
		if (node.nodekind == ExpK)
		{
			switch (node.kind)
			{
				case TreeNode.ConstIntK: 
					Token.DisplayToken(Token.NUM,String.valueOf(node.val));
					break;
				case TreeNode.ConstFloatK:
					Token.DisplayToken(Token.FLOAT,String.valueOf(node.fval));
					break;
				case TreeNode.ConstArrayK:
					Token.DisplayToken(Token.ARRAY,"");
					break;	
				case TreeNode.IdK:
					Token.DisplayToken(Token.ID,node.name);
					break;
				case TreeNode.OpK:
					Token.DisplayToken(node.op,"");
					break;
				case TreeNode.FunctionK:
					Token.DisplayToken(Token.FUNCTION,node.name);
					break;
			}
		}

	}		
}
