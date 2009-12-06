# Simple Makefile for compiling test programs
#
# Author: Rebekah Leslie
# Group: Thomas DuBuisson and John Ochsner

CC = gcc

TESTS =  \
  test1  \
  test2  \
  test3  \
  test4  \
  test5  \
  test6  \
  test7  \
  test8  \
  test9  \
  test10 \
  test11 

LIBS = \
  utils.o

all: $(LIBS) $(TESTS)

%.o : %.c 
	$(CC) -o $@ -c $<

% : %.c  $(LIBS)
	$(CC) -o $@ $< $(LIBS)

clean:
	rm -f $(TESTS)
	rm -f $(LIBS)
