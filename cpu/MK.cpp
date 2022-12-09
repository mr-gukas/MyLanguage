#include <stdlib.h>

int main(void)
{
    system("obj/asm obj/circle.txt obj/bin.txt");
    system("obj/cpu obj/bin.txt");
    system("obj/disasm obj/bin.txt obj/disasm.txt");
    
    return 0;
}

