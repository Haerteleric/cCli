cliTest.elf: \
	cliTest.c \
	../inc/cli.h 

	gcc -g -O0 -o cliTest.elf cliTest.c -I../inc -I../extern/cSuite/cAsciiPrinter/inc -I../extern/cSuite/cAsciiParser/inc  
	
clean:
	rm *.elf