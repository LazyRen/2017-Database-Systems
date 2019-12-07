.SUFFIXES: .c .o

CC=gcc

# SRCS:=$(wildcard src/*.c)
# OBJS:=$(SRCS:.c=.o)

# main source file
TARGET_SRC:=main.c
# Include more files if you write another source file.
SRCS_FOR_LIB:=page.c
OBJS_FOR_LIB:=$(SRCS_FOR_LIB:.c=.o)

CFLAGS+= -Wall

TARGET=print_tree

all: $(TARGET)

$(TARGET): $(TARGET_SRC) $(SRCS_FOR_LIB)
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET_SRC) $(SRCS_FOR_LIB)

clean:
	-rm -f $(TARGET) $(TARGET_OBJ) $(OBJS_FOR_LIB)

re: 
	make clean
	make
