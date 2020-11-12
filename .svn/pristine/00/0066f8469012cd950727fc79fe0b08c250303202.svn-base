#include "datatype.h"
#include "gpio.h"
#define BCM2708_PERI_BASE 0x20000000
#define GPIO_BASE (BCM2708_PERI_BASE + 0x200000)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

int mem_fd;
char *gpio_mem, *gpio_map;
char *spi0_mem, *spi0_map;

volatile unsigned *gpio;


#define INP_GPIO(g) *(gpio + ((g) / 10)) &= ~(7<<(((g) % 10 ) *3))
#define OUT_GPIO(g) *(gpio + ((g) / 10)) |= (1<<(((g) % 10 ) *3))
#define SET_GPIO_ALT(g,a) *(gpio + (((g) / 10))) |= (((a) < 3? (a) + 4: (a) == 4?3:2 << ((g) % 10 ) *3))

#define GPIO_SET * (gpio+7)
#define GPIO_CLR * (gpio+10)
#define GPIO_GET * (gpio+13)
#define MAX_PORTNUM 25

char valid_port[] = 
{
	1,1,0,0,
	0,0,0,1,
	1,1,1,1,
	0,0,1,1,
	0,1,1,0,
	0,1,1,1,
	1,1
};

int port_avail(int port)
{
	if((port < 0) || (port > MAX_PORTNUM))
		return(0);
	return ((int)valid_port[port]);
}

int gpio_read(int port)
{
	if(!port_avail(port))
		return(0);
	return((GPIO_GET & (1<<port)) ?1:0);
}

void gpio_write(int port,int data)
{
	if(!port_avail(port))
		return;
	if(data==0)
		GPIO_CLR = 1<<port;
	else
		GPIO_SET = 1 << port;
}

void testmain()
{
	int p;
	for(p=7;p<=11;p++){
		gpio_write(p,1);
		sleep(1);
	}
	for(p=0;p<2;p++){
		printf("%d:",gpio_read(p));
		}
	printf("\n");
	for(p=7;p<=11;p++){
		gpio_write(p,0);
		sleep(1);
	}
	for(p=0;p<2;p++){
		printf("%d:",gpio_read(p));

	}
	printf("\n");
}

CIOPort::CIOPort()
{
    memory_fd = 0;
    gpio_base = 0;
    map = NULL;
    addr = NULL;
}

CIOPort::~CIOPort()
{
}

int CIOPort::InitIOMap()
{
    if((memory_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
		printf("can't open /dev/mem \n");
	    exit(-1);
    }

    if((gpio_mem = malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL) {
	    printf("allocation error \n");;
	    exit(-1);
    }

    if((unsigned long) gpio_mem % PAGE_SIZE)
	    gpio_mem += PAGE_SIZE - ((unsigned long)gpio_mem % PAGE_SIZE);

        map = (char *)mmap (
	        (caddr_t)gpio_mem,
	        BLOCK_SIZE,
	        PROT_READ | PROT_WRITE,
	        MAP_SHARED | MAP_FIXED,
	        memory_fd,
	        GPIO_BASE
        );

    if((long)map < 0){
	    printf("mmap error %d\n",(int)map);
	    exit(-1);
    }

    addr = (volatile unsigned *)map;
    return 1;
}

void CIOPort::UnInitIOMap()
{
    munmap(map, BLOCK_SIZE);
    close(memory_fd);
}

void CIOPort::SetReg(int offset,unsigned int reg)
{
    *(addr+offset) = reg;
    printf("SetReg : %2d , %08x\n",offset,*(addr+offset));
}

void CIOPort::GetReg(int offset,unsigned int& reg)
{
    reg = *(addr+offset);
    printf("GetReg : %2d , %08x\n",offset,*(addr+offset));
}

void CIOPort::SetPinMode(int port,unsigned int reg)
{
    int offset = port/10;
    int p = port%10;
    unsigned int tmp = *(addr+offset);
    unsigned int mask = ~(0x7 << (3*p));
    *(addr+offset) = (tmp&mask) | ((0x7&reg) << (3*p));
    printf("SetPinMode : %2d, %08x\n",offset,*(addr+offset));
}

void CIOPort::Output(int port)
{
    int offset = GPSET0;
    int p = port;
    if(port >= 32)
    {
        offset = GPSET1;
        p -= 32;
    }
    unsigned int tmp = *(addr+offset);
    unsigned int mask = ~(0x1 << p);
    *(addr+offset) = (tmp&mask) | (0x1 << p);
    printf("Output : %08x\n",*(addr+offset));
}

void CIOPort::Clear(int port)
{
    int offset = GPCLR0;
    int p = port;
    if(port >= 32)
    {
        offset = GPCLR1;
        p -= 32;
    }
    unsigned int tmp = *(addr+offset);
    unsigned int mask = ~(0x1 << p);
    *(addr+offset) = (tmp&mask) | (0x1 << p);
    printf("Clear : %08x\n",*(addr+offset));
}

int CIOPort::Input(int port)
{
    int offset = GPLEV0;
    int p = port;
    unsigned int ret = 0;
    if(port >= 32)
    {
        offset = GPLEV1;
        p -= 32;
    }
    unsigned int mask = 0x1 << p;
    ret = *(addr+offset)&mask;
    printf("Input : %08x\n",*(addr+offset));
    return ret == mask;
}
