obj-m += main.o

KDIR = /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules
	gcc user_app.c -o userapp

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f userapp