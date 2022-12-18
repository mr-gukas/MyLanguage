#include "standart.h"

void PrintTreeToFile(TreeNode_t* node, FILE* standartFile)
{
	fprintf(standartFile, " { ");
	PrintNodeDataToFile(node, standartFile);
	
	if (node->left || node->right)
	{
		if (node->left)
			PrintTreeToFile(node->left, standartFile);
		else
			fprintf(standartFile, " { NIL } ");
		if (node->right)
			PrintTreeToFile(node->right, standartFile);
		else
			fprintf(standartFile, " { NIL } ");
	}

	fprintf(standartFile, " } ");

}

void PrintNodeDataToFile(TreeNode_t* node, FILE* standartFile)
{
	switch (node->type)
	{
		case Type_FUNC:
		case Type_PARAM:
		case Type_CALL:
		case Type_STAT:	   fprintf(standartFile, "%s",  node->name); break;
		case Type_PRINTF:  fprintf(standartFile, "OUT"); break;
		case Type_SCANF:   fprintf(standartFile, "IN"); break;
		case Type_RET_VAL:
		{
			if (STR_EQ(node->name, "krknaki"))
				fprintf(standartFile, "TYPE");
			else
				fprintf(standartFile, "VOID");
			break;
		}
		case Type_DEF: 	    fprintf(standartFile, "VAR");      break;
		case Type_ASSIGN:   fprintf(standartFile, "EQ");       break;
		case Type_NUM:      fprintf(standartFile, "%lg", node->numVal); break;
		case Type_FUNC_NAME:
		case Type_VAR:      fprintf(standartFile, "\"%s\"", node->name); break;
		case Type_OP:       PrintOpToFile(node, standartFile); break;
		case Type_RETURN:   fprintf(standartFile, "RET"); break;
		case Type_IF:	    fprintf(standartFile, "IF"); break;	
		case Type_ELSE:	    fprintf(standartFile, "ELSE"); break;	
		case Type_WHILE:    fprintf(standartFile, "WHILE"); break;	

		default: break;
	}
}

void PrintOpToFile(TreeNode_t* node, FILE* standartFile)
{
	switch(node->opVal)
	{
		case Op_Add: fprintf(standartFile, "ADD"); break; 
		case Op_Sub: fprintf(standartFile, "SUB"); break; 
		case Op_Mul: fprintf(standartFile, "MUL"); break; 
		case Op_Div: fprintf(standartFile, "DIV"); break; 
		case Op_Sin: fprintf(standartFile, "SIN"); break; 
		case Op_Cos: fprintf(standartFile, "COS"); break; 
		case Op_Pow: fprintf(standartFile, "POW"); break; 
		case Op_IsBt: fprintf(standartFile, "IS_BT"); break; 
		case Op_Ln : fprintf(standartFile, "LN"); break; 

		default: break;
	}
}

TreeNode_t* ReadStandart(FILE* standart)
{
	assert(standart);
	
	TEXT data        = {};
	size_t lineIndex = 1;
	textCtor(&data, standart);
	char* dataptr = data.buf;

	TreeNode_t* node = ReadTree(NULL, &dataptr);

	textDtor(&data);
	
	return node;
}


TreeNode_t* ReadTree(TreeNode_t* node, char** buf)
{
    char* data = (char*) calloc (STR_MAX_SIZE, sizeof(*data));
    double num             = 0;
	size_t lineIndex = 0;
	int    len       = 0;

    SkipSpaces(buf, &lineIndex);
	
	
    TreeNode_t* curNode = NULL; 

    if (**buf == '{')
    {
		(*buf)++; 
		SkipSpaces(buf, &lineIndex);

        if (sscanf(*buf, "%lg%n", &num, &len))
        {
            curNode = MakeNode(Type_NUM, Op_Null, num, NULL, NULL, NULL); 
			*buf += len;
        }
        else if (sscanf (*buf, "%s%n", data, &len))
        {
			if (strchr(data, '"') == NULL)
			{
				NodeType curType = GetStandartType(data);
				OperationType curOp    = GetStandartOpType(data);

				if (curType != Type_NULL)
					curNode = MakeNode(curType, curOp, 0, data, NULL, NULL);
				else
				{
					curNode = NULL;
				}
			}
			else
			{
				char name[STR_MAX_SIZE] = "";
				strcpy(name, data + 1);
				name[strlen(name) - 1] = '\0';

				if (node && (node->type == Type_FUNC  || node->type == Type_CALL))
				{
					curNode = MakeNode(Type_FUNC_NAME, Op_Null, 0, name, NULL, NULL);
				}
				else
				{
					curNode = MakeNode(Type_VAR, Op_Null, 0, name, NULL, NULL);
				}
			}

			*buf += len;
		}
	}
	
	free(data);
	if (curNode)
		curNode->parent = node;

    SkipSpaces(buf, &lineIndex);

	if (**buf == '}')
	{
		(*buf)++;
		return curNode;
	}

	if (**buf == '{')
	{
		curNode->left = ReadTree(curNode, buf);
	}

    SkipSpaces(buf, &lineIndex);

	if (**buf == '}')
	{
		(*buf)++;
		return curNode;
	}
	else if (**buf == '{')
	{
		curNode->right = ReadTree(curNode, buf);
	}
	
    SkipSpaces(buf, &lineIndex);
	
	if (**buf == '}')
	{
		(*buf)++;
		return curNode;
	}

	return curNode;
}




NodeType GetStandartType(const char* word)
{
#define KEY_WORD(name, c_name, type, standart)  \
	if (STR_EQ(standart, word))					     \
		return Type_##type;

#include "../utils/keywords.h"

#undef KEY_WORD

	return Type_NULL;
}

OperationType GetStandartOpType(const char* name)
{
	if (STR_EQ("ADD", name)) return Op_Add;
	if (STR_EQ("SUB", name)) return Op_Sub;
	if (STR_EQ("MUL", name)) return Op_Mul;
	if (STR_EQ("DIV", name)) return Op_Div;
	if (STR_EQ("SIN", name)) return Op_Sin;
	if (STR_EQ("COS", name)) return Op_Cos;
	if (STR_EQ("LN",  name)) return Op_Ln;
	if (STR_EQ("POW", name)) return Op_Pow;
	if (STR_EQ("IS_BT", name)) return Op_IsBt;

	
	return Op_Null;
}

void SkipFileSpaces(FILE* file)
{
	assert(file);

	char curChar = (char) fgetc(file);

    while (isspace(curChar)) 
    {
        curChar = (char) fgetc(file);
    }
    ungetc (curChar, file);
}


