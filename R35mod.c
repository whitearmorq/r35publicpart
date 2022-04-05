#include <linux/kallsyms.h> 
#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h> 
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/spi/spi.h>
#include <linux/sched.h>

#include "R35mod.h"

short bufferR[ONE_MEASUREMENT_SIZE+2];
short bufferW[ONE_MEASUREMENT_SIZE+2] = {0x0000, 0x0000, 0x0000, 0x0000, 0xf0f0, 0x0000, 0x0000, 0xf0f0,
											 0xf0f0, 0xf0f0, 0x0000, 0xf0f0, 0x0000, 0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0};

short* pointDataR;
short* lpusCT;
short* pointR;

struct mfuncs
{
    char *fname;
    void *faddr;
};

static struct mfuncs func_list[2] = {
    {"sys_msgget", 0},
    {"sys_msgsnd", 0}
};

static short usCT = 0;

static struct gpio gpFailureOut		= {5,	GPIOF_OUT_INIT_HIGH, "Failure Out"};
static struct gpio gpResetOut		= {22,	GPIOF_OUT_INIT_LOW, "Reset Out"};
static struct gpio gpInterruptIn	= {27,	GPIOF_IN, "Interrupt In"};

static int irq_num;

struct spi_master *master = NULL;
struct spi_device *spi_device = NULL;
struct spi_transfer	t;
struct spi_message	m;

static short usHalfCT = 0;

int iErrorCounter = 0;

//shm
int shmid				= 0;
//short* msg_p	= NULL;
int mq_id				= 0;

asmlinkage long (*sys_msgget)(key_t key, int msgflg);
asmlinkage long (*sys_msgsnd)(int msqid, struct msgbuf __user *msgp, size_t msgsz, int msgflg);


struct task_struct* pThreadR35 = NULL;

int init_msgq( void )
{
    
    sys_msgget = func_list[0].faddr;   
    sys_msgsnd = func_list[1].faddr;

    //Создаю очередь сообщений
    if((mq_id = sys_msgget(ipckey, IPC_CREAT|IPC_ACCESS)) < 0)
    {
        printk("server: can't create mq_id\n");
    }
    return 0;
}

int send_mess(struct msgbuf* lpBuf)
{
	int ret;
	ret = sys_msgsnd(mq_id, lpBuf, sizeof(mymsg[0].data), IPC_NOWAIT);
    if(ret)
	{
		printk("server: %d\n", ret);
		return 0;
	}
	
	return 1;
}

int shm_f( void* data, const char* sym, struct module* mod, unsigned long addr )
{ 
    struct mfuncs *mf  = (struct mfuncs*)data;
    if(!strcmp( mf->fname, sym )) 
    { 
        mf->faddr = (void*)addr; 
        return 1; 
    } 
    return 0; 
}; 

int get_kallsyms( void ) 
{
    int i;
    for(i = 0; i < 2; i++)
    {
        int n = kallsyms_on_each_symbol( shm_f, (void*)&func_list[i] ); 
        if( n != 0 ) 
        {
            printk("func %s is OK\n", func_list[i].fname);   
        }
        else
        {
            printk("func %s not found\n", func_list[i].fname);   
        }
    }
    return 0;
}

//static int iTMP = 0;
////////////////////////////////////////////////////////////
irqreturn_t gpio_isr(int irq, void *data)
{
//	gpio_set_value(gpControlOut.gpio, 1);
	if(iErrorCounter >= 500)
	{
		return IRQ_HANDLED;
	}
	
	spi_sync(spi_device, &m);
	usCT++;
	if(usCT >= MAX_COUNTER_VALUE)
		usCT = 0;

	if(usCT == *lpusCT)
	{
		memcpy(pointR, pointDataR, ONE_MEASUREMENT_SIZE*sizeof(short));
		pointR += ONE_MEASUREMENT_SIZE;
	
		usHalfCT++;
		if(usHalfCT == HALF_PERIOD_COUNT)
		{
			unnStatus.bits.m_MSGCounter++;
			pointR = mymsg[unnStatus.bits.m_MSGCounter].data;
			usHalfCT = 0;
		}
	}
	else
		gpio_set_value(gpResetOut.gpio, 0);
		
	
	raise_softirq(RECON_SOFTIRQ);
    return IRQ_HANDLED;
}

void gpio_proc_soft(struct softirq_action* lpData)
{
	if(usCT != *lpusCT)
	{
//		printk(KERN_ERR"softCT %d, 32CT %d -> ", usCT, *lpusCT);
		usCT = 0;
//		*lpusCT = 0;
		usHalfCT = 0;
		pointR = mymsg[unnStatus.bits.m_MSGCounter].data;
		gpio_set_value(gpResetOut.gpio, 1);
	}

//	gpio_set_value(gpControlOut.gpio, 0);
}

#include "R35Thread.c"
static int __init ksys_init(void)
{
    int ret;

	pointDataR = bufferR+2;
	lpusCT = pointDataR + ADC_MEASUREMENT_COUNT + 1;
	unnStatus.m_ulFlags = 0;
	unnStatus.bits.m_FirstTime = 1;
	pointR = mymsg[unnStatus.bits.m_MSGCounter].data;

	
    get_kallsyms();
    init_msgq();

////////////////////////////////////////////////////////////////////////////	
	ret = gpio_request(gpFailureOut.gpio, gpFailureOut.label);
	if (ret)
	{
		printk(KERN_ALERT "Failure Gpio request failed");
		return -1;
	}
	gpio_direction_output(gpFailureOut.gpio, 0);
	gpio_set_value(gpFailureOut.gpio, 0);

	ret = gpio_request(gpResetOut.gpio, gpResetOut.label);
	if (ret)
	{
		printk(KERN_ALERT "Reset Gpio request failed");
		gpio_set_value(gpFailureOut.gpio, 1);
		return -1;
	}
	gpio_direction_output(gpResetOut.gpio, 0);
	gpio_set_value(gpResetOut.gpio, 0);
	
    ret = gpio_request(gpInterruptIn.gpio,gpInterruptIn.label);
    if (ret)
    {
        printk(KERN_ALERT "In Gpio request failed");
		gpio_set_value(gpFailureOut.gpio, 1);
        return -2;
    }

    ret = gpio_to_irq(gpInterruptIn.gpio);
    if(ret < 0)
    {
        printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
		gpio_set_value(gpFailureOut.gpio, 1);
        return -3;
    }
    irq_num = ret;
    printk(KERN_INFO "Successfully requested IRQ#%d.\n", irq_num);

    open_softirq(RECON_SOFTIRQ, gpio_proc_soft);

	master = spi_busnum_to_master(0);
	if(!master )
	{
		printk(KERN_ERR"----------------SPI Master not found\n");
		gpio_set_value(gpFailureOut.gpio, 1);
		return -ENODEV;
	}

//	master->num_chipselect = 1;
	
	spi_device = spi_alloc_device(master);
	if(!spi_device )
	{
		printk(KERN_ERR"----------------SPI Failed to alloc\n");
		gpio_set_value(gpFailureOut.gpio, 1);
		return -ENODEV;
	}
	 
	spi_device->bits_per_word = 8;
	spi_device->mode = SPI_MODE_0;
	spi_device->max_speed_hz = 24000000;
	spi_device->chip_select = 0;
	spi_device->cs_gpio = 8;
	


	t.tx_buf	= bufferW;
	t.rx_buf	= bufferR;
	t.len		= sizeof(bufferW);
	t.cs_change = 1;//qqq->0
	
	spi_message_init(&m);
	m.is_dma_mapped = 1;
	master->min_speed_hz = 24000000;
	master->max_speed_hz = 24000000;
    printk(KERN_NOTICE "min_speed_hz %d\n", master->min_speed_hz);
	spi_message_add_tail(&t, &m);
	
	InitThreadR35Handler(NULL);
	
	if(pThreadR35 == NULL)
	{
        printk(KERN_ERR "kthread_run error\n");
		gpio_set_value(gpFailureOut.gpio, 1);
		return -5;
	}
	
    //IRQF_TRIGGER_RISING, IRQF_TRIGGER_FALLING
	gpio_set_value(gpResetOut.gpio, 1);
    if(request_irq(irq_num, gpio_isr, IRQF_TRIGGER_RISING, "gpio_irq", NULL))
    {
        printk(KERN_ERR "Unable to request irq\n");
		gpio_set_value(gpFailureOut.gpio, 1);
        return -4;
    }

    

    printk(KERN_NOTICE "R35mod loaded.\n");
    return 0;
} 

static void __exit ksys_exit(void)
{
	if(pThreadR35)
		kthread_stop(pThreadR35);
    free_irq(irq_num,NULL);
	
    gpio_free(gpInterruptIn.gpio);
	gpio_free(gpFailureOut.gpio);
	gpio_free(gpResetOut.gpio);

	spi_dev_put(spi_device);
    printk(KERN_NOTICE "R35mod unloaded.\n");
}

////////////////////////////////////////////////////////
module_init( ksys_init ); 
module_exit( ksys_exit );
MODULE_LICENSE( "GPL" ); 
MODULE_AUTHOR( "S" );
