CC=
CFLAGS=-Wall -Werror #-pg -g 
# LDFLAGS=
LDLIBS=
SRCS=main.c dynamic-array.c
OBJS=main.o dynamic-array.o
RM=
TARGET=

ifeq ($(OS), Windows_NT)
    CC=C:\Program Files (x86)\CodeBlocks\MinGW\bin\x86_64-w64-mingw32-gcc.exe
    RM=del
	TARGET=a.exe
else
	CC=gcc
    RM=rm
	TARGET=a.out
	LDLIBS=-lm
endif

all: $(TARGET)

clean:
	$(RM) $(OBJS)
	$(RM) $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDLIBS)