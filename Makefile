all:
	gcc -m32 -o xkcl xkcl.c -g
clean:
	rm xkcl
	rm -rf xkcl.dSYM
