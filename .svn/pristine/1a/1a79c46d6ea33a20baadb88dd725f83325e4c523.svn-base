#include <wiringPi.h>
#include "wgpio.h"

WIOPort::WIOPort()
{
    wiringPiSetup();
}

WIOPort::~WIOPort()
{

}

void WIOPort::PinMode(int port,int direction)
{
    switch(direction){
      case GPIO_IN:
        pinMode(port,INPUT);
        break;
      case GPIO_OUT:
        pinMode(port,OUTPUT);
        break;
    }
}

int WIOPort::ReadPort(int port)
{
    return digitalRead(port);
}

void WIOPort::WritePort(int port,int data)
{
    digitalWrite(port,data);
}