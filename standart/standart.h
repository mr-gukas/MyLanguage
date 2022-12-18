#pragma once

#include "../frontend/frontend.h"

TreeNode_t* ReadStandart(FILE* standart);
void PrintTreeToFile(TreeNode_t* node, FILE* standartFile);
void PrintNodeDataToFile(TreeNode_t* node, FILE* standartFile);
void PrintOpToFile(TreeNode_t* node, FILE* standartFile);
TreeNode_t* ReadTree(TreeNode_t* node, char** buf);
NodeType GetStandartType(const char* word);

OperationType GetStandartOpType(const char* name);
void SkipFileSpaces(FILE* file);

