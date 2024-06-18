CC = gcc
CFLAGS = -Wall -Wextra -g
OBJ = smalloc.c

all: test1 test2 test3 test4 test5 test6 test7

test1: test1.c $(OBJ)
	$(CC) $(CFLAGS) -o test1 test1.c $(OBJ)

test2: test2.c $(OBJ)
	$(CC) $(CFLAGS) -o test2 test2.c $(OBJ)

test3: test3.c $(OBJ)
	$(CC) $(CFLAGS) -o test3 test3.c $(OBJ)

test4: test4.c $(OBJ)
	$(CC) $(CFLAGS) -o test4 test4.c $(OBJ)

test5: test5.c $(OBJ)
	$(CC) $(CFLAGS) -o test5 test5.c $(OBJ)

test6: test6.c $(OBJ)
	$(CC) $(CFLAGS) -o test6 test6.c $(OBJ)	

clean:
	rm -rf test1 test2 test3 test4 test5 test6

