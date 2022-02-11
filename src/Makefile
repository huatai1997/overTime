CFLAGS += -g -O2 -Wall -Werror -Wno-unused
TARGET = main

$(TARGET) : *.c
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm -rf $(TARGET)
