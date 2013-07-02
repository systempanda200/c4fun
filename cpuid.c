#include <stdio.h>

int main() {
    unsigned int resultEax;
    unsigned int resultEbx;
    __asm__("movl $10, %%eax" : ); // Moves 10 in EAX: CPUID input param to get performance monitoring info
    __asm__("cpuid" : );
    __asm__("movl %%eax, %0"
    	    :"=r"(resultEax)
    	    :
	    :
    	    );
    __asm__("movl %%ebx, %0"
    	    :"=r"(resultEbx)
    	    :
	    :
    	    );
    printf("Version ID of architectural performance monitoring = %u\n", resultEax & 255); // Bits 07 to 00
    printf("Number of general-purpose performance monitoring counter per logical processor = %u\n", (resultEax >> 8) & 255); // Bits 15 to 08}
    printf("Last level cache misses = %u (0 if available, 1 if not)\n", (resultEbx >> 4) & 1); // Bits 04
    return 0;
}
