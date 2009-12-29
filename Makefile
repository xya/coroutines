TARGET = test_coroutine
OBJECTS = test_coroutine.o coroutine.o switch.o

all: $(TARGET)
	
$(TARGET): $(OBJECTS)
	gcc -o $@ $(OBJECTS)

%.o: %.c
	gcc -c -o $@ $<
coroutine.o: coroutine.c coroutine.h
	gcc -c -o coroutine.o coroutine.c
switch.o: switch.s
	as -o switch.o switch.s
test_coroutine.o: test_coroutine.c coroutine.h
	gcc -c -o test_coroutine.o test_coroutine.c

clean:
	rm -f $(TARGET) $(OBJECTS)
