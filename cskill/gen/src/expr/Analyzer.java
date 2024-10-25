package expr;
import java.lang.*;
import java.util.*;

public class Analyzer
{
	static private ObjectGraph og;

	static private TreeNode 	current_treenode;
	static private int			current_type;
	static Analyzer instance=new Analyzer();

	static private SyntaxTree st;
	static private int	context_state;
	
	private Analyzer()
	{
	}

	private void TypeCheck(int type1,int type2) throws AnalyzerException
	{
		TypeCheck(type1,type2,true);
	}
	private void TypeCheck(int type1,int type2,boolean strict) throws AnalyzerException
	{

		if(type1==Token.FUNCTION||type2==Token.FUNCTION)
			return;
		if (strict)
			if (type1 == type2)
				return;
			else
				throw new AnalyzerException("Type mismatch.",current_treenode.lineno);
		else
		{
			if (type1 == type2) return;
			/*
			//define types that can be convert
			if (type1 == og.GetIdentifier("int"))
				type1=og.GetIdentifier("float");
			else if (type1==og.GetIdentifier("float"))
				type1=og.GetIdentifier("int");
			*/
			if (ObjectGraph.reserved_type.containsValue(new Integer(type1)) && 
				ObjectGraph.reserved_type.containsValue(new Integer(type2)) )
			{
				if ( type1<type2 && current_treenode.nodekind==TreeNode.ExpK && current_treenode.kind==TreeNode.OpK && current_treenode.op==Token.ASSIGN )
				{
					//System.out.println("Warning: line "+current_treenode.lineno+". Conversion may lose accuration.");
				}
				return;
			}
			else
				throw new AnalyzerException("Type mismatch.",current_treenode.lineno);
		}
	}	
	private int OGCheck(int parent_type,int child_name)
	{
		return og.FindMemberNode(parent_type,child_name);
	}
	private void StmtCheck(TreeNode t) throws AnalyzerException
	{
		if (t==null) return;
		current_treenode=t;
		for (int i=0;i<t.child.size();i++)
		{
			StmtCheck((TreeNode)t.child.get(i));
		}	
		current_treenode=t; //reassign current_treenode,because it is changed in recursive calls.
		if (t.nodekind == TreeNode.StmtK)
		{
			switch(t.kind)
			{
				case TreeNode.IfK:
				case TreeNode.RepeatK:
				case TreeNode.ReadK:
				case TreeNode.WriteK:
					break;
				case TreeNode.AnnounceK:
					if (t.child.get(0)==null)
						throw new AnalyzerException("Invalid announcement statement.",current_treenode.lineno);
					break;	
			}
		}
		else if (t.nodekind == TreeNode.ExpK)
		{
			switch (t.kind)
			{
				case TreeNode.ConstIntK:
					t.type = og.GetIdentifier("int");
					if (t.type == Constants.INVALID_IDENTIFIER)
						throw new AnalyzerException("Cannot find 'int' type in OG.",current_treenode.lineno);
					break;
				case TreeNode.ConstFloatK:
					t.type = og.GetIdentifier("float");
					if (t.type == Constants.INVALID_IDENTIFIER)
						throw new AnalyzerException("Cannot find 'float' type in OG.",current_treenode.lineno);
					break;
				case TreeNode.ConstArrayK:
					if (context_state == ContextState._STATE_STMT)
						throw new AnalyzerException("Cannot define const array without Announcement.",current_treenode.lineno);
					if (t.child.get(0)==null)
						throw new AnalyzerException("Invalid array definition.",current_treenode.lineno);
					t.type=((TreeNode)t.child.get(0)).type;
					break;  					
				case TreeNode.IdK:
					if (t.child.size()!=0) //check by ObjectGraph
					{
						t.type=OGCheck(((TreeNode)t.child.get(0)).type,og.GetIdentifier(t.name));
						if (t.type == Constants.INVALID_IDENTIFIER) 
							throw new AnalyzerException("Invalid membership.",current_treenode.lineno);
					}
					else
					{
						t.type=OGCheck(current_type,og.GetIdentifier(t.name));
						if (t.type == Constants.INVALID_IDENTIFIER) 
						{
							TreeNode var_node=st.symtab.Lookup(t.name);
							if (var_node!=null)
							{
								t.type=var_node.type;
								t.struct_type=var_node.struct_type;
							}
							else
								throw new AnalyzerException("Invalid membership.",current_treenode.lineno);
						}
					}
					break;
				case TreeNode.OpK:
					switch (t.op)
					{
						case Token.ASSIGN:
							if (t.child.size()!=2) throw new AnalyzerException("Invalid assignment node.",current_treenode.lineno);
							TypeCheck(((TreeNode)t.child.get(0)).type,((TreeNode)t.child.get(1)).type,false);
							t.type=((TreeNode)t.child.get(0)).type;
							break;
						case Token.SELECT: // ? : 
						{
							if (t.child.size() != 3)	
								throw new AnalyzerException("Invalid select structure.",current_treenode.lineno);
							TypeCheck(((TreeNode)t.child.get(0)).type,og.GetIdentifier("bool"));
							int type1=((TreeNode)t.child.get(1)).type;
							int type2=((TreeNode)t.child.get(2)).type;
							if (type1<type2) { int tmp=type2; type2=type1; type1=tmp; }
							TypeCheck(type1,type2,false);
							t.type=type1;
							break;
						}
						case Token.MEMSL: //parent.child
							if (t.child.size()==0)
								throw new AnalyzerException("Invalid member(parent.child) structure.",current_treenode.lineno);
							t.type = ((TreeNode)t.child.get(0)).type;
							break;
						case Token.LSQRBRACKET: //index
							if (t.child.size()!=2) 
								throw new AnalyzerException("Invalid index(expr[*]) structure.",current_treenode.lineno);
							TypeCheck(((TreeNode)t.child.get(1)).type,og.GetIdentifier("int"));
							t.type=((TreeNode)t.child.get(0)).type;
							TreeNode var_node=st.symtab.Lookup(((TreeNode)t.child.get(0)).name);
							if (var_node==null)
							{
								if (context_state == ContextState._STATE_STMT)
								{
									t.type=og.FindMemberNode(((TreeNode)t.child.get(0)).type,og.GetIdentifier("value"));
									if (t.type == Constants.INVALID_IDENTIFIER)
										throw new AnalyzerException("Invalid array object. No 'value' field in OG file.",current_treenode.lineno);	
								}
								else if (context_state == ContextState._STATE_VAR_ANNC)
								{
									throw new AnalyzerException("Cannot define array of class that defined in OG.",current_treenode.lineno);
								}
							}
							else
							{
								if (context_state == ContextState._STATE_STMT)
								{
									if (var_node.struct_type != TreeNode.ArrayK)
										throw new AnalyzerException("Cann't do '[]' operation on non-array variable.",current_treenode.lineno);
								}
								else if (context_state == ContextState._STATE_VAR_ANNC)
								{
									if (t.type > ObjectGraph.MAX_RESERVED_TYPE)
										throw new AnalyzerException("cannnot create array of class which is defined in OG file.",current_treenode.lineno);
									if (!(((TreeNode)t.child.get(1)).nodekind==TreeNode.ExpK && ((TreeNode)t.child.get(1)).kind==TreeNode.ConstIntK))
										throw new AnalyzerException("Array maybe un-initialed.",current_treenode.lineno);		
									var_node.struct_type=TreeNode.ArrayK;
								}			
							}
							break;
						case Token.NE:	
						case Token.EQ:
						case Token.LT:
						case Token.GT:
						case Token.LTE:
						case Token.GTE:
							if (t.child.size() != 2)
								throw new AnalyzerException("Invalid logic operator structure.",current_treenode.lineno);
							TypeCheck(((TreeNode)t.child.get(0)).type,((TreeNode)t.child.get(1)).type,false);
							t.type=og.GetIdentifier("bool");
							break;
						case Token.AND:
						case Token.OR:
							if (t.child.size() != 2)
								throw new AnalyzerException("Invalid logic operator structure.",current_treenode.lineno);
							TypeCheck(((TreeNode)t.child.get(0)).type,((TreeNode)t.child.get(1)).type,false);
							t.type=og.GetIdentifier("bool");
							break;	
						case Token.PLUS:
						case Token.MINUS: 
						case Token.TIMES: 
						case Token.OVER:
						case Token.MOD:
						{
							if (t.child.size() != 2)
								throw new AnalyzerException("Invalid arithmetic operator structure.",current_treenode.lineno);
							int type1=((TreeNode)t.child.get(0)).type;
							int	type2=((TreeNode)t.child.get(1)).type;
							if(t.op==Token.MOD)
							{
								TypeCheck(type1,og.GetIdentifier("int"));
								TypeCheck(type2,og.GetIdentifier("int"));
							}
							else
							{
								if (type1<type2) { int tmp=type2; type2=type1; type1=tmp; }
								TypeCheck(type1,type2, false);
							}
							t.type=type1;
							break;	
						}
						default	:
							throw new AnalyzerException("Invalid node kind.",current_treenode.lineno);
					}
					break;	
			}
		}
		else
		{
			throw new AnalyzerException("Invalid node kind.",current_treenode.lineno);
		}
	}
	public static Analyzer GetInstance()
	{
		return instance;
	}
	static void Analyze(SyntaxTree _st,ObjectGraph _og,String start_point_type) throws AnalyzerException
	{
		og=_og;
		st=_st;
		if (_og == null) return;
		try
		{
			TreeNode t=_st.GetRoot();
			if (null==t) 
				throw new AnalyzerException("Syntax tree is empty.",0);
			current_type = og.GetIdentifier(start_point_type);
			if (current_type==Constants.INVALID_IDENTIFIER)
				throw new AnalyzerException("Current type ("+start_point_type+") is invalid.",0);
			while (t!=null)
			{
				current_treenode=t;
				if (t.nodekind == TreeNode.StmtK)
				{   
					switch (t.kind)
					{
						case TreeNode.AnnounceK:
							if (og.GetIdentifier(t.name)!=Constants.INVALID_IDENTIFIER)
								throw new AnalyzerException("Variable "+t.name+" has be defined in OG file.",t.lineno);
							if(! st.symtab.Insert(t)) 
								throw new AnalyzerException("Variable "+t.name+" has be defined before.",t.lineno);
							context_state=ContextState._STATE_VAR_ANNC;
							break;
						default:
							context_state=ContextState._STATE_STMT;
							break;
					}
				}
				else
				{
					context_state=ContextState._STATE_STMT;
				}  				
				GetInstance().StmtCheck(t);
				t=t.sibling;
			}
		}
		catch (AnalyzerException e)
		{
			System.out.println("Error: "+e.getMessage()+" at line "+e.getLineno());
			if (current_treenode != null) SyntaxTree.DisplayWholeTree(current_treenode,4);
			e.printStackTrace();
			throw e;
		}
	}
}
