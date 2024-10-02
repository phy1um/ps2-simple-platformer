
SRC_FILES=$(shell find . -type f -name '*.c')
EE_OBJS=$(patsubst %.c,%.o,$(SRC_FILES))
EE_BIN=game.elf
EE_INCS+=-I./include
EE_LIBS= -ldma -ldraw -lgraph lib/libp2g.a 
EE_CFLAGS+=-DLOG_TRACE

all: $(EE_BIN)

clean: 
	rm -rf $(EE_BIN) $(EE_OBJS)

run: $(EE_BIN)
	pcsx2-qt $(shell pwd)/$(EE_BIN)

include $(PS2SDK)/samples/Makefile.eeglobal
include $(PS2SDK)/samples/Makefile.pref

