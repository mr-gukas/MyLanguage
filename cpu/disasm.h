#include "cpu.h"

unsigned char* ReadBin(unsigned char* cmdArr, size_t* size, FILE* binary);
void RunDisasm(unsigned char* cmdArr, size_t* size, FILE* rezult);
void DisasmArg(int curCmd, unsigned char* cmdArr, size_t* ip, FILE* rezult);
