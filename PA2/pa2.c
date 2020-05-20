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

/* THIS FILE IS ALL YOURS; DO WHATEVER YOU WANT TO DO IN THIS FILE */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "types.h"
#include "list_head.h"

/**
 * The process which is currently running
 */
#include "process.h"
extern struct process *current;

/**
 * List head to hold the processes ready to run
 */
extern struct list_head readyqueue;


/**
 * Resources in the system.
 */
#include "resource.h"
extern struct resource resources[NR_RESOURCES];


/**
 * Monotonically increasing ticks
 */
extern unsigned int ticks;


/**
 * Quiet mode. True if the program was started with -q option
 */
extern bool quiet;

bool pcp,pip,changed = false;
/***********************************************************************
 * Default FCFS resource acquision function
 *
 * DESCRIPTION
 *   This is the default resource acquision function which is called back
 *   whenever the current process is to acquire resource @resource_id.
 *   The current implementation serves the resource in the requesting order
 *   without considering the priority. See the comments in sched.h
 ***********************************************************************/
bool fcfs_acquire(int resource_id)
{
	struct resource *r = resources + resource_id;

	if (!r->owner) {
		/* This resource is not owned by any one. Take it! */
		r->owner = current;
		return true;
	}

	/* OK, this resource is taken by @r->owner. */

	/* Update the current process state */
	current->status = PROCESS_WAIT;

	/* And append current to waitqueue */
	list_add_tail(&current->list, &r->waitqueue);

	/**
	 * And return false to indicate the resource is not available.
	 * The scheduler framework will soon call schedule() function to
	 * schedule out current and to pick the next process to run.
	 */
	return false;
}

/***********************************************************************
 * Default FCFS resource release function
 *
 * DESCRIPTION
 *   This is the default resource release function which is called back
 *   whenever the current process is to release resource @resource_id.
 *   The current implementation serves the resource in the requesting order
 *   without considering the priority. See the comments in sched.h
 ***********************************************************************/
void fcfs_release(int resource_id)
{
	struct resource *r = resources + resource_id;

	/* Ensure that the owner process is releasing the resource */
	assert(r->owner == current);

	/* Un-own this resource */
	r->owner = NULL;

	/* Let's wake up ONE waiter (if exists) that came first */
	if (!list_empty(&r->waitqueue)) {
		struct process *waiter =
				list_first_entry(&r->waitqueue, struct process, list);

		/**
		 * Ensure the waiter  is in the wait status
		 */
		assert(waiter->status == PROCESS_WAIT);

		/**
		 * Take out the waiter from the waiting queue. Note we use
		 * list_del_init() over list_del() to maintain the list head tidy
		 * (otherwise, the framework will complain on the list head
		 * when the process exits).
		 */
		list_del_init(&waiter->list);

		/* Update the process status */
		waiter->status = PROCESS_READY;

		/**
		 * Put the waiter process into ready queue. The framework will
		 * do the rest.
		 */
		list_add_tail(&waiter->list, &readyqueue);
	}
}



#include "sched.h"

/***********************************************************************
 * FIFO scheduler
 ***********************************************************************/
static int fifo_initialize(void)
{
	return 0;
}

static void fifo_finalize(void)
{
}

static struct process *fifo_schedule(void)
{
	struct process *next = NULL;

	/* You may inspect the situation by calling dump_status() at any time */
	//dump_status();

	/**
	 * When there was no process to run in the previous tick (so does
	 * in the very beginning of the simulation), there will be
	 * no @current process. In this case, pick the next without examining
	 * the current process. Also, when the current process is blocked
	 * while acquiring a resource, @current is (supposed to be) attached
	 * to the waitqueue of the corresponding resource. In this case just
	 * pick the next as well.
	 */
	if (!current || current->status == PROCESS_WAIT) {
		goto pick_next;
	}

	/* The current process has remaining lifetime. Schedule it again */
	if (current->age < current->lifespan) {
		return current;
	}

pick_next:
	/* Let's pick a new process to run next */

	if (!list_empty(&readyqueue)) {
		/**
		 * If the ready queue is not empty, pick the first process
		 * in the ready queue
		 */
		next = list_first_entry(&readyqueue, struct process, list);

		/**
		 * Detach the process from the ready queue. Note we use list_del_init()
		 * instead of list_del() to maintain the list head tidy. Otherwise,
		 * the framework will complain (assert) on process exit.
		 */
		list_del_init(&next->list);
	}

	/* Return the next process to run */
	return next;
}

struct scheduler fifo_scheduler = {
	.name = "FIFO",
	.acquire = fcfs_acquire,
	.release = fcfs_release,
	.initialize = fifo_initialize,
	.finalize = fifo_finalize,
	.schedule = fifo_schedule,
};


/***********************************************************************
 * SJF scheduler
 ***********************************************************************/
static struct process *sjf_schedule(void)
{
	/**
	 * Implement your own SJF scheduler here.
	 */
	int min = 99999;            //SJF에서 가장 lifespan이 작은 값을 골라내야 하므로 그 값을 비교하기 위한 min값 적절하게 큰 값으로 설정
	struct process *next = NULL;
	struct process *save = NULL;	//list_for_each_entry_safe n value (올려주신 파일 참조 parameter n에 해당하는 값 list_head를 포함한 process로 선언)
	struct process *cursor = NULL;		//list_for_each_entry_safe pos value(parameter pos에 해당하는 값)
	struct process *res = NULL;         //결과 값 저장

	if (!current || current->status == PROCESS_WAIT) {
		goto pick_next;
	}

	/* The current process has remaining lifetime. Schedule it again */
	if (current->age < current->lifespan) {
		return current;
	}

pick_next:
	/* Let's pick a new process to run next */

	if (!list_empty(&readyqueue)) {
		list_for_each_entry_safe(cursor,save,&readyqueue,list){		//in kernel similar "for" but include prefetch (safe) (list_for_each_safe X)

		//list_for_each_entry_safe는 커널에서의 for문과 비슷하다 하지만 리스트에서 내용이 삭제가 될 경우 prefetch가 되므로 safe하다. 
			if(cursor->lifespan < min){        //가르키는 cursor의 lifespan이 min보다 작으면 min값 갱신
				min = cursor->lifespan;
				res = cursor;           //cursor가 제일 작은 값을 가르키므로 SJF에서는 먼저 해야 할 작업으로 선정
			}
		}
		next = res;         //NEXT값 갱신

		
		list_del_init(&next->list);     //리스트에서 삭제

		
	}
	return next;
}

struct scheduler sjf_scheduler = {
	.name = "Shortest-Job First",
	.acquire = fcfs_acquire, /* Use the default FCFS acquire() */
	.release = fcfs_release, /* Use the default FCFS release() */
	.schedule = sjf_schedule,		 /* TODO: Assign sjf_schedule()
								to this function pointer to activate
								SJF in the system */
};


/***********************************************************************
 * SRTF scheduler
 ***********************************************************************/
static struct process *srtf_schedule(void){
	int min = 999999;
	struct process *next = NULL;
	struct process *save = NULL;	//list_each_entry_for_safe n value
	struct process *cursor = NULL;		//list_each_entry_for_safe pos value
	struct process *res = NULL;
	//dump_status();
	if (!current || current->status == PROCESS_WAIT) {
		goto pick_next;
	}

	if (current->age < current->lifespan) {		//this will be changed...
		list_move(&current->list,&readyqueue);      //current를 계속 봐가면서 레디큐에 들어오는 값이 더 작은지 체크 해야 하므로 list를 레디큐에 합쳐준다.
		//dump_status();
		goto pick_next;     //다음 진행
	}

pick_next:
	if (!list_empty(&readyqueue)) {
		list_for_each_entry_safe(cursor,save,&readyqueue,list){		//in kernel similar "for" but include prefetch (safe) (list_for_each_safe X)
			if(cursor->lifespan < min){
				min = cursor->lifespan;
				res = cursor;
			}
		}
		next = res;

		
		list_del_init(&next->list);

	}
	return next;

}
struct scheduler srtf_scheduler = {
	.name = "Shortest Remaining Time First",
	.acquire = fcfs_acquire, /* Use the default FCFS acquire() */
	.release = fcfs_release, /* Use the default FCFS release() */
	.schedule = srtf_schedule,
	/* You need to check the newly created processes to implement SRTF.
	 * Use @forked() callback to mark newly created processes */
	/* Obviously, you should implement srtf_schedule() and attach it here */
};


/***********************************************************************
 * Round-robin scheduler
 ***********************************************************************/
static struct process *rr_schedule(void){
	struct process *next = NULL;

	//dump_status();

	if (!current || current->status == PROCESS_WAIT) {
		goto pick_next;
	}

	if (current->age < current->lifespan) {
		list_move_tail(&current->list,&readyqueue);         //srrf에서는 head로 옮겼지만 rr은 한틱씩 진행하고 readyqueue의 맨 뒤로 보낸다. (순서가 있기 때문에)
		goto pick_next;     //다음으로 진행
	}

pick_next:

	if (!list_empty(&readyqueue)) {

		
		next = list_first_entry(&readyqueue, struct process, list);     //fifo처럼 하나씩 리스트에서 뽑는다. 순서는 위의 list_move_tail로 인해서 신경 쓸 필요 없음
		list_del_init(&next->list);
	}

	return next;
}
struct scheduler rr_scheduler = {
	.name = "Round-Robin",
	.acquire = fcfs_acquire, /* Use the default FCFS acquire() */
	.release = fcfs_release, /* Use the default FCFS release() */
	.schedule = rr_schedule,
	/* Obviously, you should implement rr_schedule() and attach it here */
};

bool prio_acquire(int resource_id){
	
	struct resource *r = resources + resource_id;
	unsigned int max_val = 1000;
	if (!r->owner) {
		/* This resource is not owned by any one. Take it! */
		r->owner = current;
		r->owner->prio_orig = r->owner->prio;		//to save original priority ( Can change priority ) 
		//우선순위를 갖는 모델들에서는 리소스를 acquire할 때 priority가 바뀔수 있기 때문에 원래 우선순위를 저장을 해놓아야 한다.
		if(pcp){                    //pcp일 경우 리소스를 잡자마자 그 priority를 max값으로 올려준다.
			r->owner->prio = max_val;		//when resource owner have resource, priority value set max value!
			changed = true;         //나중에 release에서 값이 바뀌었을 때 원래대로 복구를 시켜야 하므로 그 때 사용할 변수 changed
		}
		return true;
	}

	
	if(pip){
		if(current -> prio > r->owner->prio){		//when current process's prioity bigger than resource owner
			r->owner->prio = current->prio; 	// prioity changed!
			changed = true;
		}
	
	}
	/* Update the current process state */
	current->status = PROCESS_WAIT;
	

	list_add_tail(&current->list, &r->waitqueue);
	/**
	 * And return false to indicate the resource is not available.
	 * The scheduler framework will soon call schedule() function to
	 * schedule out current and to pick the next process to run.
	 */
	return false;
}
void prio_release(int resource_id)
{
	struct resource *r = resources + resource_id;

	/* Ensure that the owner process is releasing the resource 
	assert(r->owner == current);*/
	
	if(changed){                //값이 바뀐 경우
		r->owner->prio = r->owner->prio_orig;       //원래 값으로 복구
	}
	/* Un-own this resource */

	changed = false;            //값을 바꿔주고 다시 false로 바꿔줌
	r->owner = NULL;        

	if (!list_empty(&r->waitqueue)) {       //waitqueue가 하나라도 있으면
			
		struct process *waiter = NULL;          //fifo release와 같은 변수 사용 (waitqueue에 있는 것을 readyqueue에 넣어주는 과정을 위함)
		struct process *n = NULL;           //list_for_each_entry_safe에서의 n 변수
		list_for_each_entry_safe(waiter,n,&r->waitqueue,list){      //waitqueue의 값들 순회
			list_del_init(&waiter->list);               //waiter는 waitqueue에서 하나씩 값을 가져온다 그 값을 삭제해주고(꺼내준다는 표현이 더 맞을 듯)
			waiter->status = PROCESS_READY;     //꺼낸 프로세스 ready로 바꾸고
			list_add(&waiter->list,&readyqueue);        //readyqueue로 넣어줌 끝이 아닌 앞으로 넣어줌(차곡차곡 쌓이는 느낌)
			list_del_init(&current->list);          //release의 원래 목적인 current를 release해준다.
		}

	}
}

/***********************************************************************
 * Priority scheduler
 ***********************************************************************/
static struct process *prio_schedule(void){
	
	struct process *next = NULL;
	struct process *save = NULL;	//list_each_entry_for_safe n value
	struct process *cursor = NULL;		//list_each_entry_for_safe pos value
	struct process *res = NULL;
	//dump_status();
	int max = -1;		//prio >=0
	pcp = false;
	pip =false;

	if (!current || current->status == PROCESS_WAIT) {
		goto pick_next;
	}

	/* The current process has remaining lifetime. Schedule it again */
	if (current->age < current->lifespan) {
		list_move_tail(&current->list,&readyqueue);         //prio has sequence, so move to tail(prio는 순서가 있기 때문에 tail로 보내줌)
		goto pick_next;
	}

pick_next:
	/* Let's pick a new process to run next */

	if (!list_empty(&readyqueue)) {
		
		list_for_each_entry_safe(cursor,save,&readyqueue,list){		//in kernel similar "for" but include prefetch (safe) (list_for_each_safe X)
			if((int)cursor->prio > max){        //max는 int형, prio는 unsigned int형 형 변환 해줌
				max = (int)cursor->prio;            //max값 갱신
				res = cursor;
			}
		}
		next = res;

		list_del_init(&next->list);

	}

	/* Return the next process to run */
	return next;
}
struct scheduler prio_scheduler = {
	.name = "Priority",
	.acquire = prio_acquire,
	.release = prio_release,
	.schedule = prio_schedule,
	/**
	 * Implement your own acqure/release function to make priority
	 * scheduler correct.
	 */
	/* Implement your own prio_schedule() and attach it here */
};


static struct process *pcp_schedule(void){
	
	struct process *next = NULL;
	struct process *save = NULL;	//list_each_entry_for_safe n value
	struct process *cursor = NULL;		//list_each_entry_for_safe pos value
	struct process *res = NULL;
	//dump_status();
	int max = -1;		//prio >=0
	
	pcp = true;         //전체적인 함수 개요는 prio랑 똑같지만 acquire에서 pcp가 true로 걸리기 때문에 알아서 해결
	//이 함수에서는 단지 우선순위에 따른 스케줄링만 진행
	pip = false;

	if (!current || current->status == PROCESS_WAIT) {
		goto pick_next;
	}

	/* The current process has remaining lifetime. Schedule it again */
	if (current->age < current->lifespan) {
		list_move_tail(&current->list,&readyqueue);         //prio has sequence, so move to tail
		goto pick_next;
	}

pick_next:
	/* Let's pick a new process to run next */

	if (!list_empty(&readyqueue)) {
		list_for_each_entry_safe(cursor,save,&readyqueue,list){		//in kernel similar "for" but include prefetch (safe) (list_for_each_safe X)
			if((int)(cursor->prio) > max){
				max = (int)(cursor->prio);
				res = cursor;
			}
		}
		next = res;

		
		list_del_init(&next->list);

	}

	/* Return the next process to run */
	return next;
}

/***********************************************************************
 * Priority scheduler with priority ceiling protocol
 ***********************************************************************/
struct scheduler pcp_scheduler = {
	.name = "Priority + Priority Ceiling Protocol",
	.acquire = prio_acquire,
	.release = prio_release,
	.schedule = pcp_schedule,
	/**
	 * Implement your own acqure/release function too to make priority
	 * scheduler correct.
	 */
};


/***********************************************************************
 * Priority scheduler with priority inheritance protocol
 ***********************************************************************/
static struct process *pip_schedule(void){
	
	struct process *next = NULL;
	struct process *save = NULL;	//list_each_entry_for_safe n value
	struct process *cursor = NULL;		//list_each_entry_for_safe pos value
	struct process *res = NULL;
	//dump_status();
	int max = -1;		//prio >=0
	
	pip = true;         //pcp와 마찬가지로 acquire에서 priority inversion문제를 해결 하였기 때문에 이 함수에서는 우선순위에 따른 스케줄링만 해결
	pcp = false;
	if (!current || current->status == PROCESS_WAIT) {
		goto pick_next;
	}

	/* The current process has remaining lifetime. Schedule it again */
	if (current->age < current->lifespan) {
		list_move_tail(&current->list,&readyqueue);         //prio has sequence, so move to tail
		goto pick_next;
	}

pick_next:
	/* Let's pick a new process to run next */

	if (!list_empty(&readyqueue)) {
		
		list_for_each_entry_safe(cursor,save,&readyqueue,list){		//in kernel similar "for" but include prefetch (safe) (list_for_each_safe X)
			if((int)cursor->prio > max){
				max = (int)cursor->prio;
				res = cursor;
			}
		}
		next = res;

		
		list_del_init(&next->list);

	}

	/* Return the next process to run */
	return next;
}
struct scheduler pip_scheduler = {
	.name = "Priority + Priority Inheritance Protocol",
	.acquire = prio_acquire,
	.release = prio_release,
	.schedule = pip_schedule,
	/**
	 * Ditto
	 */
};
