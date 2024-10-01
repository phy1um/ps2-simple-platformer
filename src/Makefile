
EE_OBJS=main.o draw.o tga.o tiles.o
EE_BIN=game.elf
EE_INCS+=-I./include
EE_LIBS= -ldma -ldraw -lgraph lib/libp2g.a 
EE_CFLAGS+=-DLOG_TRACE

all: $(EE_BIN)

clean: 
	rm -rf $(EE_BIN) $(EE_OBJS)


include $(PS2SDK)/samples/Makefile.eeglobal
include $(PS2SDK)/samples/Makefile.pref

