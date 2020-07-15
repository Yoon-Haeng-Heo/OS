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
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "types.h"
#include "locks.h"
#include "atomic.h"
#include "list_head.h"

/*********************************************************************
 * Spinlock implementation
 *********************************************************************/
struct spinlock {
	int held;
	
};

/*********************************************************************
 * init_spinlock(@lock)
 *
 * DESCRIPTION
 *   Initialize your spinlock instance @lock
 */
void init_spinlock(struct spinlock *lock)
{
	lock->held = 0;
	return;
}

/*********************************************************************
 * acqure_spinlock(@lock)
 *
 * DESCRIPTION
 *   Acquire the spinlock instance @lock. The returning from this
 *   function implies that the calling thread grapped the lock.
 *   In other words, you should not return from this function until
 *   the calling thread gets the lock.
 */
void acquire_spinlock(struct spinlock *lock)
{
	while(compare_and_swap(&lock->held,0,1));
	return;
}

/*********************************************************************
 * release_spinlock(@lock)
 *
 * DESCRIPTION
 *   Release the spinlock instance @lock. Keep in mind that the next thread
 *   can grap the @lock instance right away when you mark @lock available;
 *   any pending thread may grap @lock right after marking @lock as free
 *   but before returning from this function.
 */
void release_spinlock(struct spinlock *lock)
{
	lock->held = 0;
	return;
}


/********************************************************************
 * Blocking mutex implementation
 ********************************************************************/

struct thread {
	pthread_t pthread;      //스레드 id
	struct list_head list;  
};


struct thread *caller;  //함수를 호출한 스레드 저장할 변수
struct thread *next;        //다음 스레드를 저장할 변수

void signal_handler(int sig){       //SIGUSR1에 해당하는 핸들러 함수
	//printf("mutex holder wake!! LOCK RELEASE!!!\n");
}/*
void signal_release_handler(int sig){
	printf("release!!!!!!!!\n");
}
struct sigaction sa = {
	.sa_handler = signal_handler,
	.sa_flags   = 0,
},old_sa;
*/
sigset_t set;
//sigset_t release_set;
//siginfo_t info;
//bool check = false;
int cnt = 0;
/*****************************************
		semaphore
******************************************/

struct semaphore{           //semaphore
	int s;
	struct spinlock spl;        //spinlock (prevent race condition)
	struct list_head waitqueue;
};

void init_sem(struct semaphore *sem,int s){             //initialize
	sem->s = s;
	init_spinlock(&sem->spl);           
	INIT_LIST_HEAD(&sem->waitqueue);

}
void wait_sem(struct semaphore *sem){       //acquire
    //전체적인 구조는 mutex와 똑같음
	acquire_spinlock(&sem->spl);
	sem->s--;
	if(sem->s < 0)
	{
		//sigprocmask(SIG_BLOCK,&set,NULL);
		caller = malloc(sizeof(struct thread)*1); 	
		INIT_LIST_HEAD(&caller->list);
		caller->pthread = pthread_self();

		//acquire_spinlock(&mutex->spl);
		list_add_tail(&caller->list,&sem->waitqueue);		
		release_spinlock(&sem->spl);			

		//sigprocmask(SIG_UNBLOCK,&set,NULL);
		//pause();
		sigwaitinfo(&set,NULL);
	}
}
void signal_sem(struct semaphore *sem){     //release
	
	sem->s++;	
	if(sem->s <= 0)
	{
		acquire_spinlock(&sem->spl);
		next = list_first_entry(&sem->waitqueue,struct thread,list);
		list_del_init(&next->list);
		
		release_spinlock(&sem->spl);
		while(1)
		{
			if(!(pthread_kill(next->pthread,SIGUSR1)))
			{
				break;
			}	
		}
		
	}
	else
	{
		release_spinlock(&sem->spl);
		return;
	}
}

struct mutex {      //mutex lock...
	struct spinlock spl;        //use spinlock to prevent race condition
	int held;		//held value(lock hold)
	struct list_head waitqueue;		//thread id queue
//	struct semaphore sem;
	
	
};
/*********************************************************************
 * init_mutex(@mutex)
 *
 * DESCRIPTION
 *   Initialize the mutex instance pointed by @mutex.
 */
void init_mutex(struct mutex *mutex)
{
	//init_sem(&mutex->sem,1);
	//caller = malloc(sizeof(struct thread)*1);
	//INIT_LIST_HEAD(&caller->list);	
	init_spinlock(&mutex->spl);
	//mutex->owner = NULL;
	mutex->held = 0;

	INIT_LIST_HEAD(&mutex->waitqueue);

	sigemptyset(&set);
	sigaddset(&set,SIGUSR1);

	//sigemptyset(&release_set);	
	//sigaddset(&release_set,SIGUSR2);


	signal(SIGUSR1,signal_handler);		//when release alarm
	
	return;
}

/*********************************************************************
 * acquire_mutex(@mutex)
 *
 * DESCRIPTION
 *   Acquire the mutex instance @mutex. Likewise acquire_spinlock(), you
 *   should not return from this function until the calling thread gets the
 *   mutex instance. But the calling thread should be put into sleep when
 *   the mutex is acquired by other threads.
 *
 * HINT
 *   1. Use sigwaitinfo(), sigemptyset(), sigaddset(), sigprocmask() to
 *      put threads into sleep until the mutex holder wakes up
 *   2. Use pthread_self() to get the pthread_t instance of the calling thread.
 *   3. Manage the threads that are waiting for the mutex instance using
 *      a custom data structure containing the pthread_t and list_head.
 *      However, you may need to use a spinlock to prevent the race condition
 *      on the waiter list (i.e., multiple waiters are being inserted into the 
 *      waiting list simultaneously, one waiters are going into the waiter list
 *      and the mutex holder tries to remove a waiter from the list, etc..)
 */
void acquire_mutex(struct mutex *mutex)     //acquire에서는 critical section 중요
{
	

	acquire_spinlock(&mutex->spl);      //spinlock으로 막아줌
	if(mutex->held == 1)       //lock의 주인이 있을 경우
	{
		//sigprocmask(SIG_BLOCK,&set,NULL);
		caller = malloc(sizeof(struct thread)*1); 	    //caller
		INIT_LIST_HEAD(&caller->list);      //caller 초기화
		caller->pthread = pthread_self();       //caller의 thread id 저장

		//acquire_spinlock(&mutex->spl);
		list_add_tail(&caller->list,&mutex->waitqueue);		    //waitqueue (FCFS이므로 add tail)
		release_spinlock(&mutex->spl);			//release 

		//sigprocmask(SIG_UNBLOCK,&set,NULL);
		//pause();
		sigwaitinfo(&set,NULL);         //SIGUSR1들어오면 풀림 (sleep until mutex holder wake up)
	}
	else
	{
		mutex->held = 1;	        //held = 1
		release_spinlock(&mutex->spl);          //release
	}
	return;	
	
/**********************************
use semaphore
***********************************/
	//wait_sem(&mutex->sem);
	//return ;
	/*
	//signal(SIGUSR2,signal_release_handler);
	caller->pthread = pthread_self();
	//sigprocmask(SIG_BLOCK,&release_set,NULL);
	//printf("mutex owner's thread id : %ld\n",mutex->owner->pthread);

	//printf("caller thread id (acquire) : %ld\n",caller->pthread);
	
	
	//sigprocmask(SIG_BLOCK,&set,NULL);
	//list_add_tail(&caller->list,&mutex->waitqueue);
	//while(compare_and_swap(&mutex->held,0,1));	//escape this line -> mutex holder wakes up!!!!!
	
	//sigprocmask(SIG_UNBLOCK,&set,NULL);
	
	if(mutex->owner== NULL){		//progress
		while(compare_and_swap(&mutex->held,0,1));
		mutex->owner = caller;
		//list_first_entry(&mutex->waitqueue,struct thread,list);
		//printf("mutex owner's thread id : %ld\n",mutex->owner->pthread);
		return ;
		
	}
	
	//printf("mutex owner's thread id : %ld\n",mutex->owner->pthread);
	if(!list_empty(&mutex->waitqueue)){
		struct thread * cursor = NULL;
		struct thread * save = NULL;
		list_for_each_entry_safe(cursor,save,&mutex->waitqueue,list){
			//printf("wait thread id (waitqueue) : %ld\n",cursor->pthread);
			cnt++;
			if(cursor->pthread == caller->pthread || cursor->pthread == mutex->owner->pthread){
				check = true;
				//printf("wait thread id (waitqueue) : %ld\n",cursor->pthread);
				return;
			}
		}
	}
	
	if(!check){
		//pthread_kill(mutex->owner->pthread,SIGUSR2);
		
		list_add_tail(&caller->list,&mutex->waitqueue);
		
	}
	do{
		while(sigwaitinfo(&set,NULL) != SIGUSR1);		
		while(compare_and_swap(&mutex->held,0,1));
		mutex->held=1;
		return ;
			
	}while(1);
	
	/*sigprocmask(SIG_BLOCK,&set,NULL);
	//list_add_tail(&caller->list,&mutex->waitqueue);
	
	sigprocmask(SIG_UNBLOCK,&set,NULL);
	//list_add_tail(&caller->list,&mutex->waitqueue);
	
	int sigw = sigwaitinfo(&set,NULL);			//wait for unlock...
	//printf("sigwait result! : %d\n",sigw);
	//printf("release thread id : %ld\n",caller->pthread);
	//printf("mutex owner's thread id : %ld\n",mutex->owner->pthread);
	mutex->held=1;*/
	
	
	//list_add_tail(&caller->list,&mutex->waitqueue);
	
	//while(compare_and_swap(&mutex->held,0,1));
	
	
	/*sigprocmask(SIG_BLOCK,&set,NULL);	
	
	while(compare_and_swap(&mutex->held,0,1));	//escape this line -> mutex holder wakes up!!!!!
	
	printf("pthread_id after spinlock : %ld\n",pthread_self());
	sigprocmask(SIG_UNBLOCK,&set,NULL);
*/
	/*if(sigwaitinfo(&set,NULL) == -1)		//when alarm on -> 
	{
		printf("Error !!!\n");
	}*/

/*		
	if(list_empty(&mutex->waitqueue) && mutex->owner == NULL){
		mutex->owner = caller;
		mutex->held = 1;
		return ;
	}
	
	if(caller != mutex->owner){		//caller is different mutex's owner... race condition!!! 
		list_add_tail(&caller->list,&mutex->waitqueue);
		sigemptyset(&set);
		sigaddset(&set,SIGUSR1);	
		signal(SIGUSR1,signal_handler);		//when release alarm
		sigprocmask(SIG_BLOCK,&set,NULL);
		if(sigwaitinfo(&set,NULL) == -1)		//when alarm on -> 
		{
			fprintf(stderr,"Error !!!\n");
		}
		//critical section start
		while(compare_and_swap(&mutex->held,0,1));	//escape this line -> mutex holder wakes up!!!!!
		struct thread *next = list_first_entry(&mutex->waitqueue,struct thread,list);
		mutex->owner = next;
		//critical section end
			
	}*/

	/*
	//printf("%ld\n",caller->pthread);
	sigemptyset(&set);
	sigaddset(&set,SIGALRM);	
	signal(SIGALRM,signal_handler);		//when release alarm
	//sigaction(SIGALRM,&sa,&old_sa);
	//sigwaitinfo(&set,&info);

	sigprocmask(SIG_BLOCK,&set,NULL); 	//to use sleep until the mutex holder wakes up...
	
	//critical section
	list_add_tail(&caller->list,&mutex->waitqueue);
	//critical section end
	while(compare_and_swap(&mutex->held,0,1));	//escape this line -> mutex holder wakes up!!!!!
	sigprocmask(SIG_UNBLOCK,&set,NULL);
	//mutex->getthread.pthread = pthread_self();
	struct thread *next = list_first_entry(&mutex->waitqueue,struct thread,list);
	mutex->owner = next;
	*/
	//sigprocmask(SIG_UNBLOCK,&release_set,NULL);
	
}


/*********************************************************************
 * release_mutex(@mutex)
 *
 * DESCRIPTION
 *   Release the mutex held by the calling thread.
 *
 * HINT
 *   1. Use pthread_kill() to wake up a waiter thread
 *   2. Be careful to prevent race conditions while accessing the waiter list
 */

void release_mutex(struct mutex *mutex)         //release
{
	
	
	if(!list_empty(&mutex->waitqueue))      //waitqueue가 비어있지 않다면
	{
		acquire_spinlock(&mutex->spl);      //spinlock으로 막아줌
		next = list_first_entry(&mutex->waitqueue,struct thread,list);      //first come first serve이므로 first entry로 뽑아줌
		list_del_init(&next->list);
		
		release_spinlock(&mutex->spl);          //critical section 최소화
		   //이미 next는 저장되어 있기 때문에
		while(1)
		{
			if(!(pthread_kill(next->pthread,SIGUSR1)))      //pthread_kill은 성공하면 0이 반환되기 때문
			{
				break;
			}	
		}
		
	}
	else
	{
		
		mutex->held = 0;
		release_spinlock(&mutex->spl);
		
	}

/******************************
	use semaphore
********************************/
	//signal_sem(&mutex->sem);
	
	/*if(pthread_self() != mutex->owner->pthread){
		fprintf(stderr,"error!\n");
		return ;
	}
	//printf("release call thread!!!!!!!! :%ld\n",pthread_self());
	//printf("checkpoint1!\n");
	//int sigwaitnum = sigwaitinfo(&release_set,NULL);

	if(list_empty(&mutex->waitqueue)){
		//printf("waitqueue is empty!!\n");
		list_del_init(&mutex->owner->list);
		mutex->owner = NULL;
		mutex->held = 0;
		return ;
	}
	mutex->held=0;
	//struct thread * cursor = NULL;
	//struct thread * save = NULL;
	//list_for_each_entry_safe(cursor,save,&mutex->waitqueue,list){
			//printf("wait thread id (waitqueue) : %ld\n",cursor->pthread);
	//	pthread_kill(cursor->pthread,SIGUSR1);
	//}
	next = list_first_entry(&mutex->waitqueue,struct thread,list);
	
	list_del_init(&next->list);
		//printf("checkpoint2!\n");
	
		//printf("checkpoint3!\n");
	
		//printf("NEXT pthread_id : %ld\n",next->pthread);
		
		//list_del_init(&mutex->owner->list);
	mutex->owner = next;
	
	//printf("checkpoint4!\n");
	pthread_kill(mutex->owner->pthread,SIGUSR1);*/
	return;
}
/*********************************************************************
 * Ring buffer
 *********************************************************************/
struct ringbuffer {
	/** NEVER CHANGE @nr_slots AND @slots ****/
	/**/ int nr_slots;                     /**/
	/**/ int *slots;                       /**/
	/*****************************************/
	
};
//nr_slots is size of ring buffer
//slots is location of ringbuffer

struct ringbuffer ringbuffer = {
	

	
};
static int in = 0;          
static int out = 0;
struct spinlock spl;        //ring buffer를 어떤 락으로 돌지 spinlock
struct mutex mtl;           //mutex lock
enum lock_types types;      //둘 중에 어떤 락인지
int count = 0;	        //usable slot number
/*
void init_spinlock_ringbuffer(int nr_slots){
	init_spinlock(&spl);
	splring.nr_slots = nr_slots;
	splring.slots = (int *)malloc(sizeof*/

/*********************************************************************
 * enqueue_into_ringbuffer(@value)
 *
 * DESCRIPTION
 *   Generator in the framework tries to put @value into the buffer.
 */
void enqueue_into_ringbuffer(int value)     //강의노트 그대로지만 약간의 수정
{
	
loop:
	if(types == lock_spinlock){
		acquire_spinlock(&spl);
	
		int check = 0;
	
	//printf("check! : %d\n",check);
		check = (in +1) % ringbuffer.nr_slots;
		
	//printf("check! : %d\n",check);
		if(check == out){
			release_spinlock(&spl);
			goto loop;
		}
		else{
			/*while(1){
				release_spinlock(&spl);
				acquire_spinlock(&spl);
				check
		
		ringbuffer*/
			//이 부분은 강노와 다름
			in = (in +1) % (ringbuffer.nr_slots);       //이 부분이 먼저 옴. circular queue 구현 방법 차이 
			ringbuffer.slots[in] = value;      //값 넣어줌
			release_spinlock(&spl);

			return ;
		}
	}
	else if(types == lock_mutex){           //mutex도 lock 차이 다른것은 똑같음 하지만 count 값 도입해서 진행해봄
		acquire_mutex(&mtl);
	
		int check2 = 0;
	
	//printf("check! : %d\n",check);
		check2 = (in +1) % ringbuffer.nr_slots;
		
	//printf("check! : %d\n",check);
		//if(check2 == in){
		if(count == ringbuffer.nr_slots){
			release_mutex(&mtl);
			goto loop;
		}
		
			/*while(1){
				release_spinlock(&spl);
				acquire_spinlock(&spl);
				check
		
		ringbuffer*/
		
		in = (in +1) % (ringbuffer.nr_slots);
		ringbuffer.slots[in] = value;
		count++;
		release_mutex(&mtl);

		return ;
		
	}
}


/*********************************************************************
 * dequeue_from_ringbuffer(@value)
 *
 * DESCRIPTION
 *   Counter in the framework wants to get a value from the buffer.
 *
 * RETURN
 *   Return one value from the buffer.
 */
int dequeue_from_ringbuffer(void)       //dequeue
{

loop2:
	if(types == lock_spinlock){
		acquire_spinlock(&spl);
		if(in == out){
			release_spinlock(&spl);
			goto loop2;
		}
		else{
			out = (out+1) % (ringbuffer.nr_slots);      //이 부분이 다름 원래는 다음 item을 먼저 뽑지만 circular queue 자료구조 활용 정도 차이
			//out을 다음 인덱스가 아닌 여기까지 차있다고 알려주는 index
			release_spinlock(&spl);
			return ringbuffer.slots[out];
		}
	}
	else if(types == lock_mutex){       //spinlock과 같지만 count값 도입 및 lock이 mutex
		acquire_mutex(&mtl);
		if(count ==0){
			release_mutex(&mtl);
			goto loop2;
		}
		out = (out+1) % (ringbuffer.nr_slots);
		count--;
		release_mutex(&mtl);
		return ringbuffer.slots[out];
		
	}
}


/*********************************************************************
 * fini_ringbuffer
 *
 * DESCRIPTION
 *   Clean up your ring buffer.
 */
void fini_ringbuffer(void)
{	if(types == lock_spinlock){
		acquire_spinlock(&spl);
		free(ringbuffer.slots);
		release_spinlock(&spl);
		return ;
	}
	else if(types == lock_mutex){
		acquire_mutex(&mtl);
		free(ringbuffer.slots);
		release_mutex(&mtl);
		return ;
	}
}

/*********************************************************************
 * init_ringbuffer(@nr_slots)
 *
 * DESCRIPTION
 *   Initialize the ring buffer which has @nr_slots slots.
 *
 * RETURN
 *   0 on success.
 *   Other values otherwise.
 */
int init_ringbuffer(const int nr_slots)
{
	count = nr_slots;       //count값 도입!
	/** DO NOT MODIFY THOSE TWO LINES **************************/
	/**/ ringbuffer.nr_slots = nr_slots;                     /**/
	/**/ ringbuffer.slots = malloc(sizeof(int) * nr_slots);  /**/
	/***********************************************************/
	
	return 0;
}
