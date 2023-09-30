cliTest.elf: \
	cliTest.c \
	../inc/cli.h \
	../src/cli.c 

	gcc -g -O0 -o cliTest.elf cliTest.c ../src/cli.c -I../inc 
	
clean:
	rm *.elf