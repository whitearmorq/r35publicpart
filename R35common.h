/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __R35_C_H
#define __R35_C_H


#define SYS_PAGE_MEM_SIZE		4096
#define ADC_SIGNAL_COUNT		12
#define ADC_MEASUREMENT_COUNT	14
#define ONE_MEASUREMENT_SIZE	16
#define HALF_PERIOD_COUNT		18
#define PERIOD_COUNT			HALF_PERIOD_COUNT * 2

//	((HALF_PERIOD_COUNT * ONE_MEASUREMENT_COUNT * sizeof(short) * 10 * 100) / SYS_PAGE_MEM_SIZE) * SYS_PAGE_MEM_SIZE
#define COMMON_MEM_ALL_SIZE 	573440

//	(COMMON_MEM_ALL_SIZE / (HALF_PERIOD_COUNT * ONE_MEASUREMENT_COUNT * sizeof(short))) *
//	(HALF_PERIOD_COUNT * ONE_MEASUREMENT_COUNT)
#define COMMON_MEM_SIZE 		286560

#define MAX_COUNTER_VALUE (short)32760

#define L_DATA_SIZE (long)(ONE_MEASUREMENT_SIZE * HALF_PERIOD_COUNT	)

struct msgbuf
{ 
    long type; 
    short data[L_DATA_SIZE]; 
};

enum lMessages
{
	LM_HALF_PERIOD = 1,
	LM_UPDATE,
	LM_TEST_FACTOR
};

#define shmkey  9879
#define ipckey  9987

#define IPC_ACCESS 	00000666

/* Includes ------------------------------------------------------------------*/
#endif /* __R35_C_H */
