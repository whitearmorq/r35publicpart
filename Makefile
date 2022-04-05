all: R35Aclient R35mod R35msg

R35msg: R35msg.o 
	gcc -o R35msg R35msg.o 

R35msg.o: R35msg.c R35Aclient.h
	gcc -c R35msg.c -std=gnu99

R35Aclient: R35Aclient.o R35AclientFactors.o R35AMainHandler.o
	gcc -o R35Aclient R35Aclient.o R35AclientFactors.o R35AMainHandler.o -lm -l bcm2835 -l pthread

R35Aclient.o: R35Aclient.c R35common.h
	gcc -c R35Aclient.c -std=gnu99

R35AclientFactors.o: R35AclientFactors.c R35common.h
	gcc -c R35AclientFactors.c -std=gnu99 

R35AMainHandler.o: R35AMainHandler.c R35common.h
	gcc -c R35AMainHandler.c -std=gnu99 

obj-m += R35mod.o 

R35mod:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
