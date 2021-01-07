# ==========================================
#   Unity Project - A Test Framework for C
#   Copyright (c) 2007 Mike Karlesky, Mark VanderVoord, Greg Williams
#   [Released under MIT License. Please refer to license.txt for details]
# ==========================================

#We try to detect the OS we are running on, and adjust commands as needed
C_COMPILER=gcc
ifeq ($(OS),Windows_NT)
  C_COMPILER=C:\Program Files (x86)\CodeBlocks\MinGW\bin\x86_64-w64-mingw32-gcc.exe
  ifeq ($(shell uname -s),) # not in a bash-like shell
	CLEANUP = del /F /Q
	MKDIR = mkdir
  else # in a bash-like shell, like msys
	CLEANUP = rm -f
	MKDIR = mkdir -p
  endif
	TARGET_EXTENSION=.exe
else
	CLEANUP = rm -f
	MKDIR = mkdir -p
	TARGET_EXTENSION=.out
endif
ifeq ($(shell uname -s), Darwin)
C_COMPILER=clang
endif

UNITY_ROOT=./unity

CFLAGS=-std=c11
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wpointer-arith
CFLAGS += -Wcast-align
CFLAGS += -Wwrite-strings
CFLAGS += -Wswitch-default
CFLAGS += -Wunreachable-code
CFLAGS += -Winit-self
CFLAGS += -Wmissing-field-initializers
CFLAGS += -Wno-unknown-pragmas
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wundef
CFLAGS += -Wold-style-definition
CFLAGS += -g -pg
#CFLAGS += -Wno-misleading-indentation

TEST_TARGET_BASE=test
TARGET_BASE=run
TARGET=$(TEST_TARGET_BASE)$(TARGET_EXTENSION)
MAIN_TARGET=$(TARGET_BASE)$(TARGET_EXTENSION)
SRC_FILES=src/rb-tree.c src/tg-tree.c src/tg-bst-tree.c
TEST_SRC_FILES=$(UNITY_ROOT)/src/unity.c test/test-tg-tree.c $(SRC_FILES)
INC_DIRS=-Isrc -I$(UNITY_ROOT)/src
SYMBOLS=-D RB_TREE_DEBUG -D TG_BST_TREE_DEBUG

ifeq ($(OS),Windows_NT)
	TEST_EXEC=./$(TARGET)
else
	TEST_EXEC=valgrind --leak-check=full -v --error-limit=no ./$(TARGET)
endif

all: clean main

main: clean $(SRC_FILES) src/main.c
	$(C_COMPILER) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES) src/main.c -o $(MAIN_TARGET)

test: clean $(TEST_SRC_FILES)
	$(C_COMPILER) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(TEST_SRC_FILES) -o $(TARGET)
	- $(TEST_EXEC)

clean:
	$(CLEANUP) $(TARGET) $(MAIN_TARGET)

ci: CFLAGS += -Werror
ci: default
