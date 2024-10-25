package expr;
import java.io.*;
import java.util.*;

class SymbolTable
{
	private HashMap symtab;

	public SymbolTable() 
	{
		symtab=new HashMap();
	}

	boolean Insert(TreeNode treenode)
	{
		if (treenode==null) return false;
		if (symtab.containsKey(treenode.name)) return false;
		symtab.put(treenode.name,treenode);
		return true;
	}
	TreeNode Lookup(String name)
	{
		return (TreeNode)symtab.get(name);
	}
}
