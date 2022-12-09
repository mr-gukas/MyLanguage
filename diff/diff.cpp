#include "diff.h"

#ifdef LOG_MODE
    FILE* LogFile = startLog(LogFile);
#endif

int main(void)
{
	Expression_t exp = {};
	DataDownload(&exp);
	
	FILE* texFile = fopen("obj/texfile.tex", "w+");
	MakeBook(&exp, texFile);
	fclose(texFile);

	system("pdflatex obj/texfile.tex");
	system("xdg-open texfile.pdf");
	system("cvlc obj/muz.mp3");
	
	TreeDtor(exp.tree);
	free(exp.tree);
	free(exp.varArr);
	
#ifdef LOG_MODE
    endLog(LogFile);
#endif

	return 0;
}

int DataDownload(Expression_t* exp)
{
	if (exp == NULL) return WRONG_DATA;

	FILE* loadData = fopen("obj/expression", "r");
	if (loadData == NULL)
	{
		fprintf(stderr, "..can not connect to the data\n");
		return NULL;
	}
 
	exp->tree = (Tree_t*) calloc(1, sizeof(Tree_t));
	TreeCtor(exp->tree);
	free(exp->tree->root);

	TEXT data = {};
	textCtor(&data, loadData);
	fclose(loadData);
		
	exp->tree->root = GetG(&(data.lines[1].lineStart));
	TreeUpdate(exp->tree, exp->tree->root);

	if ((sscanf(data.lines[3].lineStart, "%lu", &exp->derOrd) != 1) ||
		(sscanf(data.lines[7].lineStart, "%lf", &exp->point)  != 1) ||
		(sscanf(data.lines[5].lineStart, "%lu", &exp->macOrd) != 1))
	{
		return WRONG_DATA;
	}
	
	ReadVarValue(exp, &data);
	textDtor(&data);

	return STATUS_OK;
}

int ReadVarValue(Expression_t* exp, TEXT* data)
{
	if (exp == NULL || data == NULL) return WRONG_DATA;
	
	size_t varCount = 0;
	if (sscanf(data->lines[9].lineStart, "%lu", &varCount) != 1)
	{
		return WRONG_DATA;
	}
	exp->varCount = varCount;

	exp->varArr = (VarValue_t*) calloc(varCount, sizeof(VarValue_t));
		
	for (size_t index = 11, varInd = 0; index < 11 + varCount; index++)
	{
		char*  str   = data->lines[index].lineStart;
		size_t count = 0;
		while (isalpha(*str) && count < VAR_SIZE)
		{
			exp->varArr[varInd].name[count] = *str;
			str++;
			count++;	
		}
		sscanf(data->lines[index].lineStart + count + 3, "%lf%lf", &(exp->varArr[varInd].value), &(exp->varArr[varInd].varErr));
		varInd++;
	}

	return STATUS_OK; 
}

TreeNode_t* GetG(char** str)
{
	TreeNode_t* node = GetE(str);

	assert(**str == '\0');

	return node;
}

TreeNode_t* GetE(char** str)
{
	TreeNode_t* node = GetT(str);

	while (**str == '+' || **str == '-')
	{
		char op = **str;
		(*str)++;

		TreeNode_t* nodeR = GetT(str);
		TreeNode_t* nodeL = TrNodeCopy(node);	
		DeleteNode(node);

		if (op == '+')
			node = ADD(nodeL, nodeR);
		else
			node = SUB(nodeL, nodeR);	
	}

	return node;
}

TreeNode_t* GetT(char** str)
{
	TreeNode_t* node = GetPower(str);

	while (**str == '*' || **str == '/')
	{
		char op = **str;
		(*str)++;

		TreeNode_t* nodeR = GetPower(str);
		TreeNode_t* nodeL = TrNodeCopy(node);	
		DeleteNode(node);

		if (op == '*')
			node = MUL(nodeL, nodeR);
		else
			node = DIV(nodeL, nodeR);
	}

	return node;
}

TreeNode_t* GetPower(char** str)
{
	TreeNode_t* node = GetBracket(str);

	if (**str == '^') 
	{
		(*str)++;

		TreeNode_t* nodeR = GetPower(str);
		TreeNode_t* nodeL = TrNodeCopy(node);	
		DeleteNode(node);

		node = POW(nodeL, nodeR);
	}

	return node;
}

TreeNode_t* GetBracket(char** str)
{
	int sign = 1;

	while (**str == '+' || **str == '-')
	{
		if (**str == '-')
			sign *= -1;

		(*str)++;
	}
	
	if (sign == -1)
	{
		TreeNode_t* nodeL = MAKE_NUM(-1);
		TreeNode_t* nodeR = GetBracket(str);	

		return MUL(nodeL, nodeR);	
	}
	
	if (**str == '(')
	{
		(*str)++;
		
		TreeNode_t* node = GetE(str);
		
		assert(**str == ')');
		(*str)++;

		return node;
	}
	else
		return GetTextFunc(str);
}

TreeNode_t* GetTextFunc(char** str)
{
	OperationType op = isTextFunc(str);
	
	if (op != Op_Null)
	{
		TreeNode_t* nodeR = GetBracket(str); 
		
		switch (op)
		{
			case Op_Sin: return SIN(nodeR);
			case Op_Cos: return COS(nodeR);
			case Op_Ln:  return LN(nodeR);
			default: break;
		}
	}
	else
	{
		return GetVar(str);
	}
}

OperationType isTextFunc(char** str)
{

	if (**str == 's' && *(*str + 1) == 'i' && *(*str + 2) == 'n')
	{
		*str += 3;
		return Op_Sin;
	}
	else if (**str == 'c' && *(*str + 1) == 'o' && *(*str + 2) == 's')
	{
		*str += 3;
		return Op_Cos;
	}
	else if (**str == 'l' && *(*str + 1) == 'n')
	{
		*str += 2;
		return Op_Ln;
	}

	return Op_Null;
}

TreeNode_t* GetVar(char** str)
{
	TreeNode_t* node = NULL;
	
	if (isalpha(**str))
	{
		node = MakeNode(Type_VAR, Op_Null, 0, "", NULL, NULL);

		char var[VAR_SIZE] = "";
		char* oldStr = *str;
				
		size_t index = 0;	
		while (isalpha(**str) && index < VAR_SIZE)
		{
			var[index] = **str;
			(*str)++;
			index++;	
		}
		
		assert(oldStr != *str);
		
		strcpy(node->varVal, var);

		return node;
	}
	else
		return GetNumber(str);
}

TreeNode_t* GetNumber(char** str)
{
	double val   = 0;
	int    count = 0; 
	
	sscanf(*str, "%lf%n", &val, &count);
	*str += count;

	assert(count);
	
	return MAKE_NUM(val);
}

void MakeBook(Expression_t* exp, FILE* texFile)
{
	assert(exp && texFile);

	StartTexPrint(texFile);
	fprintf(texFile, "Итак, братик, пока мясо жарится, давай погляжу на твой задача:\\\\ f(x)=");

	TreeTexPrint(exp->tree, texFile);
	fprintf(texFile, "\\section{Производная (ара, зачем тебе это? пойдем лучше в нарды сыграем партию)}");

	fprintf(texFile, "Сначала тост! %s\\\\", armenian[rand() % DRINK]);
	fprintf(texFile, "\\includegraphics[scale=0.5]{obj/man.jpg} \n\n");
	Tree_t* diffTree = DiffExpression(exp->tree, "x", exp->derOrd, texFile);
	
	double val = CalcValue(diffTree, exp->point, "x", texFile);

	fprintf(texFile, "\n\nКлянусь, что значение %lu-ой производной в точке %lg: ", exp->derOrd, exp->point);
	fprintf(texFile, "$%lg$\n", val);
	fprintf(texFile, "\n\nМое семейное дерево конечно же больше этого, но всё же:");
	fprintf(texFile, "\n\n\\includegraphics[width=20cm, height=10cm]{obj/dump001.png}\n\n");
	fprintf(texFile, "\\section{Макларен (на самом деле Карен Макларян...)}");
	fprintf(texFile, "Апер, ради Каренчика можно и тост произнести: %s\\\\"
					 "Ладно-ладно, поиграй за меня, а я разложу пока\\\\" 
					 "\\\\%s\n\n", armenian[7], difficult[rand() % DIFFICULT]);

	Maclaurin(exp, texFile);
	fprintf(texFile, "\n\n\\includegraphics[scale=0.5]{obj/mac.jpg}\n");

	fprintf(texFile, "\\section{Касательная (девушка, можно стать вашей касательной...)}");
	fprintf(texFile, "Дорогие гости, как же я рад, что вы все пришли! Поднимем бокалы: %s\\\\"
					 "Друг, я тебе помогу с твоей касательной, но ты узнай - вдруг у нее подруга есть свободная...\\\\" 
					 "\\\\%s\\\\", armenian[rand() % DRINK], difficult[rand() % DIFFICULT]);


	TangentEquation(exp, texFile);
	
	fprintf(texFile, "\\section{График (вайййййя, а как тут Арарат нарисовать...)}");
	fprintf(texFile, "\\includegraphics[scale=0.6]{obj/ara.jpg}\n\n");
	fprintf(texFile, "Вспомнилась история: %s\\\\"
					 "Ара, у меня ребенок с закрытыми глазами лучше рисует. Учись, студент...\\\\" 
					 "\\\\%s\\\\", armenian[rand() * 0 + rand() % DRINK], difficult[rand() * 0 + rand() % DIFFICULT + 1]);

	BuildGraph(exp, texFile);
	
	CalculateError(exp, texFile);

	EndTexPrint(texFile);
	TreeDtor(diffTree);
	free(diffTree);
}

int StartTexPrint(FILE* texFile)
{
	if (texFile == NULL) return WRONG_DATA;
	
	fprintf(texFile, "\\documentclass{article}\n"
				     "\\usepackage[utf8]{inputenc}\n"
					 "\\title{Моя жизнь на кафедре вышмата, или как армяне в нарды играли на физтехе}\n"
					 "\\author{Владимир Гукасян aka mr.gukas}\n"
					 "\\date{Очереднойбрь 1984}\n"
					 "\\usepackage[T2A]{fontenc}\n"
					 "\\usepackage[utf8]{inputenc}\n"
					 "\\usepackage[english,russian]{babel}\n"
					 "\\usepackage{amsthm}\n"
					 "\\usepackage{amsmath}\n"
					 "\\usepackage{amssymb}\n"
					 "\\usepackage{tikz}\n"
					 "\\usepackage{url}\n"
					 "\\usepackage{graphicx}\n"
					 "\\graphicspath{ {./images/} }\n"
					 "\\usepackage{textcomp}\n"
				 	 "\\setlength{\\oddsidemargin}{-0.4in}\n"
					 "\\setlength{\\evensidemargin}{-0.4in}\n"
					 "\\setlength{\\emergencystretch}{2pt}"
					 "\\setlength{\\textwidth}{7in}\n"
					 "\\setlength{\\parindent}{0ex}\n"
					 "\\setlength{\\parskip}{1ex}\n"
					 "\\newcommand\\round[1]{\left[#1\right]}\n"
					 "\\begin{document}\n"
					 "\\maketitle\n"
					 "\\newpage\n"
					 "\\section{Введение}");

	fprintf(texFile, "Хочу начать данную книгу со слов римского полководца Юлия Цезаря:\\\\"
					 "\"Когда армяне, хватают друг друга за руки и плечом к плечу топчут землю под звук своих барабанов и абрикосовых инструментов,"
					 " скорее колонны моего дворца превратятся в пылинки, чем их будет возможно остановить\"\\\\");
	
	fprintf(texFile, "Как это связано с математикой? Честно - не знаю, но моя армянская душа попросила так сделать..."
					 " Тем более я уверен, что именно армяне придумали математику (как и вообще всё в этом мире)"
					 " И конечно, все наслышаны об армянском гостеприимстве, так что садись рядом за стол, мой дорогой читатель, угощайся, ешь, пей, танцуй!"
					 "\\\\ Что? Да-да, решу я тебе твою математику, но для начала выпьем за встречу, читатель джан!\\\\"
					 "\\includegraphics[scale=0.4]{obj/food.jpg}\n\\newpage");


					
	return STATUS_OK;
}

int EndTexPrint(FILE* texFile)
{
	if (texFile == NULL) return WRONG_DATA;

	fprintf(texFile, "\\end{document}\n");

	return STATUS_OK;
}

int TreeTexPrint(const Tree_t* tree, FILE* texFile)
{
	if (tree    == NULL) return NULL_TREE;
	if (texFile == NULL) return WRONG_DATA;
	
	fprintf(texFile, "\\indent \\sloppy $");

	TrNodesPrint(tree->root, texFile);

	fprintf(texFile, "$");

	return STATUS_OK;
}

int TrNodesPrint(const TreeNode_t* node, FILE* texFile)
{
	if (node == NULL || texFile == NULL) return WRONG_DATA;
	
	if ((node->left  != NULL || node->right != NULL) &&
	   !(node->opVal >= 4    && node->opVal <= 10)   &&
		 node->parent != NULL                        &&
	   !(node->opVal - node->parent->opVal > 1))

		fprintf(texFile, "(");
		
#define OperPrint_(str)              \
	fprintf(texFile, "{");			  \
	TrNodesPrint(node->left, texFile); \
	fprintf(texFile, "}" #str "{");     \
	TrNodesPrint(node->right, texFile);  \
	fprintf(texFile, "}");

	switch (node->type)
	{
		case Type_VAR: fprintf(texFile, "%s", node->varVal);  break; 
		case Type_NUM: 
		{
			if (node->numVal < 0)
				fprintf(texFile, "(");
			fprintf(texFile, "%lg", node->numVal); 
			if (node->numVal < 0)
				fprintf(texFile, ")");

			break;
		}
		case Type_OP:  
		{
			switch (node->opVal)
			{
				case Op_Add:
					OperPrint_(+);
					break;
				case Op_Sub: 
					OperPrint_(-);
					break;
			
				case Op_Mul:
					OperPrint_(\\cdot);
					break;
				case Op_Div:
					fprintf(texFile, "\\frac{");
					TrNodesPrint(node->left, texFile);
					fprintf(texFile, "}{");
					TrNodesPrint(node->right, texFile);
					fprintf(texFile, "}");
					break;

				case Op_Log:
					fprintf(texFile, "\\log_{");
					TrNodesPrint(node->left, texFile);
					fprintf(texFile, "}{");
					TrNodesPrint(node->right, texFile);
					fprintf(texFile, "}");
					break;

				case Op_Ln: 
					fprintf(texFile, "\\ln{");
					TrNodesPrint(node->right, texFile);
					fprintf(texFile, "}");
					break;

				case Op_Sin:
					fprintf(texFile, "\\sin{");
					TrNodesPrint(node->right, texFile);
					fprintf(texFile, "}");
					break;

				case Op_Cos: 
					fprintf(texFile, "\\cos{");
					TrNodesPrint(node->right, texFile);
					fprintf(texFile, "}");
					break;

				case Op_Pow:
					OperPrint_(^);
					break;

				default: fprintf(texFile, "???");
			}

			break;
		}

		default: fprintf(texFile, "???");
	}

	if ((node->left != NULL || node->right != NULL) &&
		!(node->opVal >= 4 && node->opVal <= 10)    &&
		node->parent != NULL                        &&
		!(node->opVal - node->parent->opVal > 1))

		fprintf(texFile, ")");
	
#undef OperPrint_

	return STATUS_OK;
}

Tree_t* DiffExpression(Tree_t* tree, const char* var, size_t derOrd, FILE* texFile)
{
	if (tree == NULL || tree->root == NULL) return NULL;
	
	Tree_t*	diffTree = (Tree_t*) calloc(1, sizeof(Tree_t));
	Tree_t* curTree = (Tree_t*) calloc(1, sizeof(Tree_t));

	curTree->root = TrNodeCopy(tree->root);
	TreeUpdate(curTree, curTree->root);
	
	for (size_t index = 0; index < derOrd; index++)
	{
		
		diffTree->root = DiffTree(curTree->root, var);
		TreeUpdate(diffTree, diffTree->root);
		
		if (texFile)
		{
			fprintf(texFile, "\n\n  $f^{(%lu)}(x)=$", index+1);
			TreeTexPrint(diffTree, texFile);
			fprintf(texFile, "\n\n\n%s\n\n\n", difficult[rand() % DIFFICULT]);
		}

		TreeSimplify(diffTree);
		
		if (texFile)
		{
			fprintf(texFile, "\n\n%s\n\n", filler[(rand() % FILLER + rand() % FILLER)/2]);
			fprintf(texFile, "\n\n $f^{(%lu)}(x)=$ ", index+1);
			TreeTexPrint(diffTree, texFile);

			if (index != derOrd - 1)
				fprintf(texFile, "\n\n Твой ход, брат\n\n");
		}

		TrNodeRemove(curTree, curTree->root);
		curTree->root = TrNodeCopy(diffTree->root);
		TreeUpdate(curTree, curTree->root);

		TrNodeRemove(diffTree, diffTree->root);
	}

	free(diffTree);
	TreeDump(curTree);

	return curTree;
}

TreeNode_t* DiffTree(TreeNode_t* node, const char* var)
{
	if (node == NULL) return NULL;
	
	switch (node->type)
	{
		case Type_NUM:
		{
			return MAKE_NUM(0); 
		}
		case Type_VAR:
		{
			if (strcmp(node->varVal, var) == 0)
				return MAKE_NUM(1);
			else
				return MAKE_NUM(0);
		}
		case Type_OP:
		{
			switch(node->opVal)
			{
				case Op_Add: return ADD(DL, DR); 
				case Op_Sub: return SUB(DL, DR);
				case Op_Mul: return ADD(MUL(DL, CR), MUL(CL, DR));				
				case Op_Div: return DIV(SUB(MUL(DL,CR), MUL(CL, DR)), MUL(CR, CR));
				case Op_Sin: return MUL(DR, COS(CR));
				case Op_Cos: return MUL(DR, MUL(MAKE_NUM(-1), SIN(CR)));
				case Op_Ln : return MUL(DR, DIV(MAKE_NUM(1), CR));
				case Op_Log: return	DIV(MUL(DR, DIV(MAKE_NUM(1), CR)), MUL(DL, DIV(MAKE_NUM(1), CL)));
				case Op_Pow:
				{
					int inLeft  = IsInTree(node->left, var);
					int inRight = IsInTree(node->right, var);
					
					if (inLeft && inRight)
					{
						return MUL(POW(CL, CR), ADD(MUL(DR, LN(CR)), MUL(CR, MUL(DL, DIV(MAKE_NUM(1), CL))))); 
					}
					else if (inLeft)
					{
						return MUL(MUL(CR, POW(CL, SUB(CR, MAKE_NUM(1)))), DL);
					}
					else if (inRight)
					{
						return MUL(MUL(LN(CL), POW(CL, CR)), DR);
					}
					else
						return MAKE_NUM(1); 

					break;
				
				}
				
				default: break;
			}
		}
	}
}


int IsInTree(TreeNode_t* node, const char* value)
{
	if (node == NULL || value == NULL) return WRONG_DATA;
	
	if (node->type == Type_VAR && strcmp(node->varVal, value) == 0)
		return 1;
	
	else if (node->left == NULL && node->right == NULL)
		return 0;

	if (IsInTree(node->left, value))
		return 1;
	if (IsInTree(node->right, value))
		return 1;

	return 0;
}

TreeNode_t* TrNodeCopy(TreeNode_t* node)
{
	if (node == NULL) return NULL;

	TreeNode_t* newNode = (TreeNode_t*) calloc(1, sizeof(TreeNode_t));
	if (newNode == NULL)
		return NULL;

	newNode->type   = node->type;
	newNode->opVal  = node->opVal;
	newNode->numVal = node->numVal;
	strcpy(newNode->varVal, node->varVal);
	
	if (node->left != nullptr)
		newNode->left = TrNodeCopy(node->left);
	
	if (node->right != nullptr)
		newNode->right = TrNodeCopy(node->right);

	return newNode;
}

int TreeSimplify(Tree_t* tree)
{
	if (tree == NULL) return NULL_TREE;
		
	int     isSimpled = 0;
	size_t	oldSize   = tree->size;
	do 
	{
		isSimpled = 0;
		
		TreeSimplifyConst(tree, tree->root);
		if (tree->size < oldSize)
		{
			isSimpled += 1;
			oldSize   = tree->size;
		}

		TreeSimplifyNeutral(tree, tree->root);
		if (tree->size < oldSize)
		{
			isSimpled += 1;
			oldSize    = tree->size;
		}
	} while (isSimpled);
	
	return STATUS_OK;
}

int TreeSimplifyConst(Tree_t* tree, TreeNode_t* node)
{
	if (node == NULL || tree == NULL) return WRONG_DATA;
		
	int oldSize = 0;
	
	if (node->left == NULL && node->right == NULL) return 0;
	
	int isSimpled = SimplifyConst(node, tree);
	TreeUpdate(tree, tree->root);

	if (!isSimpled && node && node->left)
		TreeSimplifyConst(tree, node->left);

	if (!isSimpled && node && node->right)
		TreeSimplifyConst(tree, node->right);

	TreeUpdate(tree, tree->root);
	return 0;
}

int SimplifyConst(TreeNode_t* node, Tree_t* tree)
{
	if (node == NULL) return WRONG_DATA;
	
	int isSimpled = 0;

	if (node->right->type == Type_NUM) 
	{                                              
		double newNum = 0;

		if (node->left != NULL && node->left->type == Type_NUM)				
		{
			switch (node->opVal)
			{
				case Op_Add: newNum = node->left->numVal + node->right->numVal;		break;
				case Op_Sub: newNum = node->left->numVal - node->right->numVal;		break;
				case Op_Mul: newNum = node->left->numVal * node->right->numVal;		break;
				case Op_Div: newNum = node->left->numVal / node->right->numVal;		break;
				case Op_Pow: newNum = pow(node->left->numVal, node->right->numVal); break;

				default: break;
			}
		}
		else if (node->left == NULL)	
		{
			switch (node->opVal)
			{
				case Op_Sin: newNum = sin(node->right->numVal);	break;
				case Op_Cos: newNum = cos(node->right->numVal);	break;
				case Op_Ln:	 newNum = log(node->right->numVal);	break;

				default: break;
			}

		}
		
		else 
			return isSimpled;

		TreeNode_t* newNode = MakeNode(Type_NUM, Op_Null, newNum, NULL, NULL, NULL);  

		if (node == tree->root)
			tree->root = newNode;
		if (node->parent && node->parent->left && node->parent->left == node)                                               
				node->parent->left = newNode;                                            
		else if (node->parent && node->parent->right && node->parent->right == node)       
				node->parent->right = newNode;                                        
		
		TrNodeRemove(tree, node);
		isSimpled = 1;
	}
	
	return isSimpled;
}

int TreeSimplifyNeutral(Tree_t* tree, TreeNode_t* node)
{
	if (node == NULL || tree == NULL) return WRONG_DATA;
	
	if (node->left == NULL && node->right == NULL) return 0;
	
	int isSimpled = SimplifyNeutral(node, tree);
	TreeUpdate(tree, tree->root);

	if (!isSimpled && node && node->left)
		TreeSimplifyNeutral(tree, node->left);

	if (!isSimpled && node && node->right)
		TreeSimplifyNeutral(tree, node->right);

	TreeUpdate(tree, tree->root);

	return 0;
}

int SimplifyNeutral(TreeNode_t* node, Tree_t* tree)
{
	if (node == NULL) return WRONG_DATA;
	
	int isSimpled = 0;
	
	int isRight = -1;
	int isLeft  = -1;

	if (node->right && node->right->type == Type_NUM)
	{
		if (node->right->numVal == 0)
			isRight = 0;
		else if (node->right ->numVal == 1)
			isRight = 1;
	}
	
	if (node->left && node->left->type == Type_NUM)
	{
		if (node->left->numVal == 0)
			isLeft = 0;
		else if (node->left->numVal == 1)
			isLeft = 1;
	}

	if (isRight == -1 && isLeft == -1)
		return isSimpled;
	
	TreeNode_t* newNode = NULL;
	
	switch (node->opVal)
	{
		case Op_Add:
		{
			if (isLeft == 0)
				newNode = TrNodeCopy(node->right);
			break;
		}
		case Op_Sub:
		{
			if (isRight == 0)
				newNode = TrNodeCopy(node->left);
			break;
		}
		case Op_Mul:
		{
			if (isLeft == 0)
				newNode = TrNodeCopy(node->left);
			else if (isLeft == 1)
				newNode = TrNodeCopy(node->right);
			break;
		}
		case Op_Div:
		{
			if (isLeft == 0 || isRight == 1)
				newNode = TrNodeCopy(node->left);
			break;
		}
		case Op_Pow:
		{
			if (isLeft == 1 || isRight == 1)
				newNode = TrNodeCopy(node->left);
			else if (isRight == 0)
				newNode = MAKE_NUM(1);

			break;
		}
		case Op_Ln:
		{
			if (isRight == 1)
				newNode = MAKE_NUM(0);
			break;
		}

		default: break;
	}
	
	if (newNode == NULL)
		return isSimpled;
	
	if (node == tree->root)
		tree->root = newNode;
	if (node->parent && node->parent->left && node->parent->left == node)                                               
			node->parent->left = newNode;                                            
	else if (node->parent && node->parent->right && node->parent->right == node)       
			node->parent->right = newNode;                                        
					
	TrNodeRemove(tree, node);
		
	isSimpled = 1;
	
	return isSimpled;
}

double CalcValue(Tree_t* tree, double point, const char* var, FILE* texFile)
{
	if (tree == NULL || tree->root == NULL) return WRONG_DATA;

	Tree_t* cpTree = (Tree_t*) calloc(1, sizeof(Tree_t));
	cpTree->root = TrNodeCopy(tree->root);
	TreeUpdate(cpTree, cpTree->root);

	PutValueInPoint(cpTree->root, var, point);
	TreeSimplify(cpTree);
	
	double val = cpTree->root->numVal;

	if (cpTree->root->type != Type_NUM)
		fprintf(texFile, "\n\nОбнаружены прочие переменные, поэтому значение функции от аргумента принято за 0\n\n"); 

	TreeDtor(cpTree);
	free(cpTree);

	return val;
}

int PutValueInPoint(TreeNode_t* node, const char* var, double point)
{
	if (node == NULL || var == NULL) return WRONG_DATA;
	
	if (node->type == Type_VAR && strcmp(node->varVal, var) == 0)
	{
		node->type   = Type_NUM;
		node->numVal = point;
	}
	
	if (node->left)
		PutValueInPoint(node->left, var, point);

	if(node->right)
		PutValueInPoint(node->right, var, point);

	return STATUS_OK;
}

int Maclaurin(Expression_t* exp, FILE* texFile)
{
	if (exp == NULL || exp->tree == NULL || exp->tree->root == NULL) return WRONG_DATA;
	
	TreeTexPrint(exp->tree, texFile);

	fprintf(texFile, "$=");
	
	Tree_t* curTree = NULL;

	double funcVal = CalcValue(exp->tree, 0, "x", texFile);
	fprintf(texFile, "%lg +", funcVal);
	
	for (size_t index = 1; index <= exp->macOrd; ++index)
	{
		curTree = DiffExpression(exp->tree, "x", index, NULL);
			
		double derivVal = CalcValue(curTree, 0, "x", texFile);
		fprintf(texFile, "\\frac{%lg\\cdot x^{%lu}}{%lu}", derivVal, index, factorial(index)); 
		
		if (index != exp->macOrd)
			fprintf(texFile, "+");
		
		TreeDtor(curTree);
		free(curTree);
	}
	
	fprintf(texFile, "+ o(x^{%lu})$\n\n", exp->macOrd);

	return STATUS_OK;
}

size_t factorial(size_t num)
{
	if (num <= 1) return 1;

	return num * factorial(num - 1);
}

int TangentEquation(Expression_t* exp, FILE* texFile)
{
	if (exp == NULL || exp->tree == NULL || exp->tree->root == NULL) return WRONG_DATA;
	
	fprintf(texFile, "\n\nУравнение касательной к графику в точке %lg примет следующий вид: $y = ", exp->point);
	
	Tree_t* curTree = NULL;

	double funcVal = CalcValue(exp->tree, exp->point, "x", texFile);
	
	curTree = DiffExpression(exp->tree, "x", 1, NULL);
	double derivVal = CalcValue(curTree, exp->point, "x", texFile);
	
	double freePart = -derivVal * exp->point + funcVal;

	fprintf(texFile, "%lgx", derivVal);
	if (freePart > 0)	fprintf(texFile, "+");
	fprintf(texFile, "%lg$\n\n", freePart);
		
	TreeDtor(curTree);
	free(curTree);

	return STATUS_OK;
}

int BuildGraph(Expression_t* exp, FILE* texFile)
{
	if (exp == NULL || exp->tree == NULL || exp->tree->root == NULL) return WRONG_DATA;
	
	FILE * gnuplotPipe = popen("gnuplot -persistent", "w");

	fprintf(gnuplotPipe, "set terminal png size 800, 600\n"
						 "set output \"graph.png\"\n"
						 "set xlabel \"X\"\n" 
						 "set ylabel \"Y\"\n"
						 "set grid\n"
						 "set title \"А вы знали, что первыми художниками были армяне?\" font \"Helvetica Bold, 20\"\n"
						 "plot '-' lt 3 linecolor 1 notitle\n");

	for (double x = -20.0; x <= 20.1; x = x + 0.001)
	{
		fprintf(gnuplotPipe, "%lf\t %lf\n", x, CalcValue(exp->tree, x, "x", texFile));
	}
	
	fprintf(gnuplotPipe, "e");
	fclose(gnuplotPipe);
	fprintf(texFile,"\n\n\\includegraphics[scale=0.5]{graph.png}\n\n");

	return STATUS_OK;
}


int CalculateError(Expression_t* exp, FILE* texFile)
{
	if (exp == NULL || texFile == NULL) return WRONG_DATA;

	double sigma = 0;
	for (size_t index = 0; index < exp->varCount; index++)
	{
		Tree_t* diffTree = DiffExpression(exp->tree, exp->varArr[index].name, 1, NULL);		
		double  val      = CalcValue(diffTree, exp->varArr[index].value, exp->varArr[index].name, texFile);

		sigma += pow(val * exp->varArr[index].varErr, 2);

		TreeDtor(diffTree);
		free(diffTree);
	}
	
	fprintf(texFile, "\\section{Пагрешност (это туда сюда двигать...)}");
	fprintf(texFile, "\\includegraphics[scale=0.5]{obj/tuda.jpg}\n\n");
	fprintf(texFile, "Вспомнилась история: %s\\\\"
					 "\\\\%s\\\\", armenian[rand() % DRINK], difficult[rand() * 0 + rand() % DIFFICULT + 1]);


	fprintf(texFile, "\n\nБери свои погрешности и иди танцуй уже: %lg\n\n", sqrt(sigma));	
	
	return STATUS_OK;
}

/*
int ReadTree(Tree_t* tree, const char* expression, size_t size)
{
	if (expression == NULL) return WRONG_DATA;

	TreeNode_t* curNode = tree->root;
	

	for (size_t index = 0; index < size; ++index)
	{
		if (*(expression + index) == ')')
		{
			curNode = curNode->parent;
			continue;
		}
		else if (*(expression + index) == '(')
		{
			if (curNode->left != NULL)
			{
				TrNodeInsert(tree, curNode, RIGHT);
				curNode = curNode->right;
				continue;
			}
			else
			{
				TrNodeInsert(tree, curNode, LEFT);
				curNode = curNode->left;
				continue;
			}
		}
		else 
		{
			index += ReadNodeValue(curNode, expression + index);	

		}	
	
		if (curNode->parent->opVal == Op_Sin ||
			curNode->parent->opVal == Op_Cos ||
			curNode->parent->opVal == Op_Ln)
		{
			curNode->parent->right = curNode;
			curNode->parent->left  = NULL;
		}
	}

	tree->root = tree->root->left;
	free(tree->root->parent);

	tree->root->parent = NULL;
	tree->size--;
	
	return STATUS_OK; 
}

static int ReadNodeValue(TreeNode_t* curNode, const char* expression)
{
	int    charCount = 0;
	double dblVal    = 0;

	if (sscanf(expression, "%lf%n", &dblVal, &charCount) == 1)
	{
		curNode->type   = Type_NUM;
		curNode->numVal = dblVal;
	
		return charCount - 1;
	}

	char*  value     = (char*) calloc(STR_MAX_SIZE, sizeof(char)); 

	if (sscanf(expression, "%[^()]%n", value, &charCount))
	{
		if (strchr(value, '+'))
		{
			curNode->type  = Type_OP;
			curNode->opVal = Op_Add;
		}
		else if (strchr(value, '-'))
		{
			curNode->type  = Type_OP;
			curNode->opVal = Op_Sub;
		}
		else if (strchr(value, '*'))
		{
			curNode->type  = Type_OP;
			curNode->opVal = Op_Mul;
		}
		else if (strchr(value, '/'))
		{
			curNode->type  = Type_OP;
			curNode->opVal = Op_Div;
		}
		else if (strchr(value, '^'))
		{
			curNode->type  = Type_OP;
			curNode->opVal = Op_Pow;
		}
		else if (strcmp(value, "sin") == 0)
		{
			curNode->type  = Type_OP;
			curNode->opVal = Op_Sin;
		}
		else if (strcmp(value, "cos") == 0)
		{
			curNode->type  = Type_OP;
			curNode->opVal = Op_Cos;
		}
		else if (strcmp(value, "ln") == 0)
		{
			curNode->type  = Type_OP;
			curNode->opVal = Op_Ln;
		}
		else if (strcmp(value, "log") == 0)
		{
			curNode->type  = Type_OP;
			curNode->opVal = Op_Log;
		}
		else
		{
			curNode->type     = Type_VAR;
			strcpy(curNode->varVal, value);
		}
		
		free(value);
		return charCount - 1;
	}
	
	return charCount;
}

*/


