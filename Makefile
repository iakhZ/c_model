CC = gcc
GCCVER =
#CC = g++
DEFINES = -D_FILE_OFFSET_BITS=64
#JFLAGS = -std=c99 -g -Wall
JFLAGS = -std=gnu99 -O3 -g -Wall

# =================================================================================

dsc_DEFS = \
	dsc_codec.h \
	dsc_types.h \
	dsc_utils.h \
	cmd_parse.h \
	dpx.h \
	fifo.h \
	hdr_dpx.h \
	logging.h \
	multiplex.h \
	psnr.h \
	utl.h \
	vdo.h \

dsc_SRCS = \
	dsc_codec.c \
	dsc_utils.c \
	cmd_parse.c \
	codec_main.c \
	dpx.c \
	fifo.c \
	hdr_dpx.c \
	logging.c \
	multiplex.c \
	psnr.c \
	utl.c

dsc_OBJS = ${dsc_SRCS:.c=.o}

# ----------------------------------------------------------------

test: $(dsc_OBJS)
	$(CC) $(dsc_OBJS) -lm -o dsc_test

# ----------------------------------------------------------------
.c.o:
	$(CC) $(JFLAGS) $(DEFINES) -c $*.c 

.c.ln:
	lint -c $*.c 

% : %.c vdo.h utl.c utl.h dpx.h
	gcc -O -o $@ $@.c utl.c dpx.c -lm -W -Wall -std=c99

all: test


dsc:
	dsc_test -F test.cfg
	
clean:
	rm -f *.ref.*
	rm -f *.out.*
	rm -f *.dsc
