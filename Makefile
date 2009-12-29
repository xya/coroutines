TARGET = test_coroutine
OBJECTS = test_coroutine.o coroutine.o

all: $(TARGET)
	
$(TARGET): $(OBJECTS)
	gcc -o $@ $(OBJECTS)

%.o: %.c
	gcc -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS)
