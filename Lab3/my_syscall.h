#ifndef __my_syscall__
#define __my_syscall__

typedef struct {
	int *buff;
	int size;
	int cap;
} IndexArray;

int v2p(unsigned long, unsigned long*, int);

int v2i(unsigned long, int*);

int i2p(int, int unsigned long*, int);

int readChars(int, char*, int);

int readLine(int);

#endif
