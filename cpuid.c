/* #include <stdio.h> */

/* int main() { */

/*     unsigned int resultEax; */
/*     unsigned int resultEbx; */
/*     unsigned int resultEdx; */

/*     __asm__("movl $0xa, %%eax" : ); // Moves 0xA in EAX: CPUID input param to get performance monitoring info */
/*     __asm__("cpuid" : ); */
/*     __asm__("movl %%eax, %0" :"=r"(resultEax) : :); */
/*     __asm__("movl %%ebx, %0" :"=r"(resultEbx) : :); */
/*     __asm__("movl %%edx, %0" :"=r"(resultEdx) : :); */

/*     printf("%-82s =  %2u\n", "Version ID of architectural performance monitoring" , resultEax & 255U); // Bits 07 to 00 */
/*     printf("%-82s =  %2u\n", "Number of general-purpose performance monitoring counter per logical processor", (resultEax >> 8) & 255); // Bits 15 to 08 */
/*     printf("%-82s =  %2u\n", "Bit width of general-purpose performance monitoring counter", (resultEax >> 16) & 255); // Bits 23 to 16 */
/*     printf("%-82s =  %2u\n", "Length of EBX bit vector to enumerate architectural performance monitoring events", (resultEax >> 24) & 255); // Bits 31 to 24 */
/*     printf("\n"); */

/*     printf("%-82s =  %2u\n", "Core cycle event (0 if available, 1 if not)", resultEbx & 1); // Bits 00 */
/*     printf("%-82s =  %2u\n", "Instruction retired event (0 if available, 1 if not)", (resultEbx >> 1) & 1); // Bits 01 */
/*     printf("%-82s =  %2u\n", "Reference cycles event (0 if available, 1 if not)", (resultEbx >> 2) & 1); // Bits 02 */
/*     printf("%-82s =  %2u\n", "Last level cache reference event (0 if available, 1 if not)", (resultEbx >> 3) & 1); // Bits 03 */
/*     printf("%-82s =  %2u\n", "Last level cache misses event (0 if available, 1 if not)", (resultEbx >> 4) & 1); // Bits 04 */
/*     printf("%-82s =  %2u\n", "Branch instruction retired event (0 if available, 1 if not)", (resultEbx >> 5) & 1); // Bits 05 */
/*     printf("%-82s =  %2u\n", "Branch mispredict retired event (0 if available, 1 if not)", (resultEbx >> 6) & 1); // Bits 06 */
/*     printf("\n"); */

/*     printf("%-82s =  %2u\n", "Number of fixed-function performance counters", (resultEdx >> 0) & 15); // Bits 04 to 00 */
/*     printf("%-82s =  %2u\n", "Bit width of fixed-function performance counters", (resultEdx >> 5) & 255); // Bits 12 to 05 */

/*     return 0; */
/* } */

#include <cpuid.h>
#include <stdio.h>
int main()
{
	unsigned a, b, c, d;
	/* check __get_cpuid_max here */
	__cpuid(10, a, b, c, d);
	printf("eax: %x ebx %x ecx %x edx %x\n", a, b, c, d);
	int i;
	for (i = 0; i < 10; i++)
		if (b & (1 << i))
			printf("event %d not supported\n", i);
	return 0;
}
