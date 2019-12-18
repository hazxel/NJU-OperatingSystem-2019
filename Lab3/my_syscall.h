#ifndef __my_syscall__
#define __my_syscall__

typedef struct {
	int *buff;
	int size;
	int cap;
} IndexArray;

typedef struct{
	unsigned long *buff;
	int size;
} PhyArray;

void putIndex(IndexArray*, int);

void putPhy(PhyArray*, unsigned long);

void checkCapacity(IndexArray*);

int isHexNum(char);

int v2p(unsigned long, unsigned long*, int);

void v2i(unsigned long, int*);

int i2p(int, int unsigned long*, int);

int readChars(int, char*, int);

int readLine(int);

void outputPhy(PhyArray*);
#endif
