#include <string.h>
#include <string.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <time.h>

const char C_FNAME_HEADER[] = "/home/boris/heap.r35set";
const char cLngPost[]	= "LngPost";
const char cLngPred[]	= "LngPred";

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
	
    lpFile = fopen (C_FNAME_HEADER, "rb");//!!!
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
			if(*lpulPostSize < 500)
			{
				*((long*)(lpHeader + sOffset)) = 500;
				*lpulPostSize = 500;
			}
			else
			{
				if(*lpulPostSize > 6000)
				{
					*((long*)(lpHeader + sOffset)) = 6000;
					*lpulPostSize = 6000;
				}
				else
					*((long*)(lpHeader + sOffset)) = *lpulPostSize;
			}
		}
			
		if((ulLexemMask & 0x00000004) && !strncmp(lpTMP, cLngPred, 7))
		{
			ulLexemMask &= 0xfffffffb;
			sOffset = *((short*)(lpTMP + 9));
			sOffset += *((short*)(lpHeader + 0x14));//HeaderLong
			
			*lpulPrevSize /= 10;
			*lpulPrevSize *= 10;
			if(*lpulPrevSize < 250)
			{
				*((long*)(lpHeader + sOffset)) = 250;
				*lpulPrevSize = 250;
			}
			else
			{
				if(*lpulPrevSize > 3000)
				{
					*((long*)(lpHeader + sOffset)) = 3000;
					*lpulPrevSize = 3000;
				}
				else
					*((long*)(lpHeader + sOffset)) = *lpulPrevSize;
			}
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
//	fwrite(lpHeader, stLen, 1, lpFile);
	
    fclose(lpFile);
	return 0;
}
