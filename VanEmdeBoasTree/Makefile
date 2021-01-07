CC=
CXX=g++
CFLAGS=-Wall -Werror -Wextra
LDFLAGS=
# LDLIBS=
SRCS=main.c van-emde-boas.c
OBJS=main.o van-emde-boas.o
RM=
TARGET=boas bitset

ifeq ($(OS), Windows_NT)
    CC=C:\Program Files (x86)\CodeBlocks\MinGW\bin\x86_64-w64-mingw32-gcc.exe
	CXX=C:\Program Files (x86)\CodeBlocks\MinGW\bin\x86_64-w64-mingw32-g++.exe
    RM=del
else
	CC=gcc
	CXX=g++
    RM=rm
	LDFLAGS=-lm
endif

all: $(TARGET)

clean:
	$(RM) $(OBJS)
	$(RM) $(TARGET)

boas: $(OBJS)
	$(CXX) $(CFLAGS) -c main.c
	$(CXX) -pg -o boas $(OBJS) $(LDFLAGS)

bitset: $(OBJS)
	$(CXX) -D BITSET $(CFLAGS) -c main.c
	$(CXX) -pg -o bitset $(OBJS) $(LDFLAGS)

van-emde-boas.o: van-emde-boas.c
	$(CC) $(CFLAGS) -c van-emde-boas.c