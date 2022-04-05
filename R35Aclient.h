/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __R35_U_H
#define __R35_U_H

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <bcm2835.h>
#include <math.h>
#include "R35common.h"


#define FACTORS_COUNT_MAX	18 //!!!
#define FACTORS_COUNT0		18 // ручной тест + цифровые входы + 16 аналоговых
#define FACTORS_COUNT1		16 // ручной тест + цифровые входы + 14 аналоговых


#define START_SKIP_COUNT	3
#define FACTORS_TEST_POS	0 // !!! 0-й для ручного теста. В factors.txt не описывается
#define FACTORS_DIGITAL_POS	1 // !!! 1-й для цифровых входов. В factors.txt не описывается
#define FACTORS_DIGITAL_MASK	(short)0x7fff
#define PIN_LED_FAILURE		RPI_V2_GPIO_P1_29
#define PIN_LED_BLINKER		RPI_V2_GPIO_P1_31
#define PIN_LED_READY		RPI_V2_GPIO_P1_33
#define PIN_LED_SAVE_FACTOR	RPI_V2_GPIO_P1_35
#define PIN_BLINKER			RPI_V2_GPIO_P1_37

#define KEY (1492)
int idSemget; /* Number by which the semaphore is known within a program */
void Lock(void);
void Unlock(void);

#define POS_ACOF12 8	//8-PC0		
#define POS_ACOF34 9	//9-PC1
#define POS_ID 14		//14-POS_ID

extern int posChannel[2][12];

typedef struct strctFactor
{
	double*			lpdParameterValue;
	double			dCompareValue;
	double			dCompareValueWithHysteresis;
	unsigned long	ulFlags;
	unsigned long	ulDelay;
	unsigned long	ulCurrentDelay;
	unsigned long*	lpulFlagsSlave;//Если не NULL, то 'И' по флагам FLAG_COMPARATOR_STATE
	unsigned short	usDigitalValues;
	char			cName[32];
} strctFactor;

extern unsigned short* lpusID;
extern int iR35AVariant;
int GetFactorsCount(void);

// Для strctFactor.ulFlags
#define FLAG_COMPARATOR_UP		0x00000001	//Компаратор на превышение; иначе - на понижение
#define FLAG_COMPARATOR_STATE	0x00000002	//Состояние компаратора
#define FLAG_FACTOR_STATE		0x00000004	//Состояние фактора
#define FLAG_DIGITAL_USE		0x00000008	//Использовать lpusID;
//#define FLAG_DIGITAL_UP			0x00000010	//
#define FLAG_COMPARATOR_AND		0x00000020	//Фактор-ведомый. Часть логической 'И'. При этом у хотя бы у одного
#define FLAG_FACTOR_IS_USE		0x00000040	//Фактор используется, если флаг установлен; иначе - игнорируется. 
#define FLAG_FACTOR_IS_SAVED	0x00000080	//Фактор уже сохраненен в файле. 

unsigned long ulHandlerFactorStatus;
#define HFS_FACTOR					0x00000001 //
#define HFS_IS_SAVED				0x00000002 //В данный момент сохраняется в файле
#define HFS_ALREADY_SAVED_FACTOR	0x00000004 //Уже сохранен по фактору; иначе - по диагностике
#define HFS_FIRST_TIME				0x00000008 //Впервые
#define HFS_SAVE_ERROR				0x00000010

typedef struct strctFactorHeader
{
	size_t	stLength;
	char*	pcData;
} strctFactorHeader;

struct strctFactorHeader strctFH;

unsigned long L_PREV_MEASUREMENT_SIZE;
unsigned long L_MEASUREMENT_COUNT;

typedef struct strctSaveFactor
{
	long	lPrevDataPos;
	long*	lpCurrDataPos;
	int		iFactorPos;
} strctSaveFactor;

double dAC[ADC_MEASUREMENT_COUNT][HALF_PERIOD_COUNT*START_SKIP_COUNT];
double dACScale[ADC_MEASUREMENT_COUNT];

struct strctFactor	saFactor[FACTORS_COUNT_MAX];//!!!!!!!!!!!!!

int InitFactorsData(void);
int ReadHeader(void);
void FactorsHandler(void);
void WriteHistoryHeader(FILE** lplpFile, char* lpcFname);
void SaveHistory(struct strctSaveFactor* lpstrctSF);
//QQQvoid Recorder(short* lpsData);
extern short* msg_p;

char cDBG[256];
void myLog(void);

//For recorder////////////////////////////////////////////////////////
/*QQQ
#define I_RECORDER_COUNT					60000 // = 100 полупериодов в сек. * 10 мин.
#define I_RECORDER_FIFO_BUFFER_COUNT		720000 // = 50 периодов в сек. * 10 мин. * ADC_SIGNAL_COUNT * 2 !!!

double dRMS_UI[ADC_SIGNAL_COUNT];
*/
//For factors////////////////////////////////////////////////////////
//
void MainHandler0(void);
void MainHandler1(void);


#endif /* __R35_U_H */

