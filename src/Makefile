
SRC_FILES=$(shell find . -type f -name '*.c' -not -path './test/*' -not -path './sim/*' -not -path './tools/*')
EE_OBJS=$(patsubst %.c,%.o,$(SRC_FILES))
EE_BIN=game.elf
EE_INCS+=-I./include
EE_LIBS= -ldma -ldraw -lgraph -lpad lib/libp2g.a 
EE_CFLAGS+=-DLOG_TRACE -std=c11

PS2HOST=192.168.20.99

all: $(EE_BIN)

clean: 
	rm -rf $(EE_BIN) $(EE_OBJS) sim/p2g.o

run: $(EE_BIN)
	pcsx2-qt $(shell pwd)/$(EE_BIN)

runps2: $(EE_BIN)
	ps2client -h $(PS2HOST) -t 500 execee host:$(EE_BIN)

.PHONY: resetps2
resetps2:
	ps2client -h $(PS2HOST) -t 5 reset

tools/packwad: tools/makewad.c wad.c
	gcc $(EE_CFLAGS) $(EE_INCS) -g -o $@ $^

tools/readwad: tools/readwad.c wad.c
	gcc $(EE_CFLAGS) $(EE_INCS) -g -o $@ $^

tools: tools/packwad tools/readwad


ifeq ($(platform), PC)
CFLAGS+=$(EE_INCS) -I./sim/include $(EE_CFLAGS)

$(EE_BIN): $(EE_OBJS) sim/p2g.o
	gcc -g -o sim.elf $(CFLAGS) $^

level_test: test/level_load.o game/context.o io.o levels/fmt.o io.o vram.o tiles.o game/entity.o wad.o alloc.o tga.o game/camera.o levels/test_load.o levels/test_entry.o levels/test_adj.o levels/shared.o game/trigger.o
	gcc -g -o tst $^ $(CFLAGS)
else
include $(PS2SDK)/samples/Makefile.eeglobal
include $(PS2SDK)/samples/Makefile.pref
endif

