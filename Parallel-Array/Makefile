CC=C:\Program Files (x86)\CodeBlocks\MinGW\bin\x86_64-w64-mingw32-gcc.exe
CFLAGS=-g -Wall -Werror
# LDFLAGS=
# LDLIBS=
TRIVIAL_SRCS=main.c parallel-trivial.c
IMPROVE_SRCS=main.c parallel-improve.c
TRIVIAL_OBJS=main.o parallel-trivial.o
IMPROVE_OBJS=main.o parallel-improve.o
TARGET=trivial improve

all: $(TARGET)

trivial: $(TRIVIAL_OBJS)
	$(CC) -D TRIVIAL -c $(TRIVIAL_SRCS)
	$(CC) -o trivial $(TRIVIAL_OBJS)

improve: $(IMPROVE_OBJS)
	$(CC) -c $(IMPROVE_SRCS)
	$(CC) -o improve $(IMPROVE_OBJS)

clean:
	del *.o 
	del $(TARGET)
