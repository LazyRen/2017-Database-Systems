.SUFFIXES: .c .o

CC=gcc

SRCDIR=src/
INC=include/
LIBS=lib/

# SRCS:=$(wildcard src/*.c)
# OBJS:=$(SRCS:.c=.o)

# main source file
TARGET_SRC:=$(SRCDIR)main.c
TARGET_OBJ:=$(SRCDIR)main.o
# Include more files if you write another source file.
SRCS_FOR_LIB:=$(SRCDIR)bpt.c $(SRCDIR)page.c $(SRCDIR)log_recovery.c
OBJS_FOR_LIB:=$(SRCS_FOR_LIB:.c=.o)

CFLAGS+= -g -Wall -fPIC -I $(INC)

TARGET=main

all: $(TARGET)

$(TARGET): $(TARGET_OBJ)
	$(foreach src, $(SRCS_FOR_LIB),$(shell $(CC) $(CFLAGS) -o $(src:.c=.o) -c $(src)))
	make static_library
	$(CC) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt

clean:
	-rm -f $(TARGET) $(TARGET_OBJ) $(OBJS_FOR_LIB) $(TEST_TARGET) $(TEST_OBJ) $(OBJS_FOR_TEST) $(LIBS)* 

library:
	gcc -shared -Wl,-soname,libbpt.so -o $(LIBS)libbpt.so $(OBJS_FOR_LIB)

static_library:
	ar cr $(LIBS)libbpt.a $(OBJS_FOR_LIB)

rmdb:
	rm -f DATA*
	rm -f *.db
	rm -f ./result.txt

re: 
	make clean
	make rmdb
	make
