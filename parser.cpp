#include "parser.h"

#ifdef LOG_MODE
    FILE* LogFile = startLog(LogFile);
#endif

int main(const int argc, const char* argv[])
{
	assert(argc == 2 && argv != NULL);
	
	FILE* code = fopen(argv[1], "r");
	assert(code != NULL);
	
	size_t        curIndex = 0;
	Tokens_t      tokens   = {};
	Tree_t        tree     = {};
	MemDefender_t memdef   = {};
	
	GetTokens(&tokens, code);
	MemDefenderCtor(&memdef, tokens.nodeCount);
	tree.root = GetCode(&tokens, &curIndex, &memdef);
	TreeUpdate(&tree, tree.root);
	
	TreeDump(&tree);
	
	TokensDtor(&tokens, &memdef);

	fclose(code);


#ifdef LOG_MODE
    endLog(LogFile);
#endif
	return 0;
}

static void MemDefenderCtor(MemDefender_t* memdef, size_t size)
{
	memdef->adr  = (TreeNode_t**) calloc(size, sizeof(TreeNode_t*));
	memdef->size = 0;
}

static void GetTokens(Tokens_t* tokens, FILE* code)
{
	
	TEXT data        = {};
	size_t lineIndex = 1;
	textCtor(&data, code);
	char* dataptr = data.buf;

	tokens->firstTok = (TreeNode_t**) calloc(data.nChar, sizeof(TreeNode_t*));
	

	for (int index = 0; index < data.nChar; index++)
	{
		if (index != data.nChar - 1 && *(dataptr + index) == '\0')
		{
			*(dataptr + index) = '\n';
		}
	}

	while (*dataptr)
	{
		if (isspace(*dataptr))
		{
			SkipSpaces(&dataptr, &lineIndex);
		}
		else if (isdigit(*dataptr))
		{
			double val    = 0;
			size_t valLen = 0;
			
			sscanf(dataptr, "%lf%n", &val, &valLen);
			*(tokens->firstTok + tokens->nodeCount) = MakeNode(Type_NUM, Op_Null, val, NULL, NULL, NULL, lineIndex);

			tokens->nodeCount++;

			dataptr += valLen;
		}
		else if (isalpha(*dataptr))
		{
			char word[30] = {};
			size_t  wordLen  = GetWord(word, dataptr);
			dataptr     += wordLen;
			
			NodeType type = GetType(word);

			if (type == Type_NULL)
			{
				SkipSpaces(&dataptr, &lineIndex);

				if (isalpha(*dataptr))
				{
					char nextWord[30] = {};
					GetWord(nextWord, dataptr);

					if (STR_EQ("bac", nextWord))
					{
						*(tokens->firstTok + tokens->nodeCount) = MakeNode(Type_FUNC, Op_Null, 0, word, NULL, NULL, lineIndex);
					}
					else
					{
						*(tokens->firstTok + tokens->nodeCount) = MakeNode(Type_VAR, Op_Null, 0, word, NULL, NULL, lineIndex);
					}
				}
				else
				{
					*(tokens->firstTok + tokens->nodeCount) = MakeNode(Type_VAR, Op_Null, 0, word, NULL, NULL, lineIndex);
				}
			}
			else
			{
				OperationType oper = GetOpType(word);
				*(tokens->firstTok + tokens->nodeCount) = MakeNode(type, oper, 0, word, NULL, NULL, lineIndex);
			}

			tokens->nodeCount++;		
		}
	}
	textDtor(&data);
}

static OperationType GetOpType(const char* name)
{
	if (STR_EQ("gumarats",    name)) return Op_Add;
	if (STR_EQ("hanats",      name)) return Op_Sub;
	if (STR_EQ("basmapatkel", name)) return Op_Mul;
	if (STR_EQ("bajanel",     name)) return Op_Div;
	if (STR_EQ("gumarats",    name)) return Op_Add;
	if (STR_EQ("sin",		  name)) return Op_Sin;
	if (STR_EQ("cos",		  name)) return Op_Cos;
	if (STR_EQ("ln",          name)) return Op_Ln;
	if (STR_EQ("astijan",     name)) return Op_Pow;
	
	return Op_Null;
}


static char* SkipSpaces(char** src, size_t* lineIndex)
{
	while (isspace(**src))
	{
		if (**src == '\n')
			(*lineIndex)++;
		++(*src);
	}

	return *src;
}

static size_t GetWord(char *dest, const char *src) 
{
    size_t wordLen = 0;

    if (isalpha(*src)) 
	{
        while (isalpha(*src))
		{
            *dest = *src;
            src++;
            dest++;

            wordLen++;
        }
    }        
    
	dest[wordLen] = '\0';

	return wordLen;
}

static NodeType GetType(const char* word)
{
#define KEY_WORD(name, c_name, type, num)  \
	if (STR_EQ(name, word))					\
		return Type_##type;

#include "keywords.h"

#undef KEY_WORD

	return Type_NULL;
}

static TreeNode_t* GetCode(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef)
{
	if ((*(tokens->firstTok + *curIndex))->type != Type_FUNC && 
		(*(tokens->firstTok + *curIndex))->type != Type_DEF)
	{
		fprintf(stderr, "False expression encountered. A variable or function declaration was expected (line %lu)\n", (*(tokens->firstTok + *curIndex))->lineIndex);
		abort();
	}

	TreeNode_t* node1 = GetAssign(tokens, curIndex, memdef);
	
	if (!node1)
	{
		TreeNode_t* nodeFunc = GetFunc(tokens, curIndex, memdef);	
		return nodeFunc;
	}
	
	SYNTAXCTRL("verj");
	TreeNode_t* stat1 = MakeNode(Type_STAT, Op_Null, 0, "ST", NULL, NULL);
	memdef->adr[(memdef->size)++] = stat1;

	TreeNode_t* root  = stat1;

	stat1->left   = node1;
	node1->parent = stat1;
		
	while ((*(tokens->firstTok + *curIndex))->type == Type_DEF)
	{
		TreeNode_t* node2 = GetAssign(tokens, curIndex, memdef);
		SYNTAXCTRL("verj");

		TreeNode_t* stat2 = MakeNode(Type_STAT, Op_Null, 0, "ST", NULL, NULL);
		memdef->adr[(memdef->size)++] = stat2;

		stat2->left   = node2;
		node2->parent = stat2;

		stat1->right  = stat2;
		stat2->parent = stat1;

		stat1 = stat2;
	}
	
	if ((*(tokens->firstTok + *curIndex))->type != Type_FUNC)
	{
		fprintf(stderr, "False expression encountered A variable or function declaration was expected (line %lu)\n", (*(tokens->firstTok + *curIndex))->lineIndex);
		abort();
	}

	TreeNode_t* nodeFunc = GetFunc(tokens, curIndex, memdef);
	stat1->right = nodeFunc;
	nodeFunc->parent = stat1;
	
	if (*curIndex + 1 < tokens->nodeCount)
	{
		fprintf(stderr, "Syntax error. An incomprehensible expression was encountered at the end of the programm (line %lu)\n",
						(*(tokens->firstTok + *curIndex))->lineIndex);
		abort();
	}
	return root;
}


static TreeNode_t* GetFunc(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef)
{
	assert(tokens);
	assert(curIndex);

	TreeNode_t* funcDef = GetFuncDef(tokens, curIndex, memdef);
	assert(funcDef != NULL);

	TreeNode_t* stat = MakeNode(Type_STAT, Op_Null, 0, "ST", NULL, NULL);
	memdef->adr[(memdef->size)++] = stat;

	stat->left      = funcDef;
	funcDef->parent = stat;

	while ((*(tokens->firstTok + *curIndex))->type == Type_FUNC)
	{
        TreeNode_t* funcDef2 = GetFuncDef(tokens, curIndex, memdef);
        TreeNode_t* stat2    = MakeNode(Type_STAT, Op_Null, 0, "ST", NULL, NULL);
		memdef->adr[(memdef->size)++] = stat2;
		
        
		stat2->left      = funcDef2;
        stat2->right     = stat;
        funcDef2->parent = stat2;
        funcDef->parent  = stat;

        stat = stat2;

		if (*curIndex + 1 >= tokens->nodeCount) break;
    }

    return stat;
}

static TreeNode_t* GetFuncDef(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef)
{
	TreeNode_t* func = MakeNode(Type_FUNC, Op_Null, 0, "FUNC", NULL, NULL);
	memdef->adr[(memdef->size)++] = func;
	
	if ((*(tokens->firstTok + *curIndex))->type == Type_FUNC)
	{
	
		TreeNode_t* funcName = *(tokens->firstTok + *curIndex);
		(*curIndex)++;
		
		SYNTAXCTRL("bac");

		TreeNode_t* arg = GetArg(tokens, curIndex, memdef);
		
		SYNTAXCTRL("pag");
		SYNTAXCTRL("barev");

		TreeNode_t* funcBody = GetStat(tokens, curIndex, memdef);
		
		SYNTAXCTRL("ctesutyun");
		
		if (*curIndex + 1 == tokens->nodeCount)
			(*curIndex) -= 1;
		else
			(*curIndex) -= 2;

		SYNTAXCTRL("verj");
		*curIndex += 1;
		
		func->left       = funcName;
		funcName->parent = func;

		funcName->left = arg;
		if (arg) arg->parent = func;

		func->right = funcBody;
		funcBody->parent = func;
	}

	return func; 
}


static TreeNode_t* GetArg(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef)
{
	TreeNode_t* param1 = NULL;

	if ((*(tokens->firstTok + *curIndex))->type == Type_VAR || (*(tokens->firstTok + *curIndex))->type == Type_NUM)
	{
		TreeNode_t* arg1 = *(tokens->firstTok + *curIndex);
		(*curIndex)++;
		
		param1 = MakeNode(Type_PARAM, Op_Null, 0, "PARAM", NULL, NULL);
		memdef->adr[(memdef->size)++] = param1;
		
		param1->left = arg1;
		arg1->parent = param1;
		
		while (STR_EQ("storaket", (*(tokens->firstTok + *curIndex))->name))
		{
			(*curIndex)++;
		
			TreeNode_t* arg2 = *(tokens->firstTok + *curIndex);
			(*curIndex)++;
		
			TreeNode_t* param2 = MakeNode(Type_PARAM, Op_Null, 0, "PARAM", NULL, NULL);
			memdef->adr[(memdef->size)++] = param2;

		
			param2->left  = arg2;
			param2->right = param1;

			arg2->parent = param2;
			param1->parent = param2;

			param1 = param2;
		}	
	}
	
	return param1;
}


static TreeNode_t* GetAssign(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef)
{
	TreeNode_t* node1 = GetE(tokens, curIndex, memdef);
	
	if (STR_EQ("nshanakuma", (*(tokens->firstTok + *curIndex))->name))
	{

		if ((*(tokens->firstTok + *curIndex - 1))->type != Type_VAR)
		{
			assert(0 && "Syntax Error");
		}

		TreeNode_t* oper = NULL;
		if (node1->type == Type_DEF)
		{
			oper = node1;
		}
		else
		{
			oper = *(tokens->firstTok + *curIndex);
			oper->left = node1;
			node1->parent = oper;
		}

		(*curIndex)++;
		
		TreeNode_t* init = GetE(tokens, curIndex, memdef);

		oper->right = init;
		init->parent  = oper;

		node1 = oper;
	}
	
	return node1;
}

static TreeNode_t* GetE(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef)
{
	TreeNode_t* node1 = GetT(tokens, curIndex, memdef);
	TreeNode_t* oper  = NULL;

	while (STR_EQ("gumarats", (*(tokens->firstTok + *curIndex))->name) || STR_EQ("hanats", (*(tokens->firstTok + *curIndex))->name))
	{
		oper = *(tokens->firstTok + *curIndex);
		(*curIndex)++;

		TreeNode_t* node2 = GetT(tokens, curIndex, memdef);

		oper->left    = node1;
		oper->right   = node2;
		node1->parent = oper;
		node2->parent = oper;

		node1 = oper;
	}

	return node1;
}

static TreeNode_t* GetT(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef)
{
	TreeNode_t* node1 = GetPow(tokens, curIndex, memdef);
	TreeNode_t* oper  = NULL;

	while (STR_EQ("basmapatkel", (*(tokens->firstTok + *curIndex))->name) || STR_EQ("bajanel", (*(tokens->firstTok + *curIndex))->name))
	{
		oper = *(tokens->firstTok + *curIndex);
		(*curIndex)++;

		TreeNode_t* node2 = GetPow(tokens, curIndex, memdef);

		oper->left    = node1;
		oper->right   = node2;
		node1->parent = oper;
		node2->parent = oper;

		node1 = oper;
	}

	return node1;
}

static TreeNode_t* GetPow(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef)
{
	TreeNode_t* node1 = GetP(tokens, curIndex, memdef);

	TreeNode_t* oper  = NULL;

	while (STR_EQ("astijan", (*(tokens->firstTok + *curIndex))->name))
	{
		oper = *(tokens->firstTok + *curIndex);
		(*curIndex)++;

		TreeNode_t* node2 = GetP(tokens, curIndex, memdef);

		oper->left    = node1;
		oper->right   = node2;
		node1->parent = oper;
		node2->parent = oper;

		node1 = oper;
	}

	return node1;
}

static TreeNode_t* GetP(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef)
{
	if (STR_EQ("bac", (*(tokens->firstTok + *curIndex))->name))
	{
		(*curIndex)++;
		TreeNode_t* exp = GetE(tokens, curIndex, memdef);
		SYNTAXCTRL("pag");
		return exp;
	}
	else if (STR_EQ("axper", (*(tokens->firstTok + *curIndex))->name))
	{
		return GetDef(tokens, curIndex, memdef);
	}
	else if ((*(tokens->firstTok + *curIndex))->type == Type_VAR)
	{
		return (*curIndex + 1 < tokens->nodeCount) ? *(tokens->firstTok + (*curIndex)++) : *(tokens->firstTok + *curIndex);
	}
	else if ((*(tokens->firstTok + *curIndex))->type == Type_RETURN)
	{
		return GetRet(tokens, curIndex, memdef);;
	}
	else if ((*(tokens->firstTok + *curIndex))->type == Type_FUNC && !STR_EQ("otec", (*(tokens->firstTok + *curIndex))->name))
	{
		return GetCall(tokens, curIndex, memdef);;
	}
	else if (STR_EQ("ete", (*(tokens->firstTok + *curIndex))->name))
	{
		return GetIf(tokens, curIndex, memdef);
	}

	else if (STR_EQ("kani", (*(tokens->firstTok + *curIndex))->name))
	{
		return GetWhile(tokens, curIndex, memdef);
	}
	else 
	{
		return GetN(tokens, curIndex);
	}
}

static TreeNode_t* GetWhile(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef)
{
	TreeNode_t* whileNode = *(tokens->firstTok + *curIndex);
	
	SYNTAXCTRL("bac");
	TreeNode_t* whileCond = GetCond(tokens, curIndex, memdef);
	SYNTAXCTRL("pag");

	SYNTAXCTRL("barev");
	TreeNode_t* whileStat = GetStat(tokens, curIndex, memdef);
	SYNTAXCTRL("ctesutyun");

	whileNode->left   = whileCond;
	whileCond->parent = whileNode;
	whileNode->right  = whileStat;
	whileStat->parent = whileNode;

	return whileNode;
}

static TreeNode_t* GetIf(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef)
{
	TreeNode_t* ifNode = *(tokens->firstTok + *curIndex);
	(*curIndex)++;

	SYNTAXCTRL("bac");
	TreeNode_t* ifCond = GetCond(tokens, curIndex, memdef);
	SYNTAXCTRL("pag");
	SYNTAXCTRL("aysdepkum");

	SYNTAXCTRL("barev");
	TreeNode_t* ifStat = GetStat(tokens, curIndex, memdef);
	SYNTAXCTRL("ctesutyun");
	*curIndex -= 2;
	SYNTAXCTRL("verj");
	*curIndex += 1;

	ifNode->left   = ifCond;
	ifCond->parent = ifNode;
	
	if (STR_EQ("hakarak", (*(tokens->firstTok + *curIndex))->name))
	{
		TreeNode_t* elseNode = *(tokens->firstTok + *curIndex);
		(*curIndex)++;
		SYNTAXCTRL("barev");
		TreeNode_t* elseStat = GetStat(tokens, curIndex, memdef);
		SYNTAXCTRL("ctesutyun");
		*curIndex -= 2;
		SYNTAXCTRL("verj");
		*curIndex += 1;	

		elseNode->left   = ifStat;
		ifStat->parent   = elseNode;
		elseNode->right  = elseStat;
		elseStat->parent = elseNode;
		ifNode->right    = elseNode;
		elseNode->parent = ifNode;
	}
	else
	{
		ifNode->right  = ifStat;
		ifStat->parent = ifNode;
	}

	return ifNode;
}


static TreeNode_t* GetRet(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef)
{
	TreeNode_t* ret = *(tokens->firstTok + *curIndex);

	if (STR_EQ("verj", (*(tokens->firstTok + *curIndex + 1))->name))
	{
		return ret;
	}

	(*curIndex)++;

	TreeNode_t* retArg = GetE(tokens, curIndex, memdef);

	ret->left      = retArg;
	retArg->parent = ret;
	
	return ret;
}


static TreeNode_t* GetDef(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef)
{
		TreeNode_t* defNode = *(tokens->firstTok + *curIndex);

		if ((*(tokens->firstTok + *curIndex + 1))->type != Type_VAR && (*(tokens->firstTok + *curIndex + 2))->type != Type_ASSIGN)
		{
			fprintf(stderr, "_VAR_ERR_: Wrong variable declaration in %lu\n", (*(tokens->firstTok + *curIndex))->lineIndex);
			abort();
		}

		(*curIndex)++;

		TreeNode_t* varName = *(tokens->firstTok + *curIndex);
		defNode->left   = varName;
		varName->parent = defNode;

		(*curIndex)++;

		return defNode;
}

static TreeNode_t* GetCall(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef)
{
	TreeNode_t* call = MakeNode(Type_CALL, Op_Null, 0, "CALL", NULL, NULL);
	memdef->adr[(memdef->size)++] = call;

	TreeNode_t* func = *(tokens->firstTok + *curIndex);
	
	(*curIndex)++;

	SYNTAXCTRL("bac");
	TreeNode_t* arg = GetArg(tokens, curIndex, memdef);
	SYNTAXCTRL("pag");
		
	call->left   = func;
	func->parent = call;
	func->left   = arg;
	arg->parent  = func;

	return call;
}

static TreeNode_t* GetCond(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef)
{
	return GetE(tokens, curIndex, memdef);
}

static TreeNode_t* GetStat(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef)
{
	TreeNode_t* node1 = GetAssign(tokens, curIndex, memdef);
	TreeNode_t* stat1 = MakeNode(Type_STAT, Op_Null, 0, "ST", NULL, NULL);
	memdef->adr[(memdef->size)++] = stat1;

	stat1->left   = node1;
	node1->parent = stat1;
	
	node1 = stat1;
	
	while (STR_EQ("verj", (*(tokens->firstTok + *curIndex))->name))
	{
		(*curIndex)++;
		
		if (STR_EQ("ctesutyun", (*(tokens->firstTok + *curIndex))->name))
		{
			break;
		}
		TreeNode_t* node2 = GetAssign(tokens, curIndex, memdef);
		TreeNode_t* stat2 = MakeNode(Type_STAT, Op_Null, 0, "ST", NULL, NULL);
		memdef->adr[(memdef->size)++] = stat2;

		
		stat2->left   = node2;
		node2->parent = stat2;
		stat2->parent = node1;
		node1->right  = stat2;

		node1 = stat2;
	}
	
	return stat1;
}

static TreeNode_t* GetN(Tokens_t* tokens, size_t* curIndex)
{
	if ((*(tokens->firstTok + *curIndex))->type != Type_NUM)
	{
		return NULL;
	}

	(*curIndex)++;

	return *(tokens->firstTok + *curIndex - 1); 
}

static void TokensDtor(Tokens_t* tokens, MemDefender_t* memdef)
{
    for (size_t index = 0; index < tokens->nodeCount; index++)
	{
        free(*(tokens->firstTok + index));
    }

    free(tokens->firstTok);

    for (size_t index = 0; index < memdef->size; index++)
	{
        free(memdef->adr[index]);
    }

    free(memdef->adr);
}


