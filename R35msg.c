#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <math.h>

#include <fcntl.h>


#include "R35Aclient.h"


const char C_FNAME_HEADER[] = "/home/boris/heap.r35set";
const char cLngPost[]	= "LngPost";
const char cLngPred[]	= "LngPred";

int mq_id;
struct msgbuf tmpMSG_TEST_FACTOR = {.type = LM_TEST_FACTOR}; 
struct msgbuf tmpMSG_UPDATE = {.type = LM_UPDATE}; 


int shmid;



//создаю стек сообщений
void init_msgq(void)
{
    if ((mq_id = msgget(ipckey, IPC_CREAT | IPC_ACCESS)) < 0)
    {
        perror("server: can't create mq_id");
        exit(1);
    }
}

int ModifyHeader(unsigned long* lpulPrevSize, unsigned long* lpulPostSize)
{
    FILE* lpFile = NULL;
	unsigned char* lpHeader = NULL;
	unsigned char* lpTMP;
	size_t stLen;
	int i;
	unsigned short sOffset, iTVPStepSize;
	unsigned long ulLexemMask;
	
	if(!lpulPrevSize || !lpulPostSize)
    {
#ifdef _DEBUG
		printf("ModifyHeader. Pointer false %p %p\n", lpulPrevSize, lpulPostSize);
#endif
		perror("ModifyHeader. Pointer false\n");
		return -1;
    }
	
    lpFile = fopen (C_FNAME_HEADER, "r+b");//!!!
    if(lpFile == NULL)
    {
#ifdef _DEBUG
		printf("ModifyHeader. Open %s file error\n", C_FNAME_HEADER);
#endif
		perror("ModifyHeader. Open heap file error\n");
		return -2;
    }
	
	fseek(lpFile, 0, SEEK_END);
	stLen = ftell(lpFile);
	
	lpHeader = (unsigned char*)malloc(stLen);
	if(lpHeader == NULL)
	{
#ifdef _DEBUG
		printf("ModifyHeader. malloc error %d\n", stLen);
#endif
		perror("ModifyHeader. malloc error %d\n");
		return -3;
	}
	
	fseek(lpFile, 0, SEEK_SET);
	fread(lpHeader, stLen, 1, lpFile);

	i = *((unsigned short*)(lpHeader + 0x16));//TVPOffset
	lpTMP = lpHeader + i;
	iTVPStepSize = (unsigned short)(*(lpTMP + 7));
	i = (int)(*(lpTMP+8));

	ulLexemMask = 0x00000006;
	while(i > 0)
	{
		if((ulLexemMask & 0x00000002) && !strncmp(lpTMP, cLngPost, 7))
		{
			ulLexemMask &= 0xfffffffd;
			sOffset = *((short*)(lpTMP + 9));
			sOffset += 	*((short*)(lpHeader + 0x14));//HeaderLong
			
			*lpulPostSize /= 10;
			*lpulPostSize *= 10;
			*((long*)(lpHeader + sOffset)) = *lpulPostSize;
		}
			
		if((ulLexemMask & 0x00000004) && !strncmp(lpTMP, cLngPred, 7))
		{
			ulLexemMask &= 0xfffffffb;
			sOffset = *((short*)(lpTMP + 9));
			sOffset += *((short*)(lpHeader + 0x14));//HeaderLong
			
			*lpulPrevSize /= 10;
			*lpulPrevSize *= 10;
			*((long*)(lpHeader + sOffset)) = *lpulPrevSize;
		}
		
		if(!ulLexemMask)
			break;
		
		lpTMP += iTVPStepSize;
		i--;
	}

#ifdef _DEBUG
		printf("ModifyHeader. %d %d\n", *lpulPrevSize, *lpulPostSize);
#endif
	
	fseek(lpFile, 0, SEEK_SET);
	//!!! Write file
	fwrite(lpHeader, stLen, 1, lpFile);
	fflush(lpFile);
    fclose(lpFile);
	
	free(lpHeader);
	
	return 0;
}

int main(int argc, char* argv[])
{
	unsigned long ulPrevSize, ulPostSize;
	int k;
	int idSemget;
	struct sembuf operations;
	
    init_msgq();
	memset(tmpMSG_UPDATE.data, 0, sizeof(tmpMSG_UPDATE.data));
	memset(tmpMSG_TEST_FACTOR.data, 0, sizeof(tmpMSG_TEST_FACTOR.data));
	
	
	switch(argc)
	{
		case 1:
			k = msgsnd(mq_id, &tmpMSG_UPDATE, sizeof(tmpMSG_UPDATE.data), /*MSG_NOERROR0*/ IPC_NOWAIT);
			if(k)
			{
				printf("Unknown error.\n", k);
				return -2;
			}
			else
				printf("UPDATE ");
			break;
		case 2:
			k = msgsnd(mq_id, &tmpMSG_TEST_FACTOR, sizeof(tmpMSG_TEST_FACTOR.data), /*MSG_NOERROR0*/ IPC_NOWAIT);
			if(k)
			{
				printf("TEST_FACTOR ", k);
				return -2;
			}
			else
				printf("UPDATE ");
			break;
		case 3:
			ulPrevSize = atoi(argv[1]);
			ulPostSize = atoi(argv[2]);
			
			if(ulPrevSize > 1000)
			{
				printf("Ошибка! Время регистрации \"До\" более  1000 mS.\n");
				return -1;
			}
			
			if(ulPostSize > 9000)
			{
				printf("Ошибка! Время регистрации \"После\" более  9000 mS.\n");
				return -1;
			}
/*
			if((ulPrevSize + ulPostSize) > 10000)
			{
				printf("Ошибка! Сумма значений параметров превышает 10 сек.\n");
				return -1;
			}
*/			
			

			//Блокирую
			idSemget = semget(KEY, 1, 0666);
			operations.sem_num = 0;
			operations.sem_op = -1;
			operations.sem_flg = 0;
			semop(idSemget, &operations, 1);
			
			k = ModifyHeader(&ulPrevSize, &ulPostSize);
			//освобождаю    
			operations.sem_num = 0;
			operations.sem_op = 1;
			operations.sem_flg = 0;
			semop(idSemget, &operations, 1);
			
			if(k)
			{
				printf("Ошибка записи. %d %d\n", ulPrevSize, ulPostSize);
				return -2;
			}
			k = msgsnd(mq_id, &tmpMSG_UPDATE, sizeof(tmpMSG_UPDATE.data), /*MSG_NOERROR0*/ IPC_NOWAIT);
			if(k)
			{
				printf("Unknown error.\n", k);
				return -2;
			}
			else
				printf("ModifyHeader ");
			break;
		default:
			break;
	}
		
	printf("OK.\n", k);
    return 0;
}

