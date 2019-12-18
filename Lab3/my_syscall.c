#include <stdio.h>
#include <stdlib.h>
//#include <asm/page.h>
//#include <linux/types.h>
//#include <linux/dup_types.h>
#include <fcntl.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>

#include <sys/mman.h>

int v2p(unsigned long, unsigned long*, int);

int readChars(int, char*, int);

int readLine(int);

int main(void){
	extern int errno;
	int pid = 555;
	char path[20];
	sprintf(path, "/proc/%d/maps", pid);
	int fd = open(path, O_RDONLY | O_SYNC);
	if (fd < 0) {
		printf("Error[%d]: open /proc/%d/maps failed.\n", errno, pid);
		return -1;
	}
	char rdbuf[20];
	char* wtf;
	unsigned long vir_start;
	unsigned long phy_start;
	unsigned long vir_end;
	unsigned long phy_end;
	int i;
	for (i = 0; i < 20; ++i) {
		readChars(fd, rdbuf, 8);
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
		printf("debug: VIRTUAL: 0x%08lX - 0x%08lX, PHYSICAL: 0x%08lX - 0x%08lX\n",
			vir_start, vir_end, phy_start, phy_end);
		readLine(fd);
	}
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

int readChars(int fd, char* rdbuf, int readNum) {
	int status = read(fd, rdbuf, readNum * sizeof(char));
	if (status != readNum * sizeof(char)) {
                printf("Error[%d]: read file error.\n", errno);
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
