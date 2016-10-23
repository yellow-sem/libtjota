CC=gcc
V=valgrind

CF_GLIB=`pkg-config glib-2.0 --libs --cflags`
CF_PURPLE=`pkg-config purple --libs --cflags`
CF=$(CF_GLIB) $(CF_PURPLE) -pthread

V_ARGS=--tool=memcheck --leak-check=full --track-origins=yes

SRC=./src
OUT=./out

PLUGINS=~/.purple/plugins

lib:
	$(CC) -shared $(SRC)/libtjota/*.c -o $(OUT)/libtjota.so -fpic $(CF)
main:
	$(CC) $(SRC)/main.c -o $(OUT)/main $(CF) -I$(SRC) -L$(OUT) -ltjota
purple:
	$(CC) -shared $(SRC)/purple.c -o $(OUT)/purple.so -fpic $(CF) -I$(SRC)
copy:
	mkdir -p $(PLUGINS)
	cp $(OUT)/purple.so $(PLUGINS)
run:
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(OUT) $(OUT)/main
debug:
	$(V) $(V_ARGS) -v $(OUT)/main
