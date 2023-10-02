cliTest.elf: \
	cliTest.c \
	../inc/cli_t.h 

	gcc -g -O0 -o cliTest.elf cliTest.c -I../inc -I../extern/cSuite/cAsciiPrinter/inc -I../extern/cSuite/cAsciiParser/inc  -I../../cAsciiParser/inc -I../../cAsciiPrinter/inc
	
clean:
	rm *.elf