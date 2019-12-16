#include <stdio.h>
#include <stdlib.h>
//#include <fcntl.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
//#include <sys/stat.h>
//#include <stdint.h>
//#include <sys/mman.h>
#include <asm/pgtable.h>

int main(void){
	extern int errno;
	int a = 2;
	
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
	struct task_struct *curr= get_current();     
	return 0;
}



