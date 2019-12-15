#include <stdio.h>
#include <stdlib.h>
#include <asm/page.h>
#include <linux/dup_types.h>

unsigned long v2p(int, unsigned long);

int main(void){
	v2p(1292, 0);

	int fd;
	char *rdbuf;
	int i;
	fd = open("/dev/mem",O_RDWR|O_SYNC);
	if(fd < 0){
		printf("open /dev/mem failed.\n");
		return -1;
	}
	read(fd,rdbuf,10);
	for(i = 0;i < 10;i++)
	{
		printf("mem[%d]:0x%02X\n",i,*(rdbuf + i) & 0xff);
	}
	close(fd);
	return 0;
}

unsigned long v2p(int pid, unsigned long va){
	unsigned long pa = 0;
	struct task_struct *pcb_tmp = NULL;
        pgd_t *pgd_tmp = NULL;
        pud_t *pud_tmp = NULL;
        pmd_t *pmd_tmp = NULL;
        pte_t *pte_tmp = NULL;
	if(!(pcb_tmp = findTaskByPid(pid)))
        {
                printf("Can't find the task %d .\n",pid);
                return 0;
        }
	printf("pgd = 0x%p\n",pcb_tmp->mm->pgd);
}



