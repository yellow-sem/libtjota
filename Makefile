CC=gcc
V=valgrind

CF_BASE=`pkg-config glib-2.0 --libs --cflags` -pthread
CF_PURPLE=`pkg-config purple --libs --cflags`

V_ARGS=--tool=memcheck --leak-check=full --track-origins=yes

SRC=./src
OUT=./out

PLUGINS=~/.purple/plugins

lib:
	$(CC) -shared $(SRC)/libtjota/*.c -o $(OUT)/libtjota.so -fpic \
	    $(CF_BASE)
main:
	$(CC) $(SRC)/main.c -o $(OUT)/main \
	    $(CF_BASE) -L$(OUT) -I$(SRC) -ltjota
tjotapl:
	$(CC) -shared $(SRC)/tjotapl.c -o $(OUT)/tjotapl.so -fpic \
	    $(CF_BASE) $(CF_PURPLE) -L$(OUT) -I$(SRC) -ltjota
run:
	LD_LIBRARY_PATH=$(OUT) $(OUT)/main
copy:
	mkdir -p $(PLUGINS)
	cp $(OUT)/tjotapl.so $(PLUGINS)
pidgin:
	LD_LIBRARY_PATH=$(OUT) pidgin -d
finch:
	LD_LIBRARY_PATH=$(OUT) finch
debug:
	LD_LIBRARY_PATH=$(OUT) $(V) $(V_ARGS) -v $(OUT)/main
