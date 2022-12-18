#pragma once

#include "../standart/standart.h"

void ArmStat(TreeNode_t* node, FILE* trans);

void ArmDef(TreeNode_t* node, FILE* trans);
void ArmAssign(TreeNode_t* node, FILE* trans);
void ArmRet(TreeNode_t* node, FILE* trans);
void ArmPrintf(TreeNode_t* node, FILE* trans);
void ArmParam(TreeNode_t* node, FILE* trans);
void ArmExp(TreeNode_t* node, FILE* trans);
void ArmOp(TreeNode_t* node, FILE* trans);
void ArmScanf(TreeNode_t* node, FILE* trans);
void ArmIf(TreeNode_t* node, FILE* trans);
void ArmWhile(TreeNode_t* node, FILE* trans);
void ArmFunc(TreeNode_t* node, FILE* trans);
void ArmCall(TreeNode_t* node, FILE* trans);

