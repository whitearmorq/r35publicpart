

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __R35_M_H
#define __R35_M_H

#include "R35common.h"

#include <linux/delay.h>
#include <linux/kthread.h>

#define IPC_CREAT 	00001000

//shm
extern int				shmid;
//extern short*	msg_p;
extern short*	lpFlags;
extern short*	lpCT;

//extern asmlinkage long (*sys_shmget)(key_t key, size_t size, int flag);
//extern asmlinkage long (*sys_shmat)(int shmid, char __user *shmaddr, int shmflg);
//extern asmlinkage long (*sys_shmdt)(char __user *shmaddr);
extern asmlinkage long (*sys_msgget)(key_t key, int msgflg);
extern asmlinkage long (*sys_msgsnd)(int msqid, struct msgbuf __user *msgp, size_t msgsz, int msgflg);


struct msgbuf mymsg[4] = {
			{.type = LM_HALF_PERIOD},
			{.type = LM_HALF_PERIOD},
			{.type = LM_HALF_PERIOD},
			{.type = LM_HALF_PERIOD}};


union unnStatus /*!<  */
{
	struct
	{
		unsigned long m_MSGCounter		: 2;///< .
		unsigned long m_FirstTime		: 1;///< .
	}bits;/*!<  */
	unsigned long m_ulFlags;/*!<  */
} unnStatus;///< 


extern struct task_struct* pThreadR35;

extern short bufferR[];
extern short bufferW[];

//msg 
extern int mq_id;
extern int iErrorCounter;


int send_mess(struct msgbuf* lpBuf);
void InitThreadR35Handler(void* data);


/* Includes ------------------------------------------------------------------*/
#endif /* __R35_M_H */

