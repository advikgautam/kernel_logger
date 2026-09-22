obj-m += logger.o

all:
	make LLVM=1 -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make LLVM=1 -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
