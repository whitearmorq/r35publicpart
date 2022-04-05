#include <regex.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>

#include "R35Aclient.h"
#include "R35AMainHandler.h"

const char C_FNAME_HEADER[] = "/home/boris/heap.r35set";
const char C_FNAME_LOG[] = "/home/boris/log.txt";

const char cMashAK[]	= "MashAK ";
const char cLngPost[]	= "LngPost";
const char cLngPred[]	= "LngPred";

short* shWriteFileBuffer = NULL;
size_t stCurrentWriteFileBufferLen = 0;

const char* lpFactorNames[2][FACTORS_COUNT_MAX] = {
	{
		"Net", "DC", "Ul1>", "Ul1<", "U01>", "U21>", "Uлвг1>", "Ia1>", "Ib1>",
		"Ic1>", "Ul2>", "Ul2<", "U02>", "U22>", "Uлвг2>", "Ia2>", "Ib2>", "Ic2>"
	},
	{
		"Net", "DC", "Ul1>", "Ul1<", "U01>", "U21>", "Uлвг1>", "3U0>", "Ia1>",
		"Ic1>", "Ia2>", "Ic2>", "Ia3>", "Ic3>", "Ia4>", "Ic4>", " ", " "
	}
};
const char cDignosticFactorName[] = "Diagn";
pthread_attr_t attr;
pthread_t thread;
unsigned short sASign;

pthread_attr_t attrRecord;
pthread_t threadRecord;

int GetFactorsCount(void)
{
	switch(iR35AVariant)
	{
		case 1:
			return FACTORS_COUNT1;
			break;
		default:
			return FACTORS_COUNT0;
			break;
	}
}

void myLog(void)
{
    FILE* lpLOG = NULL;
	
    lpLOG = fopen(C_FNAME_LOG, "a");   
    if(lpLOG == NULL)
    {
#ifdef _DEBUG
		printf("myLog. Open %s file error\n", C_FNAME_LOG);
#endif
		perror("myLog. Open file error\n");
		return;
    }
	
	fseek(lpLOG, 0, SEEK_END);
	fwrite(cDBG, strlen(cDBG), 1, lpLOG);
	fclose(lpLOG);
}

void SetWriteBuffer(short* lpsWriteData, short* lpsData)
{
	int i;
	for(i=0;i<sASign-1;i++)
		lpsWriteData[i] = *(lpsData + posChannel[iR35AVariant][i]);
	
	lpsWriteData[i] = *(lpsData+POS_ID);
}

int ReadHeader(void)
{
    FILE* lpFile = NULL;
	size_t stLen;
	float* lpfACScale;
	int i, j;
	unsigned char* lpTMP;
	unsigned short sOffset, iTVPStepSize;
	unsigned long ulLexemMask;
	
    lpFile = fopen (C_FNAME_HEADER, "rb");
    if(lpFile == NULL)
    {
		bcm2835_gpio_write(PIN_LED_FAILURE, 1);
#ifdef _DEBUG
		printf("ReadHeader. Open %s file error\n", C_FNAME_HEADER);
#endif
		sprintf(cDBG, "%d ReadHeader. Open %s file error\n", (int)time(NULL), C_FNAME_HEADER);
		myLog();
		return -1;
    }
	
	fseek(lpFile, 0, SEEK_END);
	stLen = ftell(lpFile);
	
	if(strctFH.pcData == NULL)
	{
		strctFH.pcData = (char*)malloc(stLen);
		if(strctFH.pcData == NULL)
		{
			bcm2835_gpio_write(PIN_LED_FAILURE, 1);
#ifdef _DEBUG
			printf("ReadHeader. 1 malloc error %d\n", stLen);
#endif
			sprintf(cDBG, "%d ReadHeader. 1 malloc error %d\n", (int)time(NULL), stLen);
			myLog();
			return -2;
		}
		strctFH.stLength = stLen;
	}
	else
	{
		if(strctFH.stLength != stLen)
		{
			strctFH.pcData = (char*)realloc(strctFH.pcData, stLen);
			if(strctFH.pcData == NULL)
			{
				bcm2835_gpio_write(PIN_LED_FAILURE, 1);
#ifdef _DEBUG
				printf("ReadHeader. 1 realloc error %d\n", stLen);
#endif
				sprintf(cDBG, "%d ReadHeader. 1 realloc error %d\n", (int)time(NULL), stLen);
				myLog();
				return -3;
			}
			strctFH.stLength = stLen;
		}
	}
	
	fseek(lpFile, 0, SEEK_SET);
	fread(strctFH.pcData, strctFH.stLength, 1, lpFile);

	i = *((unsigned short*)(strctFH.pcData+0x16));//TVPOffset
	lpTMP = strctFH.pcData + i;
	iTVPStepSize = (unsigned short)(*(lpTMP+7));
	i = (int)(*(lpTMP+8));

	ulLexemMask = 0x00000007;
	while(i > 0)
	{
		if((ulLexemMask & 0x00000001) && !strncmp(lpTMP, cMashAK, 7))
		{
			ulLexemMask &= 0xfffffffe;
			sOffset = *((short*)(lpTMP+9));
			sOffset += 	*((short*)(strctFH.pcData+0x14));//HeaderLong
			
			lpfACScale = (float* )(strctFH.pcData+sOffset);
			for(j=0;j<ADC_MEASUREMENT_COUNT;j++, lpfACScale++)
				dACScale[j] = fabs((double)*lpfACScale);
		}
		
		if((ulLexemMask & 0x00000002) && !strncmp(lpTMP, cLngPost, 7))
		{
			ulLexemMask &= 0xfffffffd;
			sOffset = *((short*)(lpTMP+9));
			sOffset += 	*((short*)(strctFH.pcData+0x14));//HeaderLong
				
			L_MEASUREMENT_COUNT = *((unsigned long*)(strctFH.pcData+sOffset));
			L_MEASUREMENT_COUNT /= 10;
			if(L_MEASUREMENT_COUNT > 900)
				L_MEASUREMENT_COUNT = 900;
		}
			
		if((ulLexemMask & 0x00000004) && !strncmp(lpTMP, cLngPred, 7))
		{
			ulLexemMask &= 0xfffffffb;
			sOffset = *((short*)(lpTMP+9));
			sOffset += 	*((short*)(strctFH.pcData+0x14));//HeaderLong
				
			L_PREV_MEASUREMENT_SIZE = *((long*)(strctFH.pcData+sOffset));
			L_PREV_MEASUREMENT_SIZE /= 10;
			if(L_PREV_MEASUREMENT_SIZE > 100)
				L_PREV_MEASUREMENT_SIZE = 100;
		}

		if(!ulLexemMask)
			break;
		
		lpTMP += iTVPStepSize;
		i--;
	}
	
    fclose(lpFile);
	
	if(shWriteFileBuffer == NULL)
	{
		stCurrentWriteFileBufferLen = *((unsigned long*)(strctFH.pcData+0x42));
		shWriteFileBuffer = (short*)malloc(stCurrentWriteFileBufferLen);
		
		if(shWriteFileBuffer == NULL)
		{
			bcm2835_gpio_write(PIN_LED_FAILURE, 1);
#ifdef _DEBUG
			printf("ReadHeader. 2 malloc error %d\n", stCurrentWriteFileBufferLen);
#endif
			sprintf(cDBG, "%d ReadHeader. 2 malloc error %d\n", (int)time(NULL), stCurrentWriteFileBufferLen);
			myLog();
			return -2;
		}
	}
	else
	{
		if(stCurrentWriteFileBufferLen < *((unsigned long*)(strctFH.pcData+0x42)))
		{
			stCurrentWriteFileBufferLen = *((unsigned long*)(strctFH.pcData+0x42));
			shWriteFileBuffer = (short*)realloc(shWriteFileBuffer, stCurrentWriteFileBufferLen);
			if(shWriteFileBuffer == NULL)
			{
				bcm2835_gpio_write(PIN_LED_FAILURE, 1);
#ifdef _DEBUG
				printf("ReadHeader. 2 realloc error %d\n", stCurrentWriteFileBufferLen);
#endif
				sprintf(cDBG, "%d ReadHeader. 2 realloc error %d\n", (int)time(NULL), stCurrentWriteFileBufferLen);
				myLog();
				return -3;
			}
		}
	}
	
	return 0;
}

void WriteHistoryHeader(FILE** lplpFile, char* lpcFname)
{
    FILE* 	lpFileHeader = NULL;
//    char	cFname1[128];
    time_t	seconds;
	short	shData;
	short	shTime;
	short	shAdd;
	unsigned short	shShortCount, shShortNum;
    struct	tm tm;
	unsigned char* lpTMP;
	unsigned short sOffset, iTVPStepSize;
	unsigned long ulLexemMask;
	unsigned long ulPrevSize, ulPostSize;
	int i;

    unsigned long            ms; // Milliseconds
    time_t          s;  // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    seconds = time (NULL);
	tm = *localtime(&seconds);
	
	shShortCount = *((unsigned short*)(strctFH.pcData+0x38));//sNumPrc
	shShortNum = *((unsigned short*)(strctFH.pcData+0x1fc));//sNumReg

	if(ulHandlerFactorStatus & HFS_ALREADY_SAVED_FACTOR)
	{
		sprintf(lpcFname, "R35-%04X-20%02d-%02d-%02d(%04d).winrec",
			shShortNum,
			tm.tm_year - 100, tm.tm_mon + 1, tm.tm_mday, shShortCount);
	}
	else
	{
		sprintf(lpcFname, "D35-%04X-20%02d-%02d-%02d(%04d).winrec",
			shShortNum,
			tm.tm_year - 100, tm.tm_mon + 1, tm.tm_mday, shShortCount);
	}
/*		
	while(shShortCount > 999)
		shShortCount -= 1000;
	
	while(shShortNum > 99)
		shShortNum -= 100;
	

	if(ulHandlerFactorStatus & HFS_ALREADY_SAVED_FACTOR)
		sprintf(cFname1, "ReconR%02d.%03d", shShortNum, shShortCount);
	else
		sprintf(cFname1, "ReconD%02d.%03d", shShortNum, shShortCount);
*/		
    *lplpFile = fopen(lpcFname, "wb");   
    if(*lplpFile == NULL)
    {
		bcm2835_gpio_write(PIN_LED_FAILURE, 1);
#ifdef _DEBUG
		printf("WriteHistoryHeader. Create (%04d) file error\n", shShortCount);
#endif
		sprintf(cDBG, "%d WriteHistoryHeader. Create file (%04d) error: %s\n", (int)time(NULL), shShortCount, strerror(errno));
		myLog();
		return;
    }
	
//	chmod(lpcFname, S_IRGRP|S_IROTH|S_IRUSR);

	shAdd = (tm.tm_sec & 1) ? 1800 : 0;
	
//	ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
	ms = spec.tv_nsec / 100000; // Convert nanoseconds to milliseconds
	shAdd  += (short)(ms * 18 / 100);
	
	shData = (short)tm.tm_year - 2000;//1980;
	shData <<= 4;
	shData += (short)(tm.tm_mon+1);
	shData <<= 5;
	shData += (short)tm.tm_mday;
	
	shTime = tm.tm_hour;
	shTime <<= 6;
	shTime += tm.tm_min;
	shTime <<= 5;
	tm.tm_sec >>= 1;
	shTime += tm.tm_sec;
	
	*((short*)(strctFH.pcData+0x20)) = shData;//sDate
	*((short*)(strctFH.pcData+0x22)) = shTime;//sTime
	*((short*)(strctFH.pcData+0x24)) = shAdd;//sUTime
	
	//sASign
	sASign = (unsigned short)(*((unsigned char*)(strctFH.pcData+0x4e)));
	sASign++;

	if(ulHandlerFactorStatus & HFS_ALREADY_SAVED_FACTOR)
	{
//		ReconR
		i = *((unsigned short*)(strctFH.pcData+0x16));//TVPOffset
		lpTMP = strctFH.pcData + i;
		iTVPStepSize = (unsigned short)(*(lpTMP+7));
		i = (int)(*(lpTMP+8));

		ulLexemMask = 0x00000003;
		while(i > 0)
		{
			if((ulLexemMask & 0x00000001) && !strncmp(lpTMP, cLngPost, 7))
			{
				ulLexemMask &= 0xfffffffe;
				sOffset = *((short*)(lpTMP+9));
				sOffset += 	*((short*)(strctFH.pcData+0x14));//HeaderLong
				
				L_MEASUREMENT_COUNT = *((unsigned long*)(strctFH.pcData+sOffset));
				L_MEASUREMENT_COUNT /= 10;
				ulPostSize = L_MEASUREMENT_COUNT;
			}
			
			if((ulLexemMask & 0x00000002) && !strncmp(lpTMP, cLngPred, 7))
			{
				ulLexemMask &= 0xfffffffd;
				sOffset = *((short*)(lpTMP+9));
				sOffset += 	*((short*)(strctFH.pcData+0x14));//HeaderLong
				
				L_PREV_MEASUREMENT_SIZE = *((long*)(strctFH.pcData+sOffset));
				L_PREV_MEASUREMENT_SIZE /= 10;
				ulPrevSize = L_PREV_MEASUREMENT_SIZE;
			}
				
			if(!ulLexemMask)
				break;
			
			lpTMP += iTVPStepSize;
			i--;
		}
	}
	else
	{
	//		"ReconD
		ulPrevSize = L_PREV_MEASUREMENT_SIZE;
		L_PREV_MEASUREMENT_SIZE = 25;
		ulPostSize = L_MEASUREMENT_COUNT;
		L_MEASUREMENT_COUNT = 50;
	}
		
	
	//sDtaLen
	*((unsigned long*)(strctFH.pcData+0x42)) = sASign * HALF_PERIOD_COUNT * (L_PREV_MEASUREMENT_SIZE + L_MEASUREMENT_COUNT) * sizeof(short);
	//sLoPi
	*((unsigned long*)(strctFH.pcData+0x46)) = HALF_PERIOD_COUNT * L_PREV_MEASUREMENT_SIZE;
	//sLoPro
	*((unsigned long*)(strctFH.pcData+0x4a)) = HALF_PERIOD_COUNT * L_MEASUREMENT_COUNT;
	fwrite(strctFH.pcData, strctFH.stLength, 1, *lplpFile);
	
	//sNumPrc
	*((unsigned short*)(strctFH.pcData+0x38)) += 1;

	
    lpFileHeader = fopen(C_FNAME_HEADER, "wb");
    if(lpFileHeader == NULL)
    {
		bcm2835_gpio_write(PIN_LED_FAILURE, 1);
#ifdef _DEBUG
		printf("WriteHistoryHeader. Open %s file error\n", C_FNAME_HEADER);
#endif
		sprintf(cDBG, "WriteHistoryHeader. Open %s file error\n", C_FNAME_HEADER);
		myLog();
		return;
    }

	//sDtaLen
	*((unsigned long*)(strctFH.pcData+0x42)) = sASign * HALF_PERIOD_COUNT * (ulPrevSize + ulPostSize) * sizeof(short);
	//sLoPi
	*((unsigned long*)(strctFH.pcData+0x46)) = HALF_PERIOD_COUNT * ulPrevSize;
	//sLoPro
	*((unsigned long*)(strctFH.pcData+0x4a)) = HALF_PERIOD_COUNT * ulPostSize;
	
	fwrite(strctFH.pcData, strctFH.stLength, 1, lpFileHeader);
	fflush(lpFileHeader);
    fclose(lpFileHeader);
	
//	symlink(lpcFname, cFname1);
}


void* SaveFactor(void* lpData)
{
#ifdef _DEBUG
	unsigned long lStartDataPos;
#endif

    char	cFname[128];
	unsigned long lCurrDataPos, lPrevDataPos, i, j, k, usRetWrite;
	unsigned long iMeasurementCount, lPrevMeasurementSize, iMeasurementSize;
	short*	lpsTMPData;
    FILE* lpFile = NULL;
	struct strctSaveFactor* lpstrctSF = (strctSaveFactor*)lpData;
	
	short shWriteBuffer[ONE_MEASUREMENT_SIZE-1];
	int idSemget;
	short* shTmpWriteFileBuffer;

	
	Lock();	
	WriteHistoryHeader(&lpFile, cFname);
	lCurrDataPos = lpstrctSF->lPrevDataPos;
	lPrevMeasurementSize = L_PREV_MEASUREMENT_SIZE;
	iMeasurementSize = iMeasurementCount = L_MEASUREMENT_COUNT;
	Unlock();
	
	if(lpFile == NULL || shWriteFileBuffer == NULL)
	{
		ulHandlerFactorStatus |= HFS_SAVE_ERROR;
		return (void*)-1;
	}
	
	bcm2835_gpio_write(PIN_LED_SAVE_FACTOR, 1);

	shTmpWriteFileBuffer = shWriteFileBuffer;
	if((lCurrDataPos - (lPrevMeasurementSize * L_DATA_SIZE)) >= 0)
	{
		lpsTMPData = msg_p + (lCurrDataPos - lPrevMeasurementSize  * L_DATA_SIZE);
		
		for(i=0;i<lPrevMeasurementSize;i++)
		{
//			for(j=0;j<HALF_PERIOD_COUNT;j++,lpsTMPData+=ONE_MEASUREMENT_SIZE)
			for(j=0;j<HALF_PERIOD_COUNT;j++,lpsTMPData+=ONE_MEASUREMENT_SIZE, shTmpWriteFileBuffer+=sASign)
			{
				SetWriteBuffer(shWriteBuffer, lpsTMPData);
//				usRetWrite = fwrite(shWriteBuffer, sASign*sizeof(short), 1, lpFile);
				memcpy(shTmpWriteFileBuffer, shWriteBuffer, sASign*sizeof(short));
			}
		}
	}
	else
	{
		lpsTMPData = msg_p + COMMON_MEM_SIZE - (lPrevMeasurementSize  * L_DATA_SIZE - lCurrDataPos);
		k = lPrevMeasurementSize - (lCurrDataPos / L_DATA_SIZE);
			
		for(i=0;i<k;i++)
		{
//			for(j=0;j<HALF_PERIOD_COUNT;j++,lpsTMPData+=ONE_MEASUREMENT_SIZE)
			for(j=0;j<HALF_PERIOD_COUNT;j++,lpsTMPData+=ONE_MEASUREMENT_SIZE, shTmpWriteFileBuffer+=sASign)
			{
				SetWriteBuffer(shWriteBuffer, lpsTMPData);
//				usRetWrite = fwrite(shWriteBuffer, sASign*sizeof(short), 1, lpFile);
				memcpy(shTmpWriteFileBuffer, shWriteBuffer, sASign*sizeof(short));
			}
		}
		
		lpsTMPData = msg_p;
		for(;i<lPrevMeasurementSize;i++)
		{
//			for(j=0;j<HALF_PERIOD_COUNT;j++,lpsTMPData+=ONE_MEASUREMENT_SIZE)
			for(j=0;j<HALF_PERIOD_COUNT;j++,lpsTMPData+=ONE_MEASUREMENT_SIZE, shTmpWriteFileBuffer+=sASign)
			{
				SetWriteBuffer(shWriteBuffer, lpsTMPData);
//				usRetWrite = fwrite(shWriteBuffer, sASign*sizeof(short), 1, lpFile);
				memcpy(shTmpWriteFileBuffer, shWriteBuffer, sASign*sizeof(short));
			}
		}
	}

	usRetWrite = fwrite(shWriteFileBuffer, lPrevMeasurementSize*HALF_PERIOD_COUNT*sASign*sizeof(short), 1, lpFile);
	fflush(lpFile);

	lPrevDataPos = lCurrDataPos;
	while(iMeasurementCount)
	{
		Lock();	
		lCurrDataPos = *lpstrctSF->lpCurrDataPos;
		Unlock();
		
		if(lPrevDataPos == lCurrDataPos)
			continue;


		i=lPrevDataPos;
		while(i != lCurrDataPos)
		{
//			for(j=0;j<HALF_PERIOD_COUNT;j++,lpsTMPData+=ONE_MEASUREMENT_SIZE)
			for(j=0;j<HALF_PERIOD_COUNT;j++,lpsTMPData+=ONE_MEASUREMENT_SIZE, shTmpWriteFileBuffer+=sASign)
			{
				Lock();
				SetWriteBuffer(shWriteBuffer, lpsTMPData);
				Unlock();
//				usRetWrite = fwrite(shWriteBuffer, sASign*sizeof(short), 1, lpFile);
				memcpy(shTmpWriteFileBuffer, shWriteBuffer, sASign*sizeof(short));
			}
			
			i+=L_DATA_SIZE;
			if(i >= COMMON_MEM_SIZE)
			{
				i = 0;
				lpsTMPData = msg_p;
			}
			iMeasurementCount--;
			if(!iMeasurementCount)
				break;
		}

		lPrevDataPos = lCurrDataPos;
	}

	usRetWrite = fwrite(shWriteFileBuffer+lPrevMeasurementSize*HALF_PERIOD_COUNT*sASign,
		iMeasurementSize*HALF_PERIOD_COUNT*sASign*sizeof(short), 1, lpFile);
	
	Lock();	
	fflush(lpFile);
//	fseek(lpFile, 0L, SEEK_END);
	i = ftell(lpFile);
	fclose(lpFile);
	Unlock();
	
	bcm2835_gpio_write(PIN_LED_SAVE_FACTOR, 0);
//qqq
	j = strctFH.stLength + sASign * HALF_PERIOD_COUNT * (L_PREV_MEASUREMENT_SIZE + L_MEASUREMENT_COUNT) * sizeof(short);
#ifdef _DEBUG
	if(i != j)
	{
		sprintf(cDBG, "%d Length %d file %d. Must be %d+%d*%d* %d+%d *2\n", (int)time(NULL),
				i, *((unsigned short*)(strctFH.pcData+0x38))-1,
				strctFH.stLength, sASign, HALF_PERIOD_COUNT, L_PREV_MEASUREMENT_SIZE, L_MEASUREMENT_COUNT);
		myLog();
		sprintf(cDBG, "%d StartDataPos %d CurrDataPos %d.\n", (int)time(NULL),
			lStartDataPos, lPrevDataPos);
		myLog();
		sprintf(cDBG, "%d .\n", (int)time(NULL));
		myLog();
	}
#endif
		ulHandlerFactorStatus &= ~HFS_IS_SAVED;
		return 0;

///////////////////////////
//	sprintf(cDBG, "%d Length %04d file is %d.\n", (int)time(NULL),
//		*((unsigned short*)(strctFH.pcData+0x38)), i);
	sprintf(cDBG, "%d Length %d file %d <. Must be %d\n", (int)time(NULL),
		i, *((unsigned short*)(strctFH.pcData+0x38)), j);
	myLog();
	
//qqq
	sleep(1);
	ulHandlerFactorStatus &= ~HFS_IS_SAVED;

	ulHandlerFactorStatus |= HFS_SAVE_ERROR;
	return (void*)-21;
}

void SaveHistory(struct strctSaveFactor* lpstrctSF)
{
	static int isThreadInit = 0;
	if(!isThreadInit || (pthread_kill(thread, 0) != 0))
	{
		Lock();
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		pthread_create(&thread, &attr, SaveFactor, lpstrctSF);
		pthread_attr_destroy(&attr);
		isThreadInit = 1;
		Unlock();
	}
}

int InitFactorsData0(void)
{
	saFactor[FACTORS_DIGITAL_POS+1].lpdParameterValue = &dRMS_UAB1;
	saFactor[FACTORS_DIGITAL_POS+1].ulFlags |= FLAG_COMPARATOR_UP;
	
	saFactor[FACTORS_DIGITAL_POS+2].lpdParameterValue = &dRMS_UAB1;

	saFactor[FACTORS_DIGITAL_POS+3].lpdParameterValue = &dRMS_3U01;
	saFactor[FACTORS_DIGITAL_POS+3].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+4].lpdParameterValue = &dRMS_U21;
	saFactor[FACTORS_DIGITAL_POS+4].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+5].lpdParameterValue = &dRMS_UL1;
	saFactor[FACTORS_DIGITAL_POS+5].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+6].lpdParameterValue = &dRMS_IA1;
	saFactor[FACTORS_DIGITAL_POS+6].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+7].lpdParameterValue = &dRMS_IB1;
	saFactor[FACTORS_DIGITAL_POS+7].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+8].lpdParameterValue = &dRMS_IC1;
	saFactor[FACTORS_DIGITAL_POS+8].ulFlags |= FLAG_COMPARATOR_UP;
//////////////
	saFactor[FACTORS_DIGITAL_POS+9].lpdParameterValue = &dRMS_UAB2;
	saFactor[FACTORS_DIGITAL_POS+9].ulFlags |= FLAG_COMPARATOR_UP;
	
	saFactor[FACTORS_DIGITAL_POS+10].lpdParameterValue = &dRMS_UAB2;

	saFactor[FACTORS_DIGITAL_POS+11].lpdParameterValue = &dRMS_3U02;
	saFactor[FACTORS_DIGITAL_POS+11].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+12].lpdParameterValue = &dRMS_U22;
	saFactor[FACTORS_DIGITAL_POS+12].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+13].lpdParameterValue = &dRMS_UL2;
	saFactor[FACTORS_DIGITAL_POS+13].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+14].lpdParameterValue = &dRMS_IA2;
	saFactor[FACTORS_DIGITAL_POS+14].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+15].lpdParameterValue = &dRMS_IB2;
	saFactor[FACTORS_DIGITAL_POS+15].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+16].lpdParameterValue = &dRMS_IC2;
	saFactor[FACTORS_DIGITAL_POS+16].ulFlags |= FLAG_COMPARATOR_UP;
}

int InitFactorsData1(void)
{
	saFactor[FACTORS_DIGITAL_POS+1].lpdParameterValue = &dRMS_UAB1;
	saFactor[FACTORS_DIGITAL_POS+1].ulFlags |= FLAG_COMPARATOR_UP;
	
	saFactor[FACTORS_DIGITAL_POS+2].lpdParameterValue = &dRMS_UAB1;

	saFactor[FACTORS_DIGITAL_POS+3].lpdParameterValue = &dRMS_3U01;
	saFactor[FACTORS_DIGITAL_POS+3].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+4].lpdParameterValue = &dRMS_U21;
	saFactor[FACTORS_DIGITAL_POS+4].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+5].lpdParameterValue = &dRMS_UL1;
	saFactor[FACTORS_DIGITAL_POS+5].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+6].lpdParameterValue = &dRMS_IA1;
	saFactor[FACTORS_DIGITAL_POS+6].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+7].lpdParameterValue = &dRMS_IB1;
	saFactor[FACTORS_DIGITAL_POS+7].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+8].lpdParameterValue = &dRMS_IC1;
	saFactor[FACTORS_DIGITAL_POS+8].ulFlags |= FLAG_COMPARATOR_UP;
//////////////
	saFactor[FACTORS_DIGITAL_POS+9].lpdParameterValue = &dRMS_UAB2;
	saFactor[FACTORS_DIGITAL_POS+9].ulFlags |= FLAG_COMPARATOR_UP;
	
	saFactor[FACTORS_DIGITAL_POS+10].lpdParameterValue = &dRMS_3U02;
	saFactor[FACTORS_DIGITAL_POS+10].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+11].lpdParameterValue = &dRMS_U22;
	saFactor[FACTORS_DIGITAL_POS+11].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+12].lpdParameterValue = &dRMS_UL2;
	saFactor[FACTORS_DIGITAL_POS+12].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+13].lpdParameterValue = &dRMS_IA2;
	saFactor[FACTORS_DIGITAL_POS+13].ulFlags |= FLAG_COMPARATOR_UP;

	saFactor[FACTORS_DIGITAL_POS+14].lpdParameterValue = &dRMS_IB2;
	saFactor[FACTORS_DIGITAL_POS+14].ulFlags |= FLAG_COMPARATOR_UP;
}

int InitFactorsData(void)
{
	int i, j;
    regex_t regex;
    int reti;
    char msgbuf[100];
    char p[256];
    FILE* f;
    char* lpsTMP;
	char* lpsSaveptr;
    double dTMP;
	unsigned long ulTMP;
	
///////////////////////////////////////////////////////////////////////	
	//инициализация iR35AVariant
    f = fopen("/home/boris/config.txt","r");
    if(f == NULL)
	{
#ifdef _DEBUG
		printf("File config.txt failed.\n");
#endif
		sprintf(cDBG, "File config.txt failed.\n", (int)time(NULL));
		myLog();
//		return -4;
	}
	fgets(p, 256, f);
	iR35AVariant = atoi(p);
    fclose(f);
	
	if((iR35AVariant < 0) || (iR35AVariant > 1))
	{
#ifdef _DEBUG
		printf("Error Variant %d. Must be 0 or 1.\n", iR35AVariant);
#endif
		sprintf(cDBG, "%d Error Variant %d. Must be 0 or 1.\n", (int)time(NULL), iR35AVariant);
		myLog();
		return -4;
	}

	saFactor[FACTORS_TEST_POS].ulDelay = 1;
	saFactor[FACTORS_TEST_POS].ulFlags |= FLAG_FACTOR_IS_USE;
	saFactor[FACTORS_DIGITAL_POS].ulDelay = 1;
	saFactor[FACTORS_DIGITAL_POS].ulFlags |= FLAG_FACTOR_IS_USE | FLAG_DIGITAL_USE;

	
    f = fopen("/home/boris/factors.txt","r");
    if(f == NULL)
		return -1;
///////////////////////////////////////////////////////////////////////	
	switch(iR35AVariant)
	{
		case 1:
			InitFactorsData1();
			strcpy(saFactor[FACTORS_TEST_POS].cName, lpFactorNames[1][FACTORS_TEST_POS]);
			strcpy(saFactor[FACTORS_DIGITAL_POS].cName, lpFactorNames[1][FACTORS_DIGITAL_POS]);
			break;
		default:
			strcpy(saFactor[FACTORS_TEST_POS].cName, lpFactorNames[0][FACTORS_TEST_POS]);
			strcpy(saFactor[FACTORS_DIGITAL_POS].cName, lpFactorNames[0][FACTORS_DIGITAL_POS]);
			InitFactorsData0();
			break;
	}
///////////////////////////////////////////////////////////////////////	
    reti = regcomp(&regex, "^[0-9]+[.]{0,1}[0-9]{0,}[ ]{1,}[0-9]+[.]{0,1}[0-9]{0,}[ ]{1,}[0-9]+[.]{0,1}[0-9]{0,}[ ]{1,}", REG_EXTENDED);

    if(reti)
	{
		fclose(f);
		bcm2835_gpio_write(PIN_LED_FAILURE, 1);
		return -2;
	}
///////////////////////////////////////////////////////////////////////	

	for(i=FACTORS_DIGITAL_POS+1;i<GetFactorsCount();i++)
	{
		memset(saFactor[i].cName, 0, sizeof(saFactor[i].cName)); 
		if(feof(f))
		{
			regfree(&regex);
			fclose(f);
#ifdef _DEBUG
			printf("feof %d\n", i);
#endif
			sprintf(cDBG, "%d feof %d\n", (int)time(NULL), i);
			myLog();

			bcm2835_gpio_write(PIN_LED_FAILURE, 1);
			return -3;
		}

		fgets(p, 256, f);
		reti = regexec(&regex, p, 0, NULL, 0);
		if(!reti)
		{
//			printf("Match: %s", p);
			//parse_match_srting(struct* strctFactor, char* mstring)

			lpsTMP = strtok_r(p, " ", &lpsSaveptr);
			saFactor[i].dCompareValue = atof(lpsTMP);
			if(saFactor[i].dCompareValue < 0.0)
				saFactor[i].dCompareValue = 0.0;
			
			dTMP = saFactor[i].dCompareValue * 0.05;
			if((saFactor[i].ulFlags & FLAG_COMPARATOR_UP) != 0)
			{
				saFactor[i].dCompareValueWithHysteresis = saFactor[i].dCompareValue - dTMP;
				if(saFactor[i].dCompareValueWithHysteresis < 0.0)
					saFactor[i].dCompareValueWithHysteresis = 0.0;
			}
			else
			{
				saFactor[i].dCompareValueWithHysteresis = saFactor[i].dCompareValue + dTMP;
			}
						
			lpsTMP	= strtok_r(NULL, " ", &lpsSaveptr);
			saFactor[i].ulDelay	= atoi(lpsTMP);
			if(saFactor[i].ulDelay > 255)
				saFactor[i].ulDelay = 255;
			
			lpsTMP	= strtok_r(NULL, " ", &lpsSaveptr);
			if(atoi(lpsTMP) != 0)
				saFactor[i].ulFlags |= FLAG_FACTOR_IS_USE;
			else
				saFactor[i].ulFlags &= ~FLAG_FACTOR_IS_USE;
			
			lpsTMP	= strtok_r(NULL, " ", &lpsSaveptr);

			if(strlen(lpsTMP))
			{
				memcpy(saFactor[i].cName, lpsTMP, strlen(lpsTMP));
				saFactor[i].cName[strlen(lpsTMP)-1] = '\0';
			}
			
			
#ifdef _DEBUG
			printf("as:%f, %f, %d, %d, %s\n", saFactor[i].dCompareValue, saFactor[i].dCompareValueWithHysteresis,
				saFactor[i].ulDelay, ((saFactor[i].ulFlags & FLAG_FACTOR_IS_USE)?1:0), saFactor[i].cName);
#endif
		}
		else
		{
			if( reti == REG_NOMATCH )
			{
#ifdef _DEBUG
				printf("No match\n");
#endif
				sprintf(cDBG, "%d No match\n", (int)time(NULL));
				myLog();
			}
			else
			{
				regerror(reti, &regex, msgbuf, sizeof(msgbuf));
				fclose(f);
				regfree(&regex);
				return -4;
			}
		}

		saFactor[i].ulFlags &= ~(FLAG_COMPARATOR_STATE|FLAG_FACTOR_STATE);
		saFactor[i].ulCurrentDelay = 0;
	}
	
    regfree(&regex);
    fclose(f);
    return 0;
}

void FactorsHandler(void)
{
	int i, j;
	unsigned short* lpusDTmp;

	for(i=FACTORS_DIGITAL_POS;i<GetFactorsCount();i++)
	{
		if(saFactor[i].ulFlags & FLAG_FACTOR_IS_USE)
		{
			//Компаратор включается по двоичному входу
			if(saFactor[i].ulFlags & FLAG_DIGITAL_USE)
			{
				lpusDTmp = lpusID;
				saFactor[i].ulFlags &= ~FLAG_COMPARATOR_STATE;
				for(j=0;j<HALF_PERIOD_COUNT;j++, lpusDTmp+=ONE_MEASUREMENT_SIZE)
				{
					if(saFactor[i].usDigitalValues != (*lpusDTmp & FACTORS_DIGITAL_MASK))
					{
						saFactor[i].ulFlags |= FLAG_COMPARATOR_STATE;
					}
					saFactor[i].usDigitalValues = (*lpusDTmp & FACTORS_DIGITAL_MASK);
				}
				saFactor[i].ulCurrentDelay = 0;
			}
			else//Обработка компаратора с учетом гистерезиса
			{
#ifdef _DEBUG
				if(saFactor[i].lpdParameterValue == NULL)
				{
					printf("Factor &d parameter is NULL\n");
					continue;
				}
#endif
				if(saFactor[i].ulFlags & FLAG_COMPARATOR_UP)//Компаратор верхнего уровня
				{
					if(saFactor[i].ulFlags & FLAG_COMPARATOR_STATE)//Если уже установлен
					{
						if(saFactor[i].ulCurrentDelay < saFactor[i].ulDelay)
						{
							if(*saFactor[i].lpdParameterValue < saFactor[i].dCompareValue)
							{
								saFactor[i].ulFlags &= ~FLAG_COMPARATOR_STATE;
								saFactor[i].ulCurrentDelay = 0;
							}
						}
						else
						{
							if(*saFactor[i].lpdParameterValue < saFactor[i].dCompareValueWithHysteresis)
							{
								saFactor[i].ulFlags &= ~FLAG_COMPARATOR_STATE;
								saFactor[i].ulCurrentDelay = 0;
							}
						}
					}
					else
					{
						if(*saFactor[i].lpdParameterValue > saFactor[i].dCompareValue)
							saFactor[i].ulFlags |= FLAG_COMPARATOR_STATE;
					}
				}
				else//Компаратор нижнего уровня
				{
					if(saFactor[i].ulFlags & FLAG_COMPARATOR_STATE)//Если уже установлен
					{
						if(saFactor[i].ulCurrentDelay < saFactor[i].ulDelay)
						{
							if(*saFactor[i].lpdParameterValue > saFactor[i].dCompareValue)
							{
								saFactor[i].ulFlags &= ~FLAG_COMPARATOR_STATE;
								saFactor[i].ulCurrentDelay = 0;
							}
						}
						else
						{
							if(*saFactor[i].lpdParameterValue > saFactor[i].dCompareValueWithHysteresis)
							{
								saFactor[i].ulFlags &= ~FLAG_COMPARATOR_STATE;
								saFactor[i].ulCurrentDelay = 0;
							}
						}
					}
					else
					{
						if(*saFactor[i].lpdParameterValue < saFactor[i].dCompareValue)
							saFactor[i].ulFlags |= FLAG_COMPARATOR_STATE;
					}
				}
			}
			
			if(saFactor[i].lpulFlagsSlave != NULL)
				saFactor[i].ulFlags &= (*saFactor[i].lpulFlagsSlave | ~FLAG_COMPARATOR_STATE);

			if(saFactor[i].ulFlags & FLAG_COMPARATOR_AND)
				continue;
			
			if(saFactor[i].ulFlags & FLAG_COMPARATOR_STATE)
			{
				saFactor[i].ulCurrentDelay++;
				if(saFactor[i].ulCurrentDelay >= saFactor[i].ulDelay)
				{
					saFactor[i].ulFlags |= FLAG_FACTOR_STATE;
/////////////////////////////////////////////////////////////////////				
#ifndef _DEBUG
					if((saFactor[i].ulCurrentDelay) == saFactor[i].ulDelay)
					{
						if(saFactor[i].ulFlags & FLAG_DIGITAL_USE)
						{
							sprintf(cDBG, "%d DC взведен %04X\n", (int)time(NULL), saFactor[i].usDigitalValues);
						}
						else
						{
							sprintf(cDBG, "%d %s взведен %.2f\t%.2f\n", (int)time(NULL), saFactor[i].cName,
								*saFactor[i].lpdParameterValue, saFactor[i].dCompareValue);
						}
						myLog();
					}
#endif
/////////////////////////////////////////////////////////////////////				
				}
				else
				{
					saFactor[i].ulFlags &= ~FLAG_FACTOR_STATE;
					saFactor[i].ulFlags &= ~FLAG_FACTOR_IS_SAVED;
				}
			}
			else
			{
#ifndef _DEBUG
				if(saFactor[i].ulFlags & FLAG_FACTOR_STATE)
				{
					if(saFactor[i].ulFlags & FLAG_DIGITAL_USE)
					{
						sprintf(cDBG, "%d DC сброшен %04X\n", (int)time(NULL), saFactor[i].usDigitalValues);
					}
					else
					{
						sprintf(cDBG, "%d %s сброшен %.2f\t%.2f\n", (int)time(NULL), saFactor[i].cName,
							*saFactor[i].lpdParameterValue, saFactor[i].dCompareValueWithHysteresis);
					}
					myLog();
				}
#endif
				saFactor[i].ulFlags &= ~FLAG_FACTOR_STATE;
				saFactor[i].ulFlags &= ~FLAG_FACTOR_IS_SAVED;
			}
		}
	}
}

/*QQQ
void* Record(void* lpData)
{
	long lCurrDataPos, lPrevDataPos, i, j, k;
	short*	lpsWriteBuffer;
    FILE* lpFile = NULL;

    char	cFname[128];
	unsigned short	shShortCount, shShortNum;
    struct timespec spec;
    time_t	seconds;
	short	shData;
	short	shTime;
	short	shAdd;
    struct	tm tm;
	char*	pcData;
    unsigned long            ms; // Milliseconds
//qqq
	unsigned short sLocalASign;
	struct strctFactorHeader strctRFH;
	
    clock_gettime(CLOCK_REALTIME, &spec);
    seconds = time (NULL);
	tm = *localtime(&seconds);
	
	shShortCount = *((unsigned short*)(strctFH.pcData+0x38));//sNumPrc
	shShortNum = *((unsigned short*)(strctFH.pcData+0x1fc));//sNumReg
	sprintf(cFname, "/home/boris/ftpfolder/record/R35A-%04X-%02d-%02d-%02d-%02d.record", shShortNum,
			tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);

    lpFile = fopen(cFname, "wb");
    if(lpFile == NULL)
    {
		bcm2835_gpio_write(PIN_LED_FAILURE, 1);
#ifdef _DEBUG
		printf("WriteRecorder. Open %s file error\n", C_FNAME_HEADER);
#endif
		sprintf(cDBG, "%d WriteRecorder. Open %s file error\n", (int)time(NULL), C_FNAME_HEADER);
		myLog();
		return (void*)-1;
    }

	
	strctRFH.pcData = NULL;
	strctRFH.stLength = strctFH.stLength;
	
	strctRFH.pcData = (char*)malloc(strctRFH.stLength);
	if(strctRFH.pcData == NULL)
	{
		bcm2835_gpio_write(PIN_LED_FAILURE, 1);
#ifdef _DEBUG
		printf("WriteRecorder. malloc error %d\n", strctRFH.stLength);
#endif
		sprintf(cDBG, "%d WriteRecorder. malloc error %d\n", (int)time(NULL), strctRFH.stLength);
		myLog();
		return (void*)-2;
	}
	
	memcpy(strctRFH.pcData, strctFH.pcData, strctFH.stLength);
	
	shAdd = (tm.tm_sec & 1) ? 50 : 0;
	
//	ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
	ms = spec.tv_nsec / 100000; // Convert nanoseconds to milliseconds
//qqq	shAdd  += (short)(ms * 18 / 100);
	shAdd  += (short)(ms / 50);
	
	shData = (short)tm.tm_year - 2000;//1980;
	shData <<= 4;
	shData += (short)(tm.tm_mon+1);
	shData <<= 5;
	shData += (short)tm.tm_mday;
	
	shTime = tm.tm_hour;
	shTime <<= 6;
	shTime += tm.tm_min;
	shTime <<= 5;
	tm.tm_sec >>= 1;
	shTime += tm.tm_sec;
	
	
	*((short*)(strctRFH.pcData+0x20)) = shData;//sDate
	*((short*)(strctRFH.pcData+0x22)) = shTime;//sTime
	*((short*)(strctRFH.pcData+0x24)) = shAdd;//sUTime
	*((short*)(strctRFH.pcData+0x26)) = 50;//sFreq
	*((short*)(strctRFH.pcData+0x54)) = 0;//
	*((short*)(strctRFH.pcData+0x56)) = 0;//
	*((unsigned char*)(strctRFH.pcData+0x58)) = 0;//

	//sLocalASign
	sLocalASign = (unsigned short)(*((unsigned char*)(strctRFH.pcData+0x4e)));
	
//sDtaLen
	*((unsigned long*)(strctRFH.pcData+0x42)) = ((unsigned long)sLocalASign) * I_RECORDER_COUNT / 2 * sizeof(short);
	//sLoPi
	*((unsigned long*)(strctRFH.pcData+0x46)) = I_RECORDER_COUNT / 2;
	//sLoPro
	*((unsigned long*)(strctRFH.pcData+0x4a)) = 0;
	
	fwrite(strctRFH.pcData, strctRFH.stLength, 1, lpFile);
	
	lpsWriteBuffer = (short*)lpData;
	fwrite(lpsWriteBuffer, I_RECORDER_FIFO_BUFFER_COUNT / 2 * sizeof(short) , 1, lpFile);
	
	free(strctRFH.pcData);
	fflush(lpFile);
    fclose(lpFile);
	
    return 0;
}

void Recorder(short* lpsData)
{
	pthread_attr_init(&attrRecord);
	pthread_attr_setdetachstate(&attrRecord, PTHREAD_CREATE_DETACHED);

	pthread_create(&threadRecord, &attrRecord, Record, lpsData);
	pthread_attr_destroy(&attrRecord);
}
*/
