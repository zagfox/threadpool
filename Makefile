CC = gcc

CFLAGS = -Wall -Isrc
LDLIBS = -lpthread

TARGETS = test/threadpool_test1 test/threadpool_test2

all: $(TARGETS)

test/threadpool_test1: test/threadpool_test1.o src/threadpool.o
test/threadpool_test2: test/threadpool_test2.o src/threadpool.o

src/libthreadpool.o: src/threadpool.c src/threadpool.h
	$(CC) -c ${CFLAGS} -o $@ $<

clean:
	rm -f $(TARGETS) *~ */*~ */*.o
