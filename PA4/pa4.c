/**********************************************************************
 * Copyright (c) 2020
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include "types.h"
#include "list_head.h"
#include "vm.h"

/**
 * Ready queue of the system
 */
extern struct list_head processes;

/**
 * Currently running process
 */
extern struct process *current;

/**
 * Page Table Base Register that MMU will walk through for address translation
 */
extern struct pagetable *ptbr;


/**
 * The number of mappings for each page frame. Can be used to determine how
 * many processes are using the page frames.
 */
extern unsigned int mapcounts[];


/**
 * alloc_page(@vpn, @rw)
 *
 * DESCRIPTION
 *   Allocate a page frame that is not allocated to any process, and map it
 *   to @vpn. When the system has multiple free pages, this function should
 *   allocate the page frame with the **smallest pfn**.
 *   You may construct the page table of the @current process. When the page
 *   is allocated with RW_WRITE flag, the page may be later accessed for writes.
 *   However, the pages populated with RW_READ only should not be accessed with
 *   RW_WRITE accesses.
 *
 * RETURN
 *   Return allocated page frame number.
 *   Return -1 if all page frames are allocated.
 */
struct process proc[NR_PTES_PER_PAGE];    //process들의 집합
int mallocarr[NR_PTES_PER_PAGE] ={0,};   //page table의 outer ptes의 동적할당 여부
static int pagecount = 0;           //pfn를 나타낼 변수

int forked[4] = {1,0,};    //process가 fork 됬는지 
bool cow = false;    //copy on write를 해야 하는지


int available_pfn(unsigned int mapcount[]){		//find smallest pfn
	//가능한 pfn중 가장 작은 값 찾기 위한 것
	for(int i=0;i<NR_PAGEFRAMES;i++){
		if(mapcounts[i] == 0){
			return i;
		}
	}
	return -1;
}

unsigned int alloc_page(unsigned int vpn, unsigned int rw)
{
	int outidx = vpn / 16;  //outer index
	int inidx = vpn % 16;	//inner index
	pagecount = available_pfn(mapcounts);
	if(pagecount == -1){        //가능한 pfn이 없는 경우
		return -1;
	}
	
	if(!mallocarr[outidx]){     //outer ptes가 만들어지지 않았다면 동적할당을 해줘야 함
		proc[current->pid].pagetable.outer_ptes[outidx] = malloc(sizeof(struct pte_directory));
		mallocarr[outidx] = 1;   //malloc이 됬다는 표시
	}
	//printf("%d\n",mapcounts[0]);
	proc[current->pid].pagetable.outer_ptes[outidx]->ptes[inidx].pfn = pagecount;   //pagecount를 통해 page frame number를 나타냄
	mapcounts[pagecount]++;   //mapcount값 늘려줌
	
	
	//printf("check!! : %d\n",current->pagetable.outer_ptes[outidx]->ptes[inidx].pfn);
	proc[current->pid].pagetable.outer_ptes[outidx]->ptes[inidx].valid = 1;         //valid bit 켜줌
	if(rw >= 0x02){
		proc[current->pid].pagetable.outer_ptes[outidx]->ptes[inidx].writable = 1;      //rw인 경우 writable켜주고
	}
//	current = &proc[current->pid];
	ptbr = &proc[current->pid].pagetable;           //ptbr도 업데이트
	current = &proc[current->pid];              //current도 업데이트

	return ptbr->outer_ptes[outidx]->ptes[inidx].pfn;       //return 부분은 다 같은 것을 가르키니 ptbr의 pfn이든 current든 상관없음.
/*	ptbr = &current->pagetable;
	current->pagetable.outer_ptes[outidx]->ptes[inidx].pfn = pagecount;
	mapcounts[pagecount]++;
	pagecount++;

	return current->pagetable.outer_ptes[outidx]->ptes[inidx].pfn;*/
}


/**
 * free_page(@vpn)
 *
 * DESCRIPTION
 *   Deallocate the page from the current processor. Make sure that the fields
 *   for the corresponding PTE (valid, writable, pfn) is set @false or 0.
 *   Also, consider carefully for the case when a page is shared by two processes,
 *   and one process is to free the page.
 */
int page_empty(int outidx){         //page가 empty하다는 것 체크하기 위한 함수(비어 있다면 1, 하나라도 비어있지 않다면 0)
	for(int i=0;i<NR_PTES_PER_PAGE;i++){
		if(current->pagetable.outer_ptes[outidx]->ptes[i].valid){       //현재 process에서 해당...주의 필요
			return 0;
		}
	}
	return 1;
}
void free_page(unsigned int vpn)
{

	int outidx = vpn / 16;  //outer index
	int inidx = vpn % 16;	//inner index
	int pfnnum = 0;
	//proc[current->pid] =
	pfnnum = proc[current->pid].pagetable.outer_ptes[outidx]->ptes[inidx].pfn;  //pfn값을 0으로 비워줘야 하므로 다른 곳에 저장
	proc[current->pid].pagetable.outer_ptes[outidx]->ptes[inidx].pfn = 0;       //모두 초기화
	proc[current->pid].pagetable.outer_ptes[outidx]->ptes[inidx].valid = 0;
	proc[current->pid].pagetable.outer_ptes[outidx]->ptes[inidx].writable = 0;

	if(page_empty(outidx)){     //만약 지우고 나서 빈 pte라면 그 outer ptes들을 모두 지워줘야 하므로
		//printf("123\n");
		mallocarr[outidx] = 0;		//다시 사용하게 될 경우에는 다시 할당을 해줘야 하므로
		free(proc[current->pid].pagetable.outer_ptes[outidx]);      //원래 할당 했던 것들 free
		proc[current->pid].pagetable.outer_ptes[outidx]=NULL;       
		//free(ptbr->outer_ptes[outidx]);
	}
	current = &proc[current->pid];      //current 갱신
	
	ptbr = &proc[current->pid].pagetable;       //ptbr도 갱신
	mapcounts[pfnnum]--;        //mapcount도 원래 있던 pfn값 하나 제외
	
	
}


/**
 * handle_page_fault()
 *
 * DESCRIPTION
 *   Handle the page fault for accessing @vpn for @rw. This function is called
 *   by the framework when the __translate() for @vpn fails. This implies;
 *   0. page directory is invalid
 *   1. pte is invalid
 *   2. pte is not writable but @rw is for write
 *   This function should identify the situation, and do the copy-on-write if
 *   necessary.
 *
 * RETURN
 *   @true on successful fault handling
 *   @false otherwise
 */
bool handle_page_fault(unsigned int vpn, unsigned int rw)
{
	
	int outidx = vpn / 16;  //outer index
	int inidx = vpn % 16;	//inner index
	int avpfn = 0;
	
	if(!proc[current->pid].pagetable.outer_ptes[outidx]){       //page directory가 invalid한 경우
		//printf("page directory is invalid!\n");
		return false;
	}
	if(!proc[current->pid].pagetable.outer_ptes[outidx]->ptes[inidx].valid){        //pte가 valid하지 않은 경우
		//printf("pte's valid bit is 0!\n");
		return false;
	}
	if(proc[current->pid].pagetable.outer_ptes[outidx]->ptes[inidx].writable == 0 && rw >= 2){      //권한에는 w가 없었는데 write를 요청한 경우
		if(cow){            //근데 copy on write인 상황일 때
			//printf("current's ptbr private value is : %d\n",current->pagetable.outer_ptes[outidx]->ptes[inidx].private);	
			if(proc[current->pid].pagetable.outer_ptes[outidx]->ptes[inidx].private){		//original writable = 1
				proc[current->pid].pagetable.outer_ptes[outidx]->ptes[inidx].writable = 1;      //원래 writable의 값이 1이였기 때문에 1로 바꿔주고
		
				if(mapcounts[proc[current->pid].pagetable.outer_ptes[outidx]->ptes[inidx].pfn] >= 2){       //mapcount의 값이 2이상 이였다면
					//printf("check!\n");
					mapcounts[proc[current->pid].pagetable.outer_ptes[outidx]->ptes[inidx].pfn]--;      //하나 빼주고 다른데 다시 할당
					avpfn = available_pfn(mapcounts);       //가능한 pfn확인
					if(avpfn == -1){        //가능한 pfn이 없으면 오류 발생 -> return false...
						//printf("page is full\n");
						return false;
					}
					proc[current->pid].pagetable.outer_ptes[outidx]->ptes[inidx].pfn = avpfn;       //가능한 pfn으로 재 할당
					
					mapcounts[proc[current->pid].pagetable.outer_ptes[outidx]->ptes[inidx].pfn]++;      //재 할당한 pfn에 해당하는 mapcounts 늘려주고
					
					current = &proc[current->pid];      //current와 ptbr 갱신
					ptbr = &current->pagetable;
					return true;
				}
				else if(mapcounts[proc[current->pid].pagetable.outer_ptes[outidx]->ptes[inidx].pfn] == 1){
					//printf("check2\n");
					return true;
				}
			}
		}
		//printf("checkpoint for not writable but rw is for write!\n");
		return false;
	}

	return true;
	
}


/**
 * switch_process()
 *
 * DESCRIPTION
 *   If there is a process with @pid in @processes, switch to the process.
 *   The @current process at the moment should be put into the @processes
 *   list, and @current should be replaced to the requested process.
 *   Make sure that the next process is unlinked from the @processes, and
 *   @ptbr is set properly.
 *
 *   If there is no process with @pid in the @processes list, fork a process
 *   from the @current. This implies the forked child process should have
 *   the identical page table entry 'values' to its parent's (i.e., @current)
 *   page table. 
 *   To implement the copy-on-write feature, you should manipulate the writable
 *   bit in PTE and mapcounts for shared pages. You may use pte->private for 
 *   storing some useful information :-)
 */

/***************************
 * 
* to do list
* 1. save current before change process(put into ready queue)
* 2. check already forked or no forked
* 3. write bit turn off both process(child and parent)
* 4. save original write bit -> turn off write bit
* 5. access write -> if(mapcount > 1) smallest pfn allocation....
* 6. if already forked process, just change current process
* 
****************************/
void switch_process(unsigned int pid)       //to-do list에 맞도록 차근차근 구현 진행..
{
	struct process* p = NULL;

	bool init = true;
	
	if(list_empty(&processes) && init){			//when first init ...
		list_add(&current->list,&processes);
		init = false;
	}

	
	if(!forked[pid]){   //fork가 안 된것이라면 
	
		forked[pid] = true;
		
		for (int i = 0; i < NR_PTES_PER_PAGE; i++) {		//current pagetable
		                                                //현재 pagetable부터 확인
			struct pte_directory *pd = current->pagetable.outer_ptes[i];

			if (!pd) continue;		//no error
			for (int j = 0; j < NR_PTES_PER_PAGE; j++) {
				struct pte *pte = &pd->ptes[j];
				if(pte->writable){
					current->pagetable.outer_ptes[i]->ptes[j].private = true;
					pte->private = true;
					cow = true;
				}
		 		//pte->writable = false;
				current->pagetable.outer_ptes[i]->ptes[j].writable = false;
			}
		}
		
		struct process* newpro = &proc[pid];        //새로 복사할 process

		newpro->pid = pid;

		for (int i = 0; i < NR_PTES_PER_PAGE; i++) {
			
			struct pte_directory *oldpd = current->pagetable.outer_ptes[i];

			if (!oldpd) continue;		//no error
			newpro->pagetable.outer_ptes[i] = malloc(sizeof(struct pte_directory));

			for (int j = 0; j < NR_PTES_PER_PAGE; j++){             //그대로 복사 진행
				newpro->pagetable.outer_ptes[i]->ptes[j].pfn = oldpd->ptes[j].pfn;
				newpro->pagetable.outer_ptes[i]->ptes[j].valid = oldpd->ptes[j].valid;
				newpro->pagetable.outer_ptes[i]->ptes[j].writable = oldpd->ptes[j].writable;				
				newpro->pagetable.outer_ptes[i]->ptes[j].private = oldpd->ptes[j].private;
				if(oldpd->ptes[j].valid){           //복사 했기 때문에 mapcount 늘려주고
					mapcounts[oldpd->ptes[j].pfn]++;
				}
			}
		}
		
		// add new process in list

		list_add(&newpro->list,&processes);     //복사한 process는 process들의 집합으로 

		// context switch
		current = &proc[pid];       //current값 갱신

		return;
	}

	list_for_each_entry(p,&processes,list){     //fork가 이미 된것이였다면
		if(p->pid == pid){      //해당 pid 값 찾고
			current = p;        //current 값 바꾸고
			ptbr = &current->pagetable;     //ptbr도 갱신
			return ;
		}
	}


}
