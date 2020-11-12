#pragma once

//GPIO Function Select
#define GPFSEL0 0
#define GPFSEL1 1
#define GPFSEL2 2
#define GPFSEL3 3
#define GPFSEL4 4
#define GPFSEL5 5
//Output Set
#define GPSET0 7
#define GPSET1 8
//Output Clear
#define GPCLR0 10
#define GPCLR1 11
//Pin Level
#define GPLEV0 13
#define GPLEV1 14
//Pin Event Ditect
#define GPEDS0 16
#define GPEDS1 17
//Riging Edge Ditect
#define GPREN0 19
#define GPREN1 20
//Falling Edge Ditect
#define GPFEN0 22
#define GPFEN1 23
//Pin High Ditect
#define GPHEN0 25
#define GPHEN1 26
//Pin Low Ditect
#define GPLEN0 28
#define GPLEN1 29
//Async Rising Edge Ditect
#define GPAREN0 31
#define GPAREN1 32
//Async Falling Edge Ditect
#define GPAFEN0 34
#define GPAFEN1 35
//Pull Up-Down Enable
#define GPPUD 37
//GPIO Pin Pull-up/down Enable Clock
#define GPPUDCLK0 38
#define GPPUDCLK1 39

//Pin Configuration
#define IO_INPUT 0b000
#define IO_OUTPUT 0b001
#define IO_ALT0 0b100
#define IO_ALT1 0b101
#define IO_ALT2 0b110
#define IO_ALT3 0b111
#define IO_ALT4 0b011
#define IO_ALT5 0b010


class CIOPort
{
  public:
    CIOPort();
    ~CIOPort();
    unsigned long         gpio_base;
    int                   memory_fd;
    void                  *map;
    void                  *gpio_mem;
    volatile unsigned int *addr;

    int InitIOMap();
    void UnInitIOMap();
    void SetReg(int offset,unsigned int reg);
    void GetReg(int offset,unsigned int& reg);
    void SetPinMode(int port,unsigned int reg);
    void Output(int port);
    void Clear(int port);
    int Input(int port);
};