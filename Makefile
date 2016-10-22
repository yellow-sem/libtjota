CC=gcc
V=valgrind

CF_GLIB=`pkg-config glib-2.0 --libs --cflags`
CF=$(CF_GLIB) -pthread

V_ARGS=--tool=memcheck --leak-check=full --track-origins=yes

OUT=./main

all:
	$(CC) src/*.c -o $(OUT) $(CF)
clean:
	rm $(OUT)
debug:
	$(V) $(V_ARGS) -v $(OUT)
