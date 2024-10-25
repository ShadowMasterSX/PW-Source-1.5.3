package expr;
import java.lang.*;
import java.util.*;
public class SyntaxTree
{
	private TreeNode root;
	public SymbolTable symtab;

	public SyntaxTree()
	{
		symtab=new SymbolTable();
	}

	public TreeNode GetRoot() 
	{
		return root;
	}
	public void SetRoot(TreeNode _root)
	{
		root=_root;
	}
	
	public static TreeNode CreateStmtNode(int kind,int lineno)
	{
		TreeNode t=new TreeNode();
		t.nodekind = TreeNode.StmtK;
		t.kind = kind;
		t.lineno = lineno;
		return t;
	}
	public static TreeNode CreateExprNode(int kind,int lineno)
	{
		TreeNode t=new TreeNode();
		t.nodekind = TreeNode.ExpK;
		t.kind = kind;
		t.lineno = lineno;
		return t;
	}	
	public static void DisplayWholeTree(TreeNode node,int indent)
	{
		if (node == null) return;
		
		TreeNode.DisplayTreeNode(node,indent);

		for (int i=0;i<node.child.size();i++)
			DisplayWholeTree((TreeNode)node.child.get(i),indent+4);
		if (node.sibling != null) DisplayWholeTree(node.sibling,indent);		
	}
}
