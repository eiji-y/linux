void __delay(unsigned long loops)
{
	__asm__ __volatile__("\
	bz	$0,2f\n\
1:	subu	$0,$0,1\n\
	pbnz	$0,1b\n\
2:"
	: "=r"(loops));
}
