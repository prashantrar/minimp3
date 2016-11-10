# note: this Makefile builds the Linux version only

#CFLAGS = -Wall -Os  -O3
#CFLAGS += -ffast-math
#CFLAGS += -finline-functions-called-once
#CFLAGS += -fno-loop-optimize
#CFLAGS += -fexpensive-optimizations
#CFLAGS += -fpeephole2

STRIPFLAGS  = -R .comment
STRIPFLAGS += -R .note
STRIPFLAGS += -R .note.ABI-tag
STRIPFLAGS += -R .gnu.version

BIN = minimp3
FINALBIN = $(BIN)-linux
OBJS = player_amebamod.o minimp3.o

all:	$(BIN)

release:	$(BIN)
	strip $(STRIPFLAGS) $(BIN)
	upx --brute $(BIN)

test:	$(BIN)
	./$(BIN) "../../../Gargaj -- Rude Awakening.mp3"

$(BIN):	$(OBJS)
	gcc -g $(OBJS) -o $(BIN) -lm -O3

%.o:	%.c
	gcc -g $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BIN) $(OBJS)

dist:	clean release
	mv $(BIN) $(FINALBIN)
	rm -f $(OBJS) *.o *~
