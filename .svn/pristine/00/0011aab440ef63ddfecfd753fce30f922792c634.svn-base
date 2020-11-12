#pragma once

#define GPIO_IN 0
#define GPIO_OUT 1
#define GPIO_ON 1
#define GPIO_OFF 0

class WIOPort
{
  public:
    WIOPort();
    ~WIOPort();
    void PinMode(int port,int direction);
    int ReadPort(int port);
    void WritePort(int port,int data);
};