all : lamport

lamport : lamport.o
	gcc-4.9 -pie -fsanitize=thread -std=c11 -g -o $@ $^ -ltsan

%.o : %.c Makefile
	gcc-4.9 -fPIC -fsanitize=thread -std=c11 -g -c -o $@ $<


