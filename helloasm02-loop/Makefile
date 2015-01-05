
hello_amd64: helloasm02_amd64.o
	ld -m elf_x86_64 -s -o $@ $<

helloasm02_amd64.o: helloasm02.asm
	yasm -m amd64 -f elf $< -o $@

hello_i386: helloasm02_i386.o
	ld -m elf_i386 -s -o $@ $<

helloasm02_i386.o: helloasm02.asm
	yasm -f elf $< -o $@

.PHONY: hello_amd64 hello_i386

clean:
	rm -f *.o 