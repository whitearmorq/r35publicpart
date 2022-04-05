#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <math.h>
#include <fcntl.h>
#include <time.h>

#include "R35Aclient.h"


int mq_id;
struct msgbuf mymsg; 
struct msgbuf prev_mymsg; 

short* msg_p = NULL;
unsigned short* lpusID = NULL;
int iR35AVariant = 0;

int received;
int shmid;

long shmCh = 0;
struct strctSaveFactor strctSF;

//создаю стек сообщений
void init_msgq(void)
{
    if ((mq_id = msgget(ipckey, IPC_CREAT | IPC_ACCESS)) < 0)
	{
        perror("server: can't create mq_id");
		bcm2835_gpio_write(PIN_LED_FAILURE, 1);
        exit(1);
    }
}

void init_shm(void)
{
    //COMMON_MEM_SIZE = get_needmemsize(28,10);
    if ((shmid = shmget(shmkey, COMMON_MEM_ALL_SIZE, IPC_CREAT | IPC_ACCESS)) < 0)
	{
		bcm2835_gpio_write(PIN_LED_FAILURE, 1);
        perror("client: can not get shared memory segment");
	}
	
    if ((msg_p = (short *) shmat(shmid, 0, 0)) == NULL)
	{
		bcm2835_gpio_write(PIN_LED_FAILURE, 1);
        perror("client: shared memory attach error");
	}
}

void Lock(void)
{
	struct sembuf operations;
	operations.sem_num = 0;
	operations.sem_op = -1;
	operations.sem_flg = 0;
	semop(idSemget, &operations, 1);
}

void Unlock(void)
{			
	struct sembuf operations;
	operations.sem_num = 0;
	operations.sem_op = 1;
	operations.sem_flg = 0;
	semop(idSemget, &operations, 1);
}

int main()
{
	unsigned short usCT = HALF_PERIOD_COUNT;

	struct msqid_ds msqds;
	
	short res = 0;
	long lCurrentDataPos = 0;
	int iSkipCounter = START_SKIP_COUNT*2;//!!!
	int i, j;
	int iFactorPos;
	short*	pCurrentData = NULL;
	short*	lpsTMPData1;
	short*	lpsTMPData2;
//QQQ	short*	lpRecorderFifoBuffer = NULL;
//QQQ	short*	lpCurrentRecorderFifoBuffer = NULL;
	short sOff1, sOff2;
	double dOff1, dOff2;
	
	const int iCtPos = ONE_MEASUREMENT_SIZE * HALF_PERIOD_COUNT - 1;
	lpusID = mymsg.data + POS_ID;
	
	strctFH.stLength = 0;
	strctFH.pcData = NULL;
	
//QQQ	int iRecorderCounter = 0;
	
	union semun {
		int val;
		struct semid_ds *buf;
		ushort * array;
	} argument;
	argument.val = 0;
	
    /* Create the semaphore with external key KEY if it doesn't already 
      exists. Give permissions to the world. */
    idSemget = semget(KEY, 1, 0666 | IPC_CREAT);

    /* Always check system returns. */
    if(idSemget < 0)
    {
#ifdef _DEBUG
		printf("Unable to obtain semaphore.\n");
#endif
		sprintf(cDBG, "Unable to obtain semaphore.\n", (int)time(NULL));
		myLog();
        exit(-100);
    }

    if(semctl(idSemget, 0, SETVAL, argument) < 0)
    {
#ifdef _DEBUG
		printf("Cannot set semaphore value.\n");
#endif
		sprintf(cDBG, "Cannot set semaphore value.\n", (int)time(NULL));
		myLog();
    }
    else
    {
#ifdef _DEBUG
		printf("Semaphore %d initialized.\n", KEY);
#endif
    }

	chdir("/home/boris/ftpfolder/data");
	if(!bcm2835_init())
	{
#ifdef _DEBUG
		printf("bcm2835_init is FALSE.\n");
#endif
		sprintf(cDBG, "%d bcm2835_init is FALSE.\n", (int)time(NULL));
		myLog();
		return 1;
	}
	
	bcm2835_gpio_fsel(PIN_BLINKER, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_write(PIN_BLINKER, 0);
	bcm2835_gpio_fsel(PIN_LED_BLINKER, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_write(PIN_LED_BLINKER, 0);
	bcm2835_gpio_fsel(PIN_LED_FAILURE, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_write(PIN_LED_FAILURE, 0);
	bcm2835_gpio_fsel(PIN_LED_SAVE_FACTOR, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_write(PIN_LED_SAVE_FACTOR, 0);
	bcm2835_gpio_fsel(PIN_LED_READY, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_write(PIN_LED_READY, 0);
	
    init_msgq();
    init_shm();
	
	pCurrentData = msg_p;
/*QQQ	
	lpRecorderFifoBuffer = calloc(I_RECORDER_FIFO_BUFFER_COUNT, sizeof(short));
	if(lpRecorderFifoBuffer == NULL)
	{
#ifdef _DEBUG
		printf("RecorderFifoBuffer is NULL.\n");
#endif
		sprintf(cDBG, "%d RecorderFifoBuffer is NULL.\n", (int)time(NULL));
		myLog();
		return 6;
	}
	lpCurrentRecorderFifoBuffer = lpRecorderFifoBuffer;
*/
	memset(saFactor, 0, sizeof(strctFactor));
	memset(pCurrentData, 0, sizeof(short)*COMMON_MEM_SIZE);
//QQQ	memset(dRMS_UI, 0, sizeof(dRMS_UI));
	memset(dAC, 0, sizeof(dAC));
	
	res = InitFactorsData();
	
	if(res)
	{
#ifdef _DEBUG
		printf("InitFactorsData %d\n", res);
#endif
		sprintf(cDBG, "%d InitFactorsData is false (%d)\n", (int)time(NULL), res);
		myLog();
		return 2;
	}

	res = ReadHeader();
	
	if(res)
	{
#ifdef _DEBUG
		printf("ReadHeader %d\n", res);
#endif
		sprintf(cDBG, "%d ReadHeader is false (%d)\n", (int)time(NULL), res);
		myLog();
		return 3;
	}
	
	ulHandlerFactorStatus = HFS_FIRST_TIME;
	bcm2835_gpio_write(PIN_LED_READY, 1);
	
    Unlock();
#ifdef _DEBUG
	printf("Variant %d.\n", iR35AVariant);
#endif
	sprintf(cDBG, "%d R35-%04X start. Variant %d.\n", (int)time(NULL), *((unsigned short*)(strctFH.pcData+0x1fc)), iR35AVariant);
	myLog();
	
    while(1)
	{
        received = msgrcv(mq_id, &mymsg, sizeof(mymsg.data), 0, 0/*IPC_NOWAIT*/);
        if(received < 0)
		{
			perror("client: can't receive mess");
//			exit(1);
        }
		
		switch(mymsg.type)
		{
		case LM_HALF_PERIOD:
			res = mymsg.data[iCtPos] - usCT;

			if(res)
			{
#ifdef _DEBUG
				printf("%d != %d\n", usCT, mymsg.data[iCtPos]);
//#else
				sprintf(cDBG, "%d STM32: %d != %d\n", (int)time(NULL), usCT, mymsg.data[iCtPos]);
				myLog();
#endif
				usCT = mymsg.data[iCtPos];
				iSkipCounter = START_SKIP_COUNT;
			}

			usCT += HALF_PERIOD_COUNT;
			if(usCT >= MAX_COUNTER_VALUE)
				usCT = 0;
			
			
			// Предидущие полупериоды - в начало
			for(i=0;i<ADC_MEASUREMENT_COUNT;i++)
				memcpy(&dAC[i][0], &dAC[i][HALF_PERIOD_COUNT], HALF_PERIOD_COUNT*(START_SKIP_COUNT-1)*sizeof(double));
			
			dOff1 = dOff2 = 0.0;
			lpsTMPData2 = prev_mymsg.data;
			for(i=0;i<HALF_PERIOD_COUNT;i++,lpsTMPData2+=ONE_MEASUREMENT_SIZE)
			{
				dOff1 += (double)(*(lpsTMPData2+POS_ACOF12));
				dOff2 += (double)(*(lpsTMPData2+POS_ACOF34));
			}
			lpsTMPData2 = mymsg.data;


			for(i=0;i<HALF_PERIOD_COUNT;i++,lpsTMPData2+=ONE_MEASUREMENT_SIZE)
			{
				dOff1 += (double)(*(lpsTMPData2+POS_ACOF12));
				dOff2 += (double)(*(lpsTMPData2+POS_ACOF34));
				*(lpsTMPData2+ONE_MEASUREMENT_SIZE - 2) = ~(*(lpsTMPData2+ONE_MEASUREMENT_SIZE - 2));
			}
			if(ulHandlerFactorStatus & HFS_FIRST_TIME)
			{
				ulHandlerFactorStatus &= ~HFS_FIRST_TIME;
				saFactor[FACTORS_DIGITAL_POS].usDigitalValues = *lpusID & FACTORS_DIGITAL_MASK;
			}

			
			dOff1 /= (double)PERIOD_COUNT;
			dOff2 /= (double)PERIOD_COUNT;
			sOff1 = (short)(dOff1 + 0.5);
			sOff2 = (short)(dOff2 + 0.5);
			
/*QQQ
			if((iRecorderCounter & 1) == 0)
				memset(dRMS_UI, 0, sizeof(dRMS_UI));
*/
			// Убираем смещение и масштабируем
			lpsTMPData2 = mymsg.data;
			for(i=HALF_PERIOD_COUNT*(START_SKIP_COUNT-1);i<HALF_PERIOD_COUNT*START_SKIP_COUNT;i++,lpsTMPData2+=ONE_MEASUREMENT_SIZE)
			{
				lpsTMPData1 = lpsTMPData2 + posChannel[iR35AVariant][0];
				*lpsTMPData1 = (*lpsTMPData1 - sOff1);
				dAC[0][i]	= ((double)*lpsTMPData1) * dACScale[0];
//QQQ				dRMS_UI[0] += ((double)*lpsTMPData1) * ((double)*lpsTMPData1);
				
				lpsTMPData1 = lpsTMPData2 + posChannel[iR35AVariant][1];
				*lpsTMPData1 = (*lpsTMPData1 - sOff1);
//QQQ				dRMS_UI[1] += ((double)*lpsTMPData1) * ((double)*lpsTMPData1);
				dAC[1][i]	= ((double)*lpsTMPData1) * dACScale[1];
				
				lpsTMPData1 = lpsTMPData2 + posChannel[iR35AVariant][2];
				*lpsTMPData1 = (*lpsTMPData1 - sOff1);
//QQQ				dRMS_UI[2] += ((double)*lpsTMPData1) * ((double)*lpsTMPData1);
				dAC[2][i]	= ((double)*lpsTMPData1) * dACScale[2];
				
				lpsTMPData1 = lpsTMPData2 + posChannel[iR35AVariant][3];
				*lpsTMPData1 = (*lpsTMPData1 - sOff2);
//QQQ				dRMS_UI[3] += ((double)*lpsTMPData1) * ((double)*lpsTMPData1);
				dAC[3][i]	= ((double)*lpsTMPData1) * dACScale[3];
				
				lpsTMPData1 = lpsTMPData2 + posChannel[iR35AVariant][4];
				*lpsTMPData1 = (*lpsTMPData1 - sOff2);
//QQQ				dRMS_UI[4] += ((double)*lpsTMPData1) * ((double)*lpsTMPData1);
				dAC[4][i]	= ((double)*lpsTMPData1) * dACScale[4];
				
				lpsTMPData1 = lpsTMPData2 + posChannel[iR35AVariant][5];
				*lpsTMPData1 = (*lpsTMPData1 - sOff2);
//QQQ				dRMS_UI[5] += ((double)*lpsTMPData1) * ((double)*lpsTMPData1);
				dAC[5][i]	= ((double)*lpsTMPData1) * dACScale[5];
				
				lpsTMPData1 = lpsTMPData2 + posChannel[iR35AVariant][6];
				*lpsTMPData1 = (*lpsTMPData1 - sOff1);
//QQQ				dRMS_UI[6] += ((double)*lpsTMPData1) * ((double)*lpsTMPData1);
				dAC[6][i]	= ((double)*lpsTMPData1) * dACScale[6];
				
				lpsTMPData1 = lpsTMPData2 + posChannel[iR35AVariant][7];
				*lpsTMPData1 = (*lpsTMPData1 - sOff1);
//QQQ				dRMS_UI[7] += ((double)*lpsTMPData1) * ((double)*lpsTMPData1);
				dAC[7][i]	= ((double)*lpsTMPData1) * dACScale[7];
				
				lpsTMPData1 = lpsTMPData2 + posChannel[iR35AVariant][8];
				*lpsTMPData1 = (*lpsTMPData1 - sOff1);
//QQQ				dRMS_UI[8] += ((double)*lpsTMPData1) * ((double)*lpsTMPData1);
				dAC[8][i]	= ((double)*lpsTMPData1) * dACScale[8];
				
				lpsTMPData1 = lpsTMPData2 + posChannel[iR35AVariant][9];
				*lpsTMPData1 = (*lpsTMPData1 - sOff2);
//QQQ				dRMS_UI[9] += ((double)*lpsTMPData1) * ((double)*lpsTMPData1);
				dAC[9][i]	= ((double)*lpsTMPData1) * dACScale[9];
				
				lpsTMPData1 = lpsTMPData2 + posChannel[iR35AVariant][10];
				*lpsTMPData1 = (*lpsTMPData1 - sOff2);
//QQQ				dRMS_UI[10] += ((double)*lpsTMPData1) * ((double)*lpsTMPData1);
				dAC[10][i]	= ((double)*lpsTMPData1) * dACScale[10];
				
				lpsTMPData1 = lpsTMPData2 + posChannel[iR35AVariant][11];
				*lpsTMPData1 = (*lpsTMPData1 - sOff2);
//QQQ				dRMS_UI[11] += ((double)*lpsTMPData1) * ((double)*lpsTMPData1);
				dAC[11][i]	= ((double)*lpsTMPData1) * dACScale[11];
			}
			
			Lock();
			memcpy(pCurrentData, mymsg.data, sizeof(mymsg.data));
			Unlock();
			memcpy(&prev_mymsg, &mymsg, sizeof(mymsg));
			
/*QQQ			
			if((iRecorderCounter & 1) == 1)
			{
				for(j=0;j<ADC_SIGNAL_COUNT;j++)
				{
					dRMS_UI[j] = sqrt(dRMS_UI[j]) / 6.0;
					*lpCurrentRecorderFifoBuffer = (short)dRMS_UI[j];
#ifdef _DEBUG
					if(lCurrentDataPos == L_DATA_SIZE*2)
					{
						printf("%.1f ", dRMS_UI[j]*dACScale[j]);
					}
#endif					
					lpCurrentRecorderFifoBuffer++;
				}
			}

			iRecorderCounter++;
			
			if(iRecorderCounter == I_RECORDER_COUNT)
				Recorder(lpRecorderFifoBuffer);
			else
			{
				if(iRecorderCounter == I_RECORDER_COUNT * 2)
				{
					Recorder(lpRecorderFifoBuffer + (I_RECORDER_FIFO_BUFFER_COUNT / 2));
					lpCurrentRecorderFifoBuffer = lpRecorderFifoBuffer;
					iRecorderCounter = 0;
				}
			}
*/
			
			if(iSkipCounter)
			{
				iSkipCounter--;
				
				Lock();	
				lCurrentDataPos += L_DATA_SIZE;
				if(lCurrentDataPos >= COMMON_MEM_SIZE)
				{
					lCurrentDataPos = 0;
					pCurrentData = msg_p;
				}
				else
					pCurrentData += L_DATA_SIZE;
				Unlock();
				
				continue;
			}

			switch(iR35AVariant)
			{
				case 0:
					MainHandler0();
					break;
				case 1:
					MainHandler1();
					break;
				default:
					break;
			}


			FactorsHandler();
			ulHandlerFactorStatus &= ~HFS_FACTOR;
			iFactorPos = -1;
			for(i=0;i<GetFactorsCount();i++)
			{
				if(saFactor[i].ulFlags & FLAG_FACTOR_STATE)
				{
					ulHandlerFactorStatus |= HFS_FACTOR;
					if(!(ulHandlerFactorStatus & HFS_IS_SAVED) && !(saFactor[i].ulFlags & FLAG_FACTOR_IS_SAVED))
					{
#ifdef _DEBUG
						printf("Factor %d\n", i);
#endif
						ulHandlerFactorStatus &= ~HFS_ALREADY_SAVED_FACTOR;
						if(iFactorPos < 0)
							iFactorPos = i;
					}
					saFactor[i].ulFlags |= FLAG_FACTOR_IS_SAVED;
				}
			}
			
			if(*lpusID & 0x8000)
			{
				bcm2835_gpio_write(PIN_BLINKER, 0);
				bcm2835_gpio_write(PIN_LED_BLINKER, 0);
			}
				
			
			if(!(ulHandlerFactorStatus & HFS_IS_SAVED))
			{
				if((ulHandlerFactorStatus & HFS_FACTOR) && !(ulHandlerFactorStatus & HFS_ALREADY_SAVED_FACTOR))
				{
					ulHandlerFactorStatus |= HFS_ALREADY_SAVED_FACTOR;
					ulHandlerFactorStatus |= HFS_IS_SAVED;
					
					Lock();	
					strctSF.lPrevDataPos = lCurrentDataPos;
					strctSF.lpCurrDataPos = &lCurrentDataPos;
					strctSF.iFactorPos = iFactorPos; 
					Unlock();
					
					bcm2835_gpio_write(PIN_BLINKER, 1);
					bcm2835_gpio_write(PIN_LED_BLINKER, 1);
					SaveHistory(&strctSF);
					saFactor[FACTORS_TEST_POS].ulFlags &= ~FLAG_FACTOR_STATE;
					saFactor[FACTORS_TEST_POS].ulFlags &= ~FLAG_FACTOR_IS_SAVED;
				}
				
				if(!(ulHandlerFactorStatus & HFS_FACTOR) && (ulHandlerFactorStatus & HFS_ALREADY_SAVED_FACTOR))
				{
					ulHandlerFactorStatus &= ~HFS_ALREADY_SAVED_FACTOR;
					if(iFactorPos != FACTORS_DIGITAL_POS)
					{
						ulHandlerFactorStatus |= HFS_IS_SAVED;
						
						Lock();	
						strctSF.lPrevDataPos = lCurrentDataPos;
						strctSF.lpCurrDataPos = &lCurrentDataPos;
						strctSF.iFactorPos = iFactorPos; 
						Unlock();
						
						SaveHistory(&strctSF);
					}
				}
			}
	
			if(ulHandlerFactorStatus & HFS_SAVE_ERROR)
			{
//qqq
//				system("echo '1' > /dev/watchdog");
				exit(-21);
			}
			
			if((lCurrentDataPos+L_DATA_SIZE) >= COMMON_MEM_SIZE)
			{
//qqq
//				system("echo '1' > /dev/watchdog");
				pCurrentData = msg_p;
				Lock();	
				lCurrentDataPos = 0;
				Unlock();
			}
			else
			{
				pCurrentData += L_DATA_SIZE;
				Lock();	
				lCurrentDataPos += L_DATA_SIZE;
				Unlock();
			}
			
			break;
		case LM_UPDATE:
#ifdef _DEBUG
			printf("LM_UPDATE_DBG\n");
#endif
			sprintf(cDBG, "%d LM_UPDATE\n", (int)time(NULL));
			myLog();
			res = InitFactorsData();
			if(res)
			{
#ifdef _DEBUG
				printf("InitFactorsData %d\n", res);
#endif
				sprintf(cDBG, "%d InitFactorsData false (%d)\n", (int)time(NULL), res);
				myLog();
				return 4;
			}

			Lock();	
			res = ReadHeader();
			Unlock();
			if(res)
			{
#ifdef _DEBUG
				printf("ReadHeader %d\n", res);
#endif
				sprintf(cDBG, "%d ReadHeader false (%d)\n", (int)time(NULL), res);
				myLog();
				return 5;
			}
#ifdef _DEBUG
			bcm2835_gpio_write(PIN_BLINKER, 0);
			bcm2835_gpio_write(PIN_LED_BLINKER, 0);
#endif					
			iSkipCounter = START_SKIP_COUNT;
			break;
		case LM_TEST_FACTOR:
			saFactor[0].ulFlags |= FLAG_FACTOR_STATE;
#ifdef _DEBUG
			printf("LM_TEST_FACTOR\n");
#endif
			sprintf(cDBG, "%d LM_TEST_FACTOR\n", (int)time(NULL));
			myLog();
			break;
		default:
			bcm2835_gpio_write(PIN_LED_FAILURE, 1);
#ifdef _DEBUG
			printf("%d - unknown message.\n", mymsg.type);
#endif
			sprintf(cDBG, "%d %d - unknown message.\n", (int)time(NULL), mymsg.type);
			myLog();
			break;
        }
    }
    
	if(strctFH.pcData != NULL)
	{
		free(strctFH.pcData);
	}

	bcm2835_gpio_write(PIN_LED_BLINKER, 0);
	bcm2835_gpio_write(PIN_BLINKER, 0);
	bcm2835_gpio_write(PIN_LED_FAILURE, 0);
	bcm2835_gpio_write(PIN_LED_SAVE_FACTOR, 0);
	bcm2835_gpio_write(PIN_LED_READY, 0);
    return 0;
}

