#ifndef _GNET_SYNTAXTREE_H
#define _GNET_SYNTAXTREE_H
#include "treenode.h"
#include "symtab.h"
#include "../common/thread.h"

namespace GNET
{
class SyntaxTree
{
	class Collector  //this class is used for release allocated memory,when failure occurs during building SyntraxTree
	{
		std::vector<TreeNode*> dust_collector;
	public:
		~Collector() { dust_collector.clear(); }
		void Collect(TreeNode* t) { dust_collector.push_back(t); }
		//this function is used ONLY at failure occurs
		void Clean() { for(size_t i=0;i<dust_collector.size();delete dust_collector[i],i++); }
	};
	private:
		TreeNode* root;
		Collector collector;
		void ReleaseMemory(TreeNode* node)
		{
			if (node == NULL) return;
			for (int i=0;i<MAXCHILDREN;i++)
				if (node->child[i] != NULL) ReleaseMemory(node->child[i]);
			if (node->sibling != NULL) ReleaseMemory(node->sibling);
			//printf("release node %p\n",node);
			if (node !=NULL) delete node;
		}
	public:
		Thread::Mutex locker;
		SymbolTable symtab;

		SyntaxTree(TreeNode* _root=NULL) : root(_root) {  }
		~SyntaxTree()
		{
			//if (root!=NULL) printf("enter ~syntaxtree(). root=%p\n",root);
			ReleaseMemory(root);
		}
		void operator=(SyntaxTree& _rhs) { root=_rhs.root; symtab.swap(_rhs.symtab); }
		
		TreeNode* GetRoot() { return root; }
		void SetRoot(TreeNode* node) { root=node; }
		void ReleaseBranch(TreeNode* node) { ReleaseMemory(node); }

		TreeNode* CreateStmtNode(TreeNode::StmtKind kind,int lineno=0)
		{
			TreeNode* t=new TreeNode();
			t->nodekind = TreeNode::StmtK;
			t->kind.stmt = kind;
			t->lineno = lineno;
			//printf("Create new node %p\n",t);
			collector.Collect(t);
			return t;
		}
		TreeNode* CreateExprNode(TreeNode::ExpKind kind,int lineno=0)
		{
			TreeNode* t=new TreeNode();
			t->nodekind = TreeNode::ExpK;
			t->kind.exp = kind;
			t->lineno = lineno;
			//printf("Create new node %p\n",t);
			collector.Collect(t);
			return t;
		}
		//this function is used ONLY at failure occurs
		void CleanDust() { collector.Clean(); }
		static void DisplaySolitaryTree(TreeNode* node,unsigned int indent=0)
		{
			if (node == NULL) return;
			TreeNode::DisplayTreeNode(node,indent);
			for (int i=0;i<MAXCHILDREN;i++)
				if (node->child[i] != NULL) DisplaySolitaryTree(node->child[i],indent+4);
		}

		static void DisplayWholeTree(TreeNode* node,unsigned int indent=0)
		{
			if (node == NULL) return;
			TreeNode::DisplayTreeNode(node,indent);
			for (int i=0;i<MAXCHILDREN;i++)
				if (node->child[i] != NULL) DisplayWholeTree(node->child[i],indent+4);
			if (node->sibling != NULL) DisplayWholeTree(node->sibling,indent);
		}
};//end class	
};//end namespace
#endif
