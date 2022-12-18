#include "translator.h"

#ifdef LOG_MODE
    FILE* LogFile = startLog(LogFile);
#endif
int main(void)
{
	FILE* standart = fopen("obj/test.awp", "r");

	Tree_t  stTree = {};
	stTree.root = ReadStandart(standart);

	fclose(standart);
	TreeUpdate(&stTree, stTree.root);
	TreeDump(&stTree);

	standart = fopen("obj/test.arm", "w+");
	ArmStat(stTree.root, standart);
	
	fclose(standart);

	TreeDtor(&stTree);

#ifdef LOG_MODE
    endLog(LogFile);
#endif
	return 0;
}

void ArmStat(TreeNode_t* node, FILE* trans)
{
	assert(node && trans);
	
	if (node->parent)
	{
		fprintf(trans, "\nbarev\n");
	}

	TreeNode_t* curNode = node;
	while (curNode)
	{
		switch(curNode->left->type)
		{
			case Type_STAT:   ArmStat   (curNode->left, trans); break;
			case Type_DEF:    ArmDef	(curNode->left, trans); break;
			case Type_ASSIGN: ArmAssign (curNode->left, trans); break;
			case Type_FUNC:	  ArmFunc	(curNode->left, trans); break;
			case Type_PRINTF: ArmPrintf (curNode->left->left, trans); break;
			case Type_SCANF:  ArmScanf  (curNode->left->left, trans); break;
			case Type_CALL:   ArmCall   (curNode->left, trans); break;
			case Type_IF:     ArmIf     (curNode->left, trans); break;
			case Type_WHILE:  ArmWhile  (curNode->left, trans); break;
			case Type_RETURN: ArmRet    (curNode->left, trans); break;
			
			default: curNode = NULL; 
		}
		
		if (curNode)
			curNode = curNode->right;
	}

	if (node->parent)
	{
		fprintf(trans, "ctesutyun\n");
	}			
}

void ArmDef(TreeNode_t* node, FILE* trans)
{
	fprintf(trans, "axper %s nshanakuma ", node->left->name);
	
	ArmExp(node->right, trans);

	fprintf(trans, "\n");	
}

void ArmAssign(TreeNode_t* node, FILE* trans)
{
	fprintf(trans, "\n%s nshanakuma ", node->left->name);
	ArmExp(node->right, trans);

}

void ArmRet(TreeNode_t* node, FILE* trans)
{
	fprintf(trans, "\nveradardz ");
	ArmExp(node->left, trans);
	fprintf(trans, " verj\n");
}

void ArmPrintf(TreeNode_t* node, FILE* trans)
{
	fprintf(trans, "\ntpek bac");
	ArmParam(node->left, trans);
	fprintf(trans, "\ntpek pag");
}

void ArmParam(TreeNode_t* node, FILE* trans)
{
	TreeNode_t* curNode = node;
	
	if (node->left)
	fprintf(trans, "%s ", node->left->name);
	
	while (curNode->right)
	{
		curNode = curNode->right;
		fprintf(trans, "storaket %s ", curNode->left->name);
	}
}	
			

void ArmExp(TreeNode_t* node, FILE* trans)
{
	if (node->left)
		ArmExp(node->left, trans);
	if (node->right)
		ArmExp(node->right, trans);

	switch (node->type)
	{
		case Type_VAR: fprintf(trans, "%s ", node->name); break;
		case Type_NUM: fprintf(trans, "%lg ", node->numVal); break;
		case Type_OP:  ArmOp(node, trans); break;
		case Type_CALL: ArmCall(node, trans); break;

		default: break;
	}
}

void ArmOp(TreeNode_t* node, FILE* trans)
{
	switch (node->opVal)
	{
		case Op_Add: fprintf(trans, "gumarats ");   break;
		case Op_Sub: fprintf(trans, "hanats ");     break;
		case Op_Mul: fprintf(trans, "basmapatkel"); break;
		case Op_Div: fprintf(trans, "bajanel");     break;
		case Op_Pow: fprintf(trans, "astijan ");    break;
		case Op_Sin: fprintf(trans, "sin ");        break;
		case Op_Cos: fprintf(trans, "cos ");        break;
		case Op_Ln:  fprintf(trans, "ln ");         break;
		case Op_IsBt: fprintf(trans, "pakas ");     break;

		default: break;
	}
}

void ArmScanf(TreeNode_t* node, FILE* trans)
{
	fprintf(trans, "\ntpek bac");
	ArmParam(node->left, trans);
	fprintf(trans, "\ntpek pag");
}

void ArmIf(TreeNode_t* node, FILE* trans)
{
	fprintf(trans, "\nete bac ");
	ArmExp(node->left, trans);
	fprintf(trans, " pag aysdepkum\n");

	if (node->right->type != Type_ELSE)
	{
		ArmStat(node->right, trans);
	}
	else
	{
		ArmStat(node->right->left, trans);
		fprintf(trans, "\nhakarak");
		ArmStat(node->right->right, trans);
	}
	
	fprintf(trans, "verj\n");
}

void ArmWhile(TreeNode_t* node, FILE* trans)
{
	fprintf(trans, "\nkani bac ");
	ArmExp(node->left, trans);
	fprintf(trans, " pag\n");

	ArmStat(node->right, trans);
	fprintf(trans, "verj\n");
}

void ArmFunc(TreeNode_t* node, FILE* trans)
{
	fprintf(trans, "\n");
	if (STR_EQ("TYPE", node->left->right->name))
	{
		fprintf(trans, "krknaki");
	}
	else
	{
		fprintf(trans, "datark");
	}
	fprintf(trans, " %s bac ", node->left->name);
	
	if (node->left->left)
		ArmParam(node->left->left, trans);
	
	fprintf(trans, " pag");

	ArmStat(node->right, trans);
}

void ArmCall(TreeNode_t* node, FILE* trans)
{
	fprintf(trans, " %s bac ", node->left->name);
	
	if (node->left->left)
		ArmParam(node->left->left, trans);
	
	fprintf(trans, " pag");
}


