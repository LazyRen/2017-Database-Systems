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
TEST_SRC:=$(SRCDIR)main_origin.c
TEST_OBJ:=$(SRCDIR)main_origin.o
# Include more files if you write another source file.
SRCS_FOR_LIB:=$(SRCDIR)bpt.c $(SRCDIR)page.c
OBJS_FOR_LIB:=$(SRCS_FOR_LIB:.c=.o)
SRCS_FOR_TEST:=$(SRCDIR)bpt_origin.c
OBJS_FOR_TEST:=$(SRCS_FOR_TEST:.c=.o)

CFLAGS+= -g -fPIC -I $(INC)

TARGET=main
TEST_TARGET=main_origin

all: $(TARGET)

$(TARGET): $(TARGET_OBJ)
	$(foreach src, $(SRCS_FOR_LIB),$(shell $(CC) $(CFLAGS) -o $(src:.c=.o) -c $(src)))
	# $(CC) $(CFLAGS) -o $(OBJS_FOR_LIB) -c $(SRCS_FOR_LIB)
	make static_library
	$(CC) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt

test: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJ)
	$(foreach src, $(SRCS_FOR_TEST),$(shell $(CC) $(CFLAGS) -o $(src:.c=.o) -c $(src)))
	make static_library_test
	$(CC) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt_origin

clean:
	-rm -f $(TARGET) $(TARGET_OBJ) $(OBJS_FOR_LIB) $(TEST_TARGET) $(TEST_OBJ) $(OBJS_FOR_TEST) $(LIBS)* 

library:
	gcc -shared -Wl,-soname,libbpt.so -o $(LIBS)libbpt.so $(OBJS_FOR_LIB)

static_library:
	ar cr $(LIBS)libbpt.a $(OBJS_FOR_LIB)

static_library_test:
	ar cr $(LIBS)libbpt_origin.a $(OBJS_FOR_TEST)
rmdb:
	rm -f ./mydb
re: 
	make clean
	make rmdb
	make
