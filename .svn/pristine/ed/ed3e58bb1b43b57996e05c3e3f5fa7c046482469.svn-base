#include "aviwriter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include <plog/Log.h>
#include "avi.h"
void 
fwrite_DWORD(FILE * file_ptr, DWORD word){
	unsigned char * p;

	p = (unsigned char *)&word;
	int i;
	for(i = 0; i<4;i++){
		fputc(p[i],file_ptr);
	}

}

void 
fwrite_WORD(FILE * file_ptr, WORD word){
	unsigned char * p;

	p = (unsigned char*)&word;
	int i;
	for(i = 0; i<2;i++){
		fputc(p[i],file_ptr);
	}

}
CAVIWriter::CAVIWriter()
{

}

CAVIWriter::~CAVIWriter()
{
    
}

void CAVIWriter::Open(const char* filepath,int fps,int width,int height,int totalframe)
{
    m_AVIHandle = fopen(filepath,"w");
    RIFF RIFF_LIST;
    total = totalframe;
    nframe = 0;
    length = new unsigned long[total];
	RIFF_LIST.dwRIFF = 'RIFF';
	LOG_INFO << "save avi file fps=" << fps << " W=" << width << " H=" << height << " TotalFrame=" << totalframe;
	int cc = 'R';
	fputc('R',m_AVIHandle); 
	fputc('I',m_AVIHandle); 
	fputc('F',m_AVIHandle); 
	fputc('F',m_AVIHandle); 

	RIFF_LIST.dwSize =  150 + 12 + 8*totalframe + 8 + 4*4*totalframe;
    sizeSeek = ftell(m_AVIHandle);
	fwrite_DWORD(m_AVIHandle, 0);

	RIFF_LIST.dwFourCC = 'AVI ';
	fputc('A',m_AVIHandle);
	fputc('V',m_AVIHandle);
	fputc('I',m_AVIHandle);
	fputc(' ',m_AVIHandle);
	// 	RIFF_LIST.data = WAIT WITH THIS

	LIST hdrl;
	hdrl.dwList = 'LIST';
	fputc('L',m_AVIHandle);
	fputc('I',m_AVIHandle);
	fputc('S',m_AVIHandle);
	fputc('T',m_AVIHandle);
	hdrl.dwSize = 208;
	fwrite_DWORD(m_AVIHandle, hdrl.dwSize);
	hdrl.dwFourCC = 'hdrl';
	fputc('h',m_AVIHandle);
	fputc('d',m_AVIHandle);
	fputc('r',m_AVIHandle);
	fputc('l',m_AVIHandle);

	MainAVIHeader avih;

	avih.dwFourCC = 'avih';
	fputc('a',m_AVIHandle);
	fputc('v',m_AVIHandle);
	fputc('i',m_AVIHandle);
	fputc('h',m_AVIHandle);
	avih.dwSize = 56;
	fwrite_DWORD(m_AVIHandle, avih.dwSize);

	avih.dwMicroSecPerFrame = 1000000/fps;
	fwrite_DWORD(m_AVIHandle, avih.dwMicroSecPerFrame);

	avih.dwMaxBytesPerSec = 7000;
	fwrite_DWORD(m_AVIHandle, avih.dwMaxBytesPerSec);

	avih.dwPaddingGranularity = 0;
	fwrite_DWORD(m_AVIHandle, avih.dwPaddingGranularity);

	// dwFlags set to 16, do not know why! 
	avih.dwFlags = 16;
	fwrite_DWORD(m_AVIHandle, avih.dwFlags);

	avih.dwTotalFrames = totalframe;
	fwrite_DWORD(m_AVIHandle, avih.dwTotalFrames);

	avih.dwInitialFrames = 0;
	fwrite_DWORD(m_AVIHandle, avih.dwInitialFrames);

	avih.dwStreams = 1;
	fwrite_DWORD(m_AVIHandle, avih.dwStreams);

	avih.dwSuggestedBufferSize = 0;
	fwrite_DWORD(m_AVIHandle, avih.dwSuggestedBufferSize);

	avih.dwWidth = width;
	fwrite_DWORD(m_AVIHandle, avih.dwWidth);

	avih.dwHeight = height;
	fwrite_DWORD(m_AVIHandle, avih.dwHeight);

	avih.dwReserved[0] = 0;
	fwrite_DWORD(m_AVIHandle, avih.dwReserved[0]);
	avih.dwReserved[1] = 0;
	fwrite_DWORD(m_AVIHandle, avih.dwReserved[1]);
	avih.dwReserved[2] = 0;
	fwrite_DWORD(m_AVIHandle, avih.dwReserved[2]);
	avih.dwReserved[3] = 0;
	fwrite_DWORD(m_AVIHandle, avih.dwReserved[3]);

	LIST strl;
	strl.dwList = 'LIST';
	fputc('L',m_AVIHandle); 
	fputc('I',m_AVIHandle); 
	fputc('S',m_AVIHandle); 
	fputc('T',m_AVIHandle); 
	strl.dwSize = 132;
	fwrite_DWORD(m_AVIHandle, strl.dwSize);

	strl.dwFourCC = 'strl';
	fputc('s',m_AVIHandle); 
	fputc('t',m_AVIHandle); 
	fputc('r',m_AVIHandle); 
	fputc('l',m_AVIHandle); 

	AVIStreamHeader strh;
	strh.dwFourCC = 'strh';
	fputc('s',m_AVIHandle); 
	fputc('t',m_AVIHandle); 
	fputc('r',m_AVIHandle); 
	fputc('h',m_AVIHandle); 

	strh.dwSize = 48;
	fwrite_DWORD(m_AVIHandle, strh.dwSize);
	strh.fccType = 'vids';
	fputc('v',m_AVIHandle); 
	fputc('i',m_AVIHandle); 
	fputc('d',m_AVIHandle); 
	fputc('s',m_AVIHandle); 
	strh.fccHandler = 'MJPG';
	fputc('M',m_AVIHandle); 
	fputc('J',m_AVIHandle); 
	fputc('P',m_AVIHandle); 
	fputc('G',m_AVIHandle); 
	strh.dwFlags = 0; 
	fwrite_DWORD(m_AVIHandle, strh.dwFlags);
	strh.wPriority = 0; // +2 = 14
	fwrite_WORD(m_AVIHandle, strh.wPriority);
	strh.wLanguage = 0; // +2 = 16
	fwrite_WORD(m_AVIHandle, strh.wLanguage);
	strh.dwInitialFrames = 0; // +4 = 20
	fwrite_DWORD(m_AVIHandle, strh.dwInitialFrames);
	strh.dwScale = 1; // +4 = 24
	fwrite_DWORD(m_AVIHandle, strh.dwScale);
	// insert FPS
	strh.dwRate = fps; // +4 = 28
	fwrite_DWORD(m_AVIHandle, strh.dwRate);
	strh.dwStart = 0; // +4 = 32
	fwrite_DWORD(m_AVIHandle, strh.dwStart);
	// insert nbr of jpegs
	strh.dwLength = totalframe; // +4 = 36
	fwrite_DWORD(m_AVIHandle, strh.dwLength);

	strh.dwSuggestedBufferSize = 0; // +4 = 40
	fwrite_DWORD(m_AVIHandle, strh.dwSuggestedBufferSize);
	strh.dwQuality = 0; // +4 = 44
	fwrite_DWORD(m_AVIHandle, strh.dwQuality);
	// Specifies the size of a single sample of data.
	// This is set to zero if the samples can vary in size.
	// If this number is nonzero, then multiple samples of data
	// can be grouped into a single chunk within the file.
	// If it is zero, each sample of data (such as a video frame) must be in a separate chunk.
	// For video streams, this number is typically zero, although
	// it can be nonzero if all video frames are the same size.
	//
	strh.dwSampleSize = 0; // +4 = 48
	fwrite_DWORD(m_AVIHandle, strh.dwSampleSize);

	EXBMINFOHEADER strf;

	strf.dwFourCC = 'strf';
	fputc('s',m_AVIHandle); 
	fputc('t',m_AVIHandle); 
	fputc('r',m_AVIHandle); 
	fputc('f',m_AVIHandle); 
	strf.dwSize = 40;
	fwrite_DWORD(m_AVIHandle, strf.dwSize);

	strf.biSize = 40;
	fwrite_DWORD(m_AVIHandle, strf.biSize);

	strf.biWidth = width;
	fwrite_DWORD(m_AVIHandle, strf.biWidth);
	strf.biHeight = height;
	fwrite_DWORD(m_AVIHandle, strf.biHeight);
	strf.biPlanes = 1;
	fwrite_WORD(m_AVIHandle, strf.biPlanes);
	strf.biBitCount = 24;
	fwrite_WORD(m_AVIHandle, strf.biBitCount);
	strf.biCompression = 'MJPG';
	fputc('M',m_AVIHandle); 
	fputc('J',m_AVIHandle); 
	fputc('P',m_AVIHandle); 
	fputc('G',m_AVIHandle); 

	strf.biSizeImage = ((strf.biWidth*strf.biBitCount/8 + 3)&0xFFFFFFFC)*strf.biHeight;
	fwrite_DWORD(m_AVIHandle, strf.biSizeImage);
	strf.biXPelsPerMeter = 0;
	fwrite_DWORD(m_AVIHandle, strf.biXPelsPerMeter);
	strf.biYPelsPerMeter = 0;
	fwrite_DWORD(m_AVIHandle, strf.biYPelsPerMeter);
	strf.biClrUsed = 0;
	fwrite_DWORD(m_AVIHandle, strf.biClrUsed);
	strf.biClrImportant = 0;
	fwrite_DWORD(m_AVIHandle, strf.biClrImportant);

	fputc('L',m_AVIHandle); 
	fputc('I',m_AVIHandle); 
	fputc('S',m_AVIHandle); 
	fputc('T',m_AVIHandle); 

	DWORD ddww = 16;
	fwrite_DWORD(m_AVIHandle, ddww);
	fputc('o',m_AVIHandle); 
	fputc('d',m_AVIHandle); 
	fputc('m',m_AVIHandle); 
	fputc('l',m_AVIHandle); 

	fputc('d',m_AVIHandle); 
	fputc('m',m_AVIHandle); 
	fputc('l',m_AVIHandle); 
	fputc('h',m_AVIHandle); 

	DWORD szs = 4;
	fwrite_DWORD(m_AVIHandle, szs);

	// nbr of jpgs
	DWORD totalframes = totalframe;
	fwrite_DWORD(m_AVIHandle, totalframes);

	LIST movi;
	movi.dwList = 'LIST';
	fputc('L',m_AVIHandle); 
	fputc('I',m_AVIHandle); 
	fputc('S',m_AVIHandle); 
	fputc('T',m_AVIHandle);

	movi.dwSize = 4 + 8*totalframe;
    moviSeek = ftell(m_AVIHandle);
	fwrite_DWORD(m_AVIHandle, 0);
	movi.dwFourCC = 'movi';
	fputc('m',m_AVIHandle);
	fputc('o',m_AVIHandle);
	fputc('v',m_AVIHandle);
	fputc('i',m_AVIHandle);
}

void CAVIWriter::Finish()
{
	LOG_DEBUG << "close file process start";
    fputc('i', m_AVIHandle); fputc('d', m_AVIHandle); fputc('x', m_AVIHandle); fputc('1', m_AVIHandle);
	unsigned long index_length = 4*4*total;
	fwrite_DWORD(m_AVIHandle, index_length);
	LOG_DEBUG << "write avi length";
	unsigned long AVI_KEYFRAME = 16;

	unsigned long offset_count = 4;

    for(unsigned long l = 0;l<total;l++)
    {
        fputc('0', m_AVIHandle); fputc('0', m_AVIHandle); fputc('d', m_AVIHandle); fputc('b', m_AVIHandle);
		fwrite_DWORD(m_AVIHandle, AVI_KEYFRAME);
		fwrite_DWORD(m_AVIHandle, offset_count);
		fwrite_DWORD(m_AVIHandle, length[l]);
		offset_count+=length[l]+8;
    }
	LOG_DEBUG << "write avi frame param finish";
    fseek(m_AVIHandle,sizeSeek,SEEK_SET);
    fwrite_DWORD(m_AVIHandle, 50 + 12 + offset_count + 8 + 4*4*total);
    fseek(m_AVIHandle,moviSeek,SEEK_SET);
    fwrite_DWORD(m_AVIHandle,offset_count + 4);
    fseek(m_AVIHandle,0,SEEK_END);
	LOG_DEBUG << "write avi header adjust";
    fflush(m_AVIHandle);
    fclose(m_AVIHandle);
    printf("close file\n");
}

void CAVIWriter::WriteSegment(unsigned char* buffer,unsigned int size)
{
    unsigned long* tmp = (unsigned long*)buffer;
    unsigned char* write = buffer + (tmp[0]+1)*sizeof(unsigned long);
    for(unsigned long i = 0;i<tmp[0];i++)
    {
        length[nframe] = tmp[i+1]+1;
        //printf("frame : %5d length : %7d\n",nframe+1,length[nframe]);
        nframe++;
    }
    fwrite(write,1,size-(tmp[0]+1)*sizeof(unsigned long),m_AVIHandle);
    fflush(m_AVIHandle);
}

void CAVIWriter::SaveFrame(unsigned char* buffer,unsigned long size,int frame,FILE* fp)
{
	fseek(fp,frame*sizeof(unsigned long),SEEK_SET);
    fwrite(&size,sizeof(unsigned long),1,fp);
    fseek(fp,zeroSeek,SEEK_SET);
    fputc('0',fp);
    fputc('0',fp);
    fputc('d',fp);
    fputc('b',fp);
    fwrite(&size,sizeof(unsigned long),1,fp);
    fwrite(buffer, 1, size, fp);
    zeroSeek = ftell(fp);
    fflush(fp);

}

void CAVIWriter::SaveHead(unsigned long fps,FILE* fp)
{
	unsigned long *tmp = new unsigned long[fps+1];
    memset(tmp,0,(fps+1)*sizeof(unsigned long));
    tmp[0]=fps;
    fwrite(tmp,sizeof(unsigned long),fps,fp);
    zeroSeek = ftell(fp);
	delete[] tmp;
}