CC = cc
9cc: 
	$(CC) -std=c99 9cc.c -o 9cc

test: 9cc
		./test.sh

clean:
		rm -rf 9cc *.o *~ tmp* *.out
