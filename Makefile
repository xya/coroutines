OBJECTS = pingpong.o nested.o coroutine.o switch.o

all: pingpong nested
	
pingpong: pingpong.o coroutine.o switch.o
	gcc -o pingpong pingpong.o coroutine.o switch.o
nested: nested.o coroutine.o switch.o
	gcc -o nested nested.o coroutine.o switch.o
coroutine.o: coroutine.c coroutine.h
	gcc -c -ggdb -o coroutine.o coroutine.c
switch.o: switch-amd64.S
	as -ggdb -o switch.o switch-amd64.S
pingpong.o: pingpong.c coroutine.h
	gcc -c -ggdb -o pingpong.o pingpong.c
nested.o: nested.c coroutine.h
	gcc -c -ggdb -o nested.o nested.c

clean:
	rm -f pingpong nested $(OBJECTS)
