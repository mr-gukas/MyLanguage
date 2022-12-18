#include <stdlib.h>

int main(void)
{
    system("run/front obj/square.ara");
	system("run/middle");
	system("run/back");
    system("run/asm obj/arm.asm obj/bin.txt");
    system("run/cpu obj/bin.txt");
    
    return 0;
}

