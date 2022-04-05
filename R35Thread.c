//#include "R35mod.h"


static int ThreadR35Handler(void* data)
{
	int iPosMSG = unnStatus.bits.m_MSGCounter;
	printk(KERN_NOTICE "ThreadR35Handler, %p\n", data);

    while(!kthread_should_stop())
	{
		if(iPosMSG != unnStatus.bits.m_MSGCounter)
		{
//			unnStatus.bits.m_FirstTime = 0;
//			send_mess(&mymsg);
			if(send_mess(&mymsg[iPosMSG]))
			{
				iErrorCounter = 0;
			}
			else
			{
				iErrorCounter++;
			}
				
			iPosMSG = unnStatus.bits.m_MSGCounter;
		}
	}
   return 0;
}

void InitThreadR35Handler(void* data)
{
	pThreadR35 = kthread_run(ThreadR35Handler, data, "R35Thread");
}
