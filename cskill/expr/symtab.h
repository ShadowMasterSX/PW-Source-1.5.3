#ifndef _GNET_SYMTAB_H
#define _GNET_SYMTAB_H
#include "treenode.h"
#include <vector>

#include "visitors.h"
namespace GNET
{
	class SymbolTable
	{
		std::vector<TreeNode*> symtab;
	public:	
		SymbolTable() { symtab.clear(); }
		~SymbolTable()
		{
			for (size_t i=0;i<symtab.size();i++)
			{
				TreeNode* var_node=symtab[i];
				if (var_node->type > MAX_RESERVED_TYPE)
				{
					//printf("Destroy node: type=%d, pval=%p, name=%s\n",var_node->type,var_node->value.pval,var_node->name.data());
					OperationVisitor::DestroyCO((CommonObject*)var_node->value.pval);
				}
				if (var_node->struct_type == TreeNode::ArrayK)
				{
					//printf("Destroy array node: type=%d, pval=%p, name=%s\n",var_node->type,var_node->value.pval,var_node->name.data());
					OperationVisitor::DestroyArray((CommonObject*)var_node->value.pval,var_node->type);
				}
			}
			symtab.clear();
		}
		void swap(SymbolTable& _rhs) { symtab.swap(_rhs.symtab); }
		bool insert(TreeNode* node)
		{
			if (node==NULL) return false;
			if (lookup(node->name)!=NULL) return false;
			symtab.push_back(node);
			return true;
		}
		TreeNode* lookup(const std::string& identifier)
		{
			for (size_t i=0;i<symtab.size();i++)
				if (symtab[i]->name.compare(identifier)==0) return symtab[i];
			return NULL;
		}
	};
};
#endif
