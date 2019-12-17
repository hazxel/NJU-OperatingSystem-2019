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

int v2p(unsigned long, unsigned long*);

int main(void){
	extern int errno;
	int pid = 917;
	char path[20];
	sprintf(path, "/proc/%d/maps", pid);
	int fd = open(path, O_RDONLY | O_SYNC);
	if (fd < 0) {
		printf("Error[%d]: open /proc/%d/maps failed.\n", errno, pid);
		return -1;
	}
	char rdbuf[20];
	if (read(fd, &rdbuf, 8 * sizeof(char)) != 8 * sizeof(char)) {
                printf("Error[%d]: read file error.\n", errno);
                return -1;
        }
	rdbuf[8] = '\0';
	printf("%s\n", rdbuf);
	char* sttr;
	unsigned long aa = strtol(rdbuf, &sttr, 16);
	printf("%08lx\n", aa);
	return 0;
	



	int a=0;
	//convert the address
	unsigned long vir = (unsigned long)&a;
	unsigned long phy = 0;
	if (v2p(vir, &phy) == -1) {
		printf("convert error.\n");
		return -1;
	}
	printf("the virtual  address is: 0x%08lX\nthe physical address is: 0x%08lX\n", vir, phy);


	//print the pages
	fd = open("/dev/mem", O_RDONLY | O_SYNC);
	if (fd < 0){
		printf("open /dev/mem failed.\n");
		return -1;
	}
	/*
	if (lseek(fd, 1048574, SEEK_SET) == -1) {
                printf("lseek error %d\n", errno);
		close(fd);
                return -1;
        }
	if (read(fd, rdbuf, 2) != 2) {
		printf("read error %d\n", errno);
		close(fd);
		return -1;
	}
	*/
	//rdbuf = (char*) mmap(phy, phy, PROT_READ, MAP_SHARED, fd, 0);
	if (rdbuf < 0) {
		print("mmap faied %d", errno);
		close(fd);
		return -1;
	}
	int i;
	for (i = 0; i < 1; i = i + 1) {
		printf("mem[%d]:0x%02X\n",i,*(rdbuf + i) & 0xff);
	}
	//read(fd, rdbuf, phy + 1);
	//printf("%d\n", *(rdbuf + phy) & 0xff);
	
	close(fd);
	return 0;
}

int v2p(unsigned long va, unsigned long *pa){
	int pageSize = getpagesize();
	unsigned long v_pageIndex = va / pageSize;
	unsigned long v_offset = v_pageIndex * sizeof(uint64_t);
	unsigned long page_offset = va % pageSize;
	uint64_t item = 0;
	printf(" ** virtual addr: 0x%lX\n"
	       " ** page size: %d\n"
	       " ** entry size: %d\n"
	       " ** virtual page index: %ld\n"
	       " ** in-page offset: %ld\n",
	       va, pageSize, sizeof(uint64_t), v_pageIndex, page_offset);	

	int fd = open("/proc/self/pagemap", O_RDONLY);
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
		printf("page present is 0\n");
		return -1;
	}
	
	printf(" ** the No.%ld item in page map is: 0x%016llX\n",v_pageIndex, item);
	
	uint64_t p_pageIndex = ((((uint64_t) 1 << 55) - 1) & item);
	*pa =  (p_pageIndex * pageSize) + page_offset;
	printf(" ** physical page index: %lld\n", p_pageIndex);
	
	//struct task_struct *pcb_tmp = NULL;
        //pgd_t *pgd_tmp = NULL;
        //pud_t *pud_tmp = NULL;
        //pmd_t *pmd_tmp = NULL;
        //pte_t *pte_tmp = NULL;
	//if(!(pcb_tmp = findTaskByPid(pid)))
        //{
        //        printf("Can't find the task %d .\n",pid);
        //        return 0;
        //}
	//printf("pgd = 0x%p\n",pcb_tmp->mm->pgd);
	//struct task_struct *curr= get_current();     
	close(fd);
	return 0;
}



