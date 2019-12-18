#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include "my_syscall.h"


#define LEN_INIT 20


int main(void){
	extern int errno;
	int pid = 555;
	char path[20];
	IndexArray ia = {
                buff :  (int*) malloc(sizeof(int) * LEN_INIT),
                size :  0,
                cap :  LEN_INIT
        };
	sprintf(path, "/proc/%d/maps", pid);
	int fd = open(path, O_RDONLY | O_SYNC);
	if (fd < 0) {
		printf("Error[%d]: open /proc/%d/maps failed.\n", errno, pid);
		return -1;
	}
	char rdbuf[20];
	int index;
	unsigned long phy;
	char* wtf;
	unsigned long vir_start;
	unsigned long phy_start;
	int index_start;
	int index_end;
	unsigned long vir_end;
	unsigned long phy_end;
	printf("VIRTURAL ADDRESS       PHYSICAL ADDRESS\n");
	int i;
	int j;
	while (readChars(fd, rdbuf, 8) > 0) {
		vir_start = strtoul(rdbuf, &wtf, 16);
		if (v2p(vir_start, &phy_start, pid) == -1) {
        	        printf("convert error.\n");
                	return -1;
	        }
		readChars(fd, rdbuf, 1);
		readChars(fd, rdbuf, 8);
		vir_end = strtoul(rdbuf, &wtf, 16);
		if (v2p(vir_end, &phy_end, pid) == -1) {
        	        printf("convert error.\n");
                	return -1;
	        }
		v2i(vir_start, &index_start);
		v2i(vir_end, &index_end);
		for (j = index_start; j <= index_end; ++j) {
			putIndex(&ia, j);
		}
		printf("0x%08lX-0x%08lX  0x%08lX-0x%08lX\n",
			vir_start, vir_end, phy_start, phy_end);
		readLine(fd);
	}
	PhyArray pa = {
		buff : (unsigned long*) malloc(sizeof(unsigned long) * ia.size),
		size : 0
	};
	int form = 0;
	unsigned long f = 0;
	for(i = 0; i < ia.size; ++i) {
		//if (form==ia.buff[i]){printf("!\n");form=ia.buff[i];}
		if (i2p(ia.buff[i], &phy, pid) != -1) {
			putPhy(&pa, phy);
		}
	}
	//printf("compare: %d, %d\n", ia.size, pa.size);
	outputPhy(&pa);
	close(fd);
	return 0;
}

void putPhy(PhyArray *pa, unsigned long phy) {
	pa -> buff[pa -> size] = phy;
	pa -> size++;
}

void putIndex(IndexArray *ia, int index) {
	checkCapacity(ia);
	ia -> buff[ia -> size] = index;
	ia -> size++;
}

void checkCapacity(IndexArray *ia) {
	if (ia->cap <= ia->size) {
		int *buf = (int*) malloc(2 * sizeof(int) * ia->cap);
		memcpy(buf, ia->buff, ia->cap * sizeof(int));
		ia->cap = ia->cap * 2;
		free(ia->buff);
		ia->buff = buf;
	}	
}

void v2i(unsigned long va, int *index) {
	int pageSize = getpagesize();
        *index = va / pageSize;
}

int i2p(int index, unsigned long *pa, int pid) {
	unsigned long v_offset = index * sizeof(uint64_t);
	uint64_t item = 0;
	char path[20];
	sprintf(path, "/proc/%d/pagemap", pid);
	int fd = open(path, O_RDONLY);
        if (fd < 0) {
                printf("open /proc/self/pagemap failed.\n");
                return -1;
        }
        if (lseek(fd, v_offset, SEEK_SET) == -1) {
                printf("lseek error\n");
		close(fd);
                return -1;
        }
        if (read(fd, &item, sizeof(uint64_t)) != sizeof(uint64_t)) {
                printf("read item error.\n");
		close(fd);
                return -1;
        }
        if ((((uint64_t)1 << 63) & item) == 0) {
                //page present is 0
		//printf("virtual page %d not in physical memory.\n", index);
		close(fd);
                return -1;
        }
        *pa = ((((uint64_t) 1 << 55) - 1) & item) * getpagesize();
	close(fd);
	return 0;
}

int v2p(unsigned long va, unsigned long *pa, int pid){
	int pageSize = getpagesize();
	unsigned long v_pageIndex = va / pageSize;
	unsigned long v_offset = v_pageIndex * sizeof(uint64_t);
	unsigned long page_offset = va % pageSize;
	uint64_t item = 0;
	char path[20];
        sprintf(path, "/proc/%d/pagemap", pid);

	int fd = open(path, O_RDONLY);
	if (fd < 0) {
                printf("open /proc/self/pagemap failed.\n");
                return -1;
        }
	if (lseek(fd, v_offset, SEEK_SET) == -1) {
		printf("lseek error\n");
		return -1;
	}
	if (read(fd, &item, sizeof(uint64_t)) != sizeof(uint64_t)) {
		printf("read item error\n");
		return -1;
	}
	if ((((uint64_t)1 << 63) & item) == 0) {
		//printf(" * page present is 0\n");
		//return -1;
	}
	
	
	uint64_t p_pageIndex = ((((uint64_t) 1 << 55) - 1) & item);
	*pa =  (p_pageIndex * pageSize) + page_offset;
        /*
	printf(" *********************************************\n"
               " * virtual addr: 0x%lX\n"
               " * virtual page index: %ld\n"
	       " * physical page index: %lld\n"
               " * in-page offset: %ld\n"
	       " * the No.%ld item in page map is: 0x%016llX\n",
               va, v_pageIndex, p_pageIndex, page_offset, v_pageIndex, item);	
	*/

	close(fd);
	return 0;
}

int isHexNum(char c) {
	if (c >= 48 && c <= 57) {
		return 1;
	} else if (c >= 97 && c <= 102) {
		return 1;
	}
	return 0;
}

int readChars(int fd, char* rdbuf, int readNum) {
	int status = read(fd, rdbuf, readNum * sizeof(char));
	if (status != readNum * sizeof(char)) {
                if (errno != 0) {
			printf("Error[%d]: read file error.\n", errno);
                }
		return -1;
        } else if (!isHexNum(rdbuf[0])) {
		return -1;
	} else {
		return status;
	}

}

int readLine(int fd) {
	char buf;
	do {
		if (read(fd, &buf, sizeof(char)) != sizeof(char)) {
                	printf("Error[%d]: read file error.\n", errno);
                	return -1;
        	}
	} while (buf != '\n');
	return 0;
}

void outputPhy(PhyArray* pa) {
	if (pa -> size == 0) {printf("none in memory!\n"); return;}
	if (pa -> size == 1) {printf("only page %lx in mem.\n", pa -> buff[0]); return;}
	printf("Pages that are present in physical memory:\n"
	       "> %d pages in total, 4K per page, marked by its beginning address <\n", pa -> size);
	unsigned long start, end;
	start = pa -> buff[0];
	end = pa -> buff[0];
	int i;
	for (i = 1; i < pa -> size; ++i) {
		if (end == pa-> buff[i]) {
			continue;
		} else if (end + 0x00001000 == pa -> buff[i]) {
			end = pa -> buff[i];
		} else {
			printf("0x%08lX - 0x%08lX\n", start, end);
			start = pa -> buff[i];
			end = pa -> buff[i];
		}

	}	



}
