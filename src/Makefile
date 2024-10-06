
SRC_FILES=$(shell find . -type f -name '*.c')
EE_OBJS=$(patsubst %.c,%.o,$(SRC_FILES))
EE_BIN=game.elf
EE_INCS+=-I./include
EE_LIBS= -ldma -ldraw -lgraph -lpad lib/libp2g.a 
EE_CFLAGS+=-DLOG_TRACE -std=c11

PS2HOST=192.168.20.99

all: $(EE_BIN)

clean: 
	rm -rf $(EE_BIN) $(EE_OBJS)

run: $(EE_BIN)
	pcsx2-qt $(shell pwd)/$(EE_BIN)

runps2: $(EE_BIN)
	ps2client -h $(PS2HOST) -t 10 execee host:$(EE_BIN)

.PHONY: resetps2
resetps2:
	ps2client -h $(PS2HOST) -t 5 reset


include $(PS2SDK)/samples/Makefile.eeglobal
include $(PS2SDK)/samples/Makefile.pref

