#pragma once
#include <stdio.h>

class CAVIWriter
{
    public:
        CAVIWriter();
        ~CAVIWriter();
        void Open(const char* filepath,int fps,int wight,int height,int totalframe);
        void Finish();
        void WriteSegment(unsigned char* buffer,unsigned int size);
        void SaveFrame(unsigned char* buffer,unsigned long size,int frame,FILE* fp);
        void SaveHead(unsigned long fps,FILE* fp);
        FILE* m_AVIHandle;
        unsigned long total;
        unsigned long nframe;
        unsigned long* length;
        unsigned long moviSeek;
        unsigned long sizeSeek;
        unsigned long zeroSeek;
};