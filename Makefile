TARGET = pingpong
OBJECTS = pingpong.o coroutine.o switch.o

all: $(TARGET)
	
$(TARGET): $(OBJECTS)
	gcc -o $@ $(OBJECTS)

coroutine.o: coroutine.c coroutine.h
	gcc -c -ggdb -o coroutine.o coroutine.c
switch.o: switch-amd64.S
	as -ggdb -o switch.o switch-amd64.S
pingpong.o: pingpong.c coroutine.h
	gcc -c -ggdb -o pingpong.o pingpong.c

clean:
	rm -f $(TARGET) $(OBJECTS)
