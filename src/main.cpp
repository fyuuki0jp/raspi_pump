#include <nan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <malloc.h>
#include <time.h>
#include <plog/Log.h>
#include <linux/videodev2.h>
#include "datatype.h"
#include "Event.h"
#include "cap.h"
#include "wgpio.h"
#include "aviwriter.h"
#include "Data.h"
#include "SQLite.h"

#define VERSION "1.0.0.0"
#define MOUNTPATH "./public/img/hdd/"
#define SAVEPATH "./public/img/hdd/Pictures/"
#define MODE_LIVE 1
#define MODE_CHECK 0
#define TIME_FRAME 70
class FrameLoop;
FrameLoop *control;
int fControl;
int fFirstFrame = 0;
int fTriger = 0;
pthread_t thread;
pthread_t triger;
pthread_t pool;
CCap gCap;
CEvent gTriEv;
CEvent gCapEv;
CEvent gSendEv;
CEvent gSaveEv;
unsigned char fileHeader[27];
char filelist[TIME_FRAME][256];
int lengthlist[TIME_FRAME] = {0};
int writebuffer = 0;
int readbuffer = 1;
int writestream = 0;
int readstream = 1;
unsigned char *buffer;
int jpeglength;
bool fChangeMode = false;
bool fConfUpdate = false;
bool fMountUpdate = false;
int mode = MODE_LIVE;
unsigned int h264length[10] = {0};
unsigned int h264size[10] = {0};
unsigned char* h264data[10];
struct tm* save;
int errorcode = -1;
const char* errorinfo[] = {
    "カメラが接続されていません。カメラの接続を確認してください。",
    "",
    "",
}

typedef struct _config_str_
{
    int nBeforeTime = 30;
    int nAfterTime = 10;
    int fps = 30;
    int Height = 640;
    int Width = 480;
    bool useH264 = false;
    bool isRising = true;
    bool isRec = false;
}Conf;

Conf Config;

const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

bool getMacAddress(BYTE* ret)
{
	char mac[256];
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (fd == -1)
	{
		LOG_ERROR << "socket error : " << errno;
		return false;
	}

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);

	ioctl(fd, SIOCGIFHWADDR, &ifr);

	close(fd);

	memcpy(ret, (BYTE *)ifr.ifr_hwaddr.sa_data, 6);

	sprintf(mac, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
			(unsigned char)ret[0],
			(unsigned char)ret[1],
			(unsigned char)ret[2],
			(unsigned char)ret[3],
			(unsigned char)ret[4],
			(unsigned char)ret[5]);
	LOG_DEBUG << "mac address : " << mac;

	return true;
}

void calcLicenseFromMac(BYTE *mac, UINT64 *llcode, UINT64 *lllicense)
{
	UINT64 llmac, lldat;

	llmac = ((((UINT64)mac[0]) << 40) & ((UINT64)0xff << 40));
	llmac += ((((UINT64)mac[1]) << 32) & ((UINT64)0xff << 32));
	llmac += ((((UINT64)mac[2]) << 24) & ((UINT64)0xff << 24));
	llmac += ((((UINT64)mac[3]) << 16) & ((UINT64)0xff << 16));
	llmac += ((((UINT64)mac[4]) << 8) & ((UINT64)0xff << 8));
	llmac += ((((UINT64)mac[5]) << 0) & ((UINT64)0xff << 0));
	llmac = (~llmac);
	llmac = llmac + ((((UINT64)0x12345678) << 32) + (UINT64)0x9abcdef0);
	*llcode = llmac;

	lldat = ((((UINT64)mac[0]) << 54) & ((UINT64)0xff << 54));
	lldat += ((((UINT64)mac[1]) << 48) & ((UINT64)0xff << 48));
	lldat += ((((UINT64)mac[2]) << 40) & ((UINT64)0xff << 40));
	lldat += ((((UINT64)mac[3]) << 32) & ((UINT64)0xff << 32));
	lldat += ((((UINT64)mac[5]) << 16) & ((UINT64)0xff << 16));
	lldat += ((((UINT64)mac[4]) << 0) & ((UINT64)0xff << 0));
	lldat = (~lldat);
	lldat = lldat + ((((UINT64)0x12343256) << 32) + (UINT64)0x93233329);
	*lllicense = lldat;
}

void calcCode2String(UINT64 mac, char *pStr)
{
	int nn;
	UINT64 md0 = mac;
	DWORD md1;

	for (nn = 0; nn < 19; nn++)
	{
		if ((nn == 4) || (nn == 9) || (nn == 14))
			pStr[nn] = '-';
		else
		{
			md1 = (DWORD)(md0 >> 60) & 0x0f;

			if (md1 < 10)
				pStr[nn] = (((char)md1 & 0x0f) + (char)'0');
			else
				pStr[nn] = (((char)md1 & 0x0f) - 10 + (char)'A');

			md0 = md0 << 4;
		}
	}

	pStr[nn] = 0;
}

bool setProtectKey(const char *protectkey)
{
	fprintf(stderr, "setProtectKey\n");
	LOG_INFO << "setProtectKey";
    CSQLite m_DB;

	if ( m_DB.Open("./DataBase.sqlite3") == false )
	{
		fprintf(stderr, "error open db\n");
		LOG_ERROR << "not open db";
		return false;
	}

	char sql[512];

	if ( m_DB.CreateStmt("SELECT * FROM info WHERE id='PROTECTKEY';") == false)
	{
		fprintf(stderr, "error CreateStmt\n");
		LOG_ERROR << "error CreateStmt";
		return false;
	}

	if (m_DB.RowExec())
	{
		fprintf(stderr, "setProtectKey update\n");
		sprintf(sql, "update info set value ='%s' where id='PROTECTKEY';", protectkey);
	}
	else
	{
		fprintf(stderr, "setProtectKey insert\n");
		sprintf(sql, "insert into info values('PROTECTKEY','%s');", protectkey);
	}

//	fprintf(stderr, "SQL = %s\n", sql);

	if ( m_DB.Exec(sql) == false )
	{
        fprintf(stderr, "error : %s\n", sql);
	    
        m_DB.Finalize();

	    m_DB.Close();
        return false;
    }
	m_DB.Finalize();

	m_DB.Close();
	fprintf(stderr, "setProtectKey end\n");
    return true;
}


bool createLicense2(int mode)
{
	UINT64 llhcode;
	UINT64 llscode;
	char hwcode[32] = {0};
	char swcode[32] = {0};
	char productkey[256] = {0};
	BYTE mac[6];

	LOG_INFO << "start";

	switch (mode)
	{
	case 1:
		sprintf(productkey, "%s-%s", "1234", "5678");
		break;
	default:
		getMacAddress(mac);
		calcLicenseFromMac(mac, &llhcode, &llscode);
		calcCode2String(llhcode, hwcode);
		calcCode2String(llscode, swcode);
		sprintf(productkey, "%s-%s", hwcode, swcode);
		break;
	}

//	LOG_INFO << "create productkey : " << productkey;

	setProtectKey(productkey);

	LOG_INFO << "end";
	return true;
}
/*
bool createLicense(void)
{
	UINT64 llhcode;
	UINT64 llscode;
	char hwcode[32];
	char swcode[32];
	char productkey[256];
	bool ret = false;
	BYTE mac[6];
	LOG_INFO << "start";

	if (checkLisenceCreateKey() == false)
	{
		LOG_INFO << "not lisence create";
		return false;
	}

	createLicense2(0);
	changeWifiPassword(m_passw);
	LOG_INFO << "lisence create";
	LOG_INFO << "end";
	return true;
}*/

void LoadLicense(char* key)
{
    CSQLite DB;

    DB.Open("./DataBase.sqlite3");

    DB.CreateStmt("SELECT value FROM info WHERE id='PROTECTKEY'");

    if(DB.RowExec())
    {
        sprintf(key,DB.GetColumnStr(0));
    }
}

bool checkLicense(void)
{
	UINT64 llhcode;
	UINT64 llscode;
	char hwcode[32];
	char swcode[32];
	bool ret = false;
	BYTE mac[6];
    char m_ProtectKey[256] = {0};

    LoadLicense(m_ProtectKey);

	getMacAddress(mac);

	calcLicenseFromMac(mac, &llhcode, &llscode);
	calcCode2String(llhcode, hwcode);
	calcCode2String(llscode, swcode);

//	fprintf(stderr, "m_ProtectKey : %s / hwcode : %s / swcode : %s\n", m_ProtectKey, hwcode, swcode);

	if (strstr(m_ProtectKey, hwcode) != NULL)
		if (strstr(m_ProtectKey, swcode) != NULL)
			ret = true;

	LOG_INFO << "License : " << ((ret==true) ? "OK" : "NG");

	if ( ret == false )
	{
        printf("license : hw=%s sw=%s\n",hwcode,swcode);
		LOG_DEBUG << "license : hw/" << hwcode << " sw/" << swcode;
		LOG_DEBUG << "productkey : " << m_ProtectKey;
	}

	return ret;
}


void LoadConf(bool isCommand)
{
    CData ini("setting.ini");
    char command[512] = {'\0'},save[256] = {'\0'},user[256] = {'\0'},pass[256] = {'\0'};
    LOG_INFO << "load configuration";
    ini.getstrdata("SavePath",save,"/dev/sda1");
    ini.getstrdata("LoginUser",user,"root");
    ini.getstrdata("LoginPass",pass,"admin");
    ini.getintdata("fps",Config.fps,90);
    ini.getintdata("width",Config.Width,640);
    ini.getintdata("height",Config.Height,480);
    ini.getbooldata("useH264",Config.useH264,false);
    ini.getbooldata("isRising",Config.isRising,true);
    ini.getintdata("After",Config.nAfterTime,30);
    ini.getintdata("Before",Config.nBeforeTime,30);
    ini.getbooldata("isRec",Config.isRec,false);
    if(Config.useH264==false)
    {
        Config.Width = 320;
        Config.Height = 240;
    }
    mode = (Config.isRec) ? MODE_CHECK:MODE_LIVE;
    if(isCommand){
        if(save[0]=='/'&&save[1]=='/')
        {
            sprintf(command,"mount -t cifs %s /mnt/nat -o user=%s -o password=%s -o uid=$(id -u),gid=$(id -g) > tmp.log",save,user,pass);
            LOG_INFO << command;
            LOG_INFO << "save folder is samba file server.";
            FILE* fp = fopen("tmp.log","r+");
            char str[256] = "";
            while(fgets(str, 256, fp) != NULL) {
		        printf("%s", str);
	        }
            fclose(fp);
        }
        else
        {
            sprintf(command,"mount %s %s -o uid=$(id -u),gid=$(id -g)",save,MOUNTPATH);
            LOG_INFO << command;
            LOG_INFO << "save folder is phisical device.";
        }
        system(command);
    }
}

char* createDir(const char* folder,struct tm *local,char* path)
{
    char path2[256];
    char command[512];
    sprintf(path2,"%s%04d/%02d/%02d/%02d/",folder,local->tm_year + 1900,local->tm_mon+1,local->tm_mday,local->tm_hour);
    sprintf(command,"sudo mkdir -p %s",path2);
    LOG_INFO << command;
    system(command);
    if(Config.useH264)
        sprintf(path,"%s%02d%02d.mp4",path2,local->tm_min,local->tm_sec);
    else
        sprintf(path,"%s%02d%02d.avi",path2,local->tm_min,local->tm_sec);
    return path;
}

class FrameLoop : public Nan::AsyncWorker
{
  public:
    FrameLoop(Nan::Callback *callback)
        : Nan::AsyncWorker(callback)
    {
        ret = 0;
    }

    // 非同期処理の中身
    void Execute()
    {
        //printf("wait next frame\n");
        while (gSendEv.WaitForSingleObject(1000) == 1 && fControl);
        //printf("get frame event\n");
        if(!Config.useH264)
            ret = 1;
        else
            ret = Config.fps;
    }

    // 非同期処理が完了したとき呼び出される
    void HandleOKCallback()
    {
        if (ret == 1)
        {
            v8::Local<v8::Value> *callbackArgs = new v8::Local<v8::Value>[2];
            callbackArgs[0] = Nan::New(ret);
            callbackArgs[1] = Nan::Encode(buffer, jpeglength, Nan::Encoding::BASE64);
            callback->Call(2, callbackArgs);
            delete[] callbackArgs;
            delete[] buffer;
        }
        else if(ret == Config.fps)
        {
                v8::Local<v8::Value> *callbackArgs = new v8::Local<v8::Value>[2];
                callbackArgs[0] = Nan::New(ret);
                unsigned char* sps = h264data[readstream];
                //printf("<%02x %02x %02x %02x %02x>",sps[0],sps[1],sps[2],sps[3],sps[4]&0x1f);
                if((sps[4]&0x1f) != 0x07){
                    LOG_DEBUG << "i or p frame send";
                    callbackArgs[1] = Nan::Encode(h264data[readstream],h264length[readstream],Nan::Encoding::BUFFER);
                    callback->Call(2, callbackArgs);
                }
                else
                {
                    LOG_DEBUG << "sps and pps frame send";
                    //printf("<%02x %02x %02x %02x %02x>",sps[18],sps[19],sps[20],sps[21],sps[22]&0x1f);
                    callbackArgs[1] = Nan::Encode(sps,19,Nan::Encoding::BUFFER);
                    callback->Call(2, callbackArgs);
                    callbackArgs[1] = Nan::Encode(sps+18,h264length[readstream]-19,Nan::Encoding::BUFFER);
                    callback->Call(2, callbackArgs);
                }
                delete[] callbackArgs;
        }
        if (fControl == 1) //ループを続けるか？
        {
            v8::Local<v8::Function> func = callback->GetFunction();
            Nan::Callback *call = new Nan::Callback(func);
            control = new FrameLoop(call);
            Nan::AsyncQueueWorker(control);
        }
    }

  public:
    int ret;
};

void *CaptureThread(void *pParam)
{
    CEvent ev;
    CCap cap;
    int frame = -1;
    int savetime = 0;
    int aftertime = 0;
    //int timing = 0;
    int isSaving = 0;
    FILE *fp;
    CAVIWriter writer;
    fp = fopen(filelist[0], "wb");
    LOG_INFO << "capture thread start";
    ev.CreateEvent();
    cap.open_device();
    cap.set_ctrl("power line frequency",V4L2_CID_POWER_LINE_FREQUENCY,3);
    if(Config.useH264){
        cap.set_ctrl("V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP",V4L2_CID_MPEG_VIDEO_H264_I_PERIOD,(mode==MODE_LIVE) ? 60:15);
        cap.set_ctrl("V4L2_CID_MPEG_VIDEO_BITRATE",V4L2_CID_MPEG_VIDEO_BITRATE,(mode==MODE_LIVE) ? 1000000:5000000);
        cap.set_ctrl("V4L2_CID_MPEG_VIDEO_REPEAT_SEQ_HEADER",V4L2_CID_MPEG_VIDEO_REPEAT_SEQ_HEADER,1);
        cap.set_ctrl("V4L2_CID_MPEG_VIDEO_H264_PROFILE",V4L2_CID_MPEG_VIDEO_H264_PROFILE,V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE);
        cap.set_ctrl("V4L2_CID_MPEG_VIDEO_H264_LEVEL",V4L2_CID_MPEG_VIDEO_H264_LEVEL,V4L2_MPEG_VIDEO_H264_LEVEL_4_0);
        cap.set_ctrl("V4L2_CID_MPEG_VIDEO_BITRATE_MODE",V4L2_CID_MPEG_VIDEO_BITRATE_MODE,V4L2_MPEG_VIDEO_BITRATE_MODE_CBR);
    }
    cap.setFPSSub(Config.fps);
    if(Config.useH264)
        cap.init_device(IO_METHOD_USERPTR, Config.Width, Config.Height, V4L2_PIX_FMT_H264);
    else
        cap.init_device(IO_METHOD_USERPTR, Config.Width, Config.Height, V4L2_PIX_FMT_MJPEG);
    cap.SetEventHandle(ev.GetHandle());
    cap.getFPS(Config.fps);
    cap.start_capturing();
    cap.Go();
    fChangeMode = false;
    while (fControl)
    {

        
        if (ev.WaitForSingleObject(1000) == 1)
        {
            continue;
        }
#if 1
        if(fChangeMode)
        {
            fChangeMode = false;
            LOG_INFO << "capture setting";
            cap.Stop();
            cap.stop_capturing();
            cap.uninit_device();
            //usleep(100*1000);
            cap.close_device();
            cap.open_device();
            cap.set_ctrl("power line frequency",V4L2_CID_POWER_LINE_FREQUENCY,3);
            if(Config.useH264){
                cap.set_ctrl("V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP",V4L2_CID_MPEG_VIDEO_H264_I_PERIOD,(mode==MODE_LIVE) ? 60:15);
                cap.set_ctrl("V4L2_CID_MPEG_VIDEO_BITRATE",V4L2_CID_MPEG_VIDEO_BITRATE,(mode==MODE_LIVE) ? 1000000:5000000);
                //cap.set_ctrl("V4L2_CID_MPEG_VIDEO_REPEAT_SEQ_HEADER",V4L2_CID_MPEG_VIDEO_REPEAT_SEQ_HEADER,1);
                //cap.set_ctrl("V4L2_CID_MPEG_VIDEO_H264_PROFILE",V4L2_CID_MPEG_VIDEO_H264_PROFILE,V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE);
                //cap.set_ctrl("V4L2_CID_MPEG_VIDEO_H264_LEVEL",V4L2_CID_MPEG_VIDEO_H264_LEVEL,(mode==MODE_LIVE) ? V4L2_MPEG_VIDEO_H264_LEVEL_4_0:V4L2_MPEG_VIDEO_H264_LEVEL_4_0);
            }
            cap.setFPSSub(Config.fps);
            if(Config.useH264)
                cap.init_device(IO_METHOD_USERPTR, Config.Width, Config.Height, V4L2_PIX_FMT_H264);
            else
                cap.init_device(IO_METHOD_USERPTR, Config.Width, Config.Height, V4L2_PIX_FMT_MJPEG);
            cap.getFPS(Config.fps);
            cap.start_capturing();
            isSaving = 0;
            aftertime = 0;
            cap.Go();
            printf("capture setting mode=%s",(mode) ? "MODE_CHECK":"MODE_LIVE");
        }
#endif
        if (fTriger == 1)
        {
            
            fTriger = 2;
            savetime = Config.nAfterTime;
            aftertime = 0;
            time_t t = time(NULL);
            save = localtime(&t);
        }

        if(aftertime > Config.nBeforeTime && isSaving == 0)
        {
            LOG_INFO << "save ok";
            printf("save ok\n");
            isSaving = 1;
        }
        unsigned char* tmp = (unsigned char*)cap.m_pbuffers[cap.m_out_buf].start;
        if(Config.useH264==false){
            frame++;
            writer.SaveFrame((unsigned char*)cap.m_pbuffers[cap.m_out_buf].start,cap.m_pbuffers[cap.m_out_buf].used,frame,fp);
            if (frame == Config.fps)
            {
                fflush(fp);
                fclose(fp);
                aftertime++;
                printf("file : %s frame=%3d after=%3d save=%3d\n", filelist[writebuffer],frame, aftertime, savetime);
                writebuffer = (writebuffer + 1) % TIME_FRAME;
                lengthlist[writebuffer] = 0;
                fp = fopen(filelist[writebuffer], "wb");
                writer.SaveHead(Config.fps,fp);
                frame = 0;
                if (aftertime == savetime && fTriger == 2)
                {
                    if(isSaving == 1){
                        fTriger = 3;
                        gSaveEv.SetEvent();
                    }
                    else
                    {
                        LOG_INFO << "Before Time dont reach";
                    }
                }
            }
            if (frame % (Config.fps/10) == 0 && mode==MODE_LIVE)
            {
                buffer = new unsigned char[cap.m_pbuffers[cap.m_out_buf].used];
                memcpy(buffer, cap.m_pbuffers[cap.m_out_buf].start, cap.m_pbuffers[cap.m_out_buf].used);
                jpeglength = cap.m_pbuffers[cap.m_out_buf].used;
                gSendEv.SetEvent();
            }
        }
        else
        {
            if(h264size[writestream] < cap.m_pbuffers[cap.m_out_buf].length)
            {
                //printf("create stream mem old=%d,new=%d\n",h264size[writestream],cap.m_pbuffers[cap.m_out_buf].used);
                if(h264data[writestream] != NULL)
                    delete[] h264data[writestream];
                h264size[writestream] = cap.m_pbuffers[cap.m_out_buf].length;
                h264data[writestream] = new unsigned char[cap.m_pbuffers[cap.m_out_buf].length];
            }
            //printf("write frame %d\n",writestream);
            h264length[writestream] = cap.m_pbuffers[cap.m_out_buf].used;
            memcpy(h264data[writestream],cap.m_pbuffers[cap.m_out_buf].start, h264length[writestream]);
            readstream = writestream;
            writestream = (writestream+1)%10;
            if(mode==MODE_LIVE)
                gSendEv.SetEvent();
            
            //printf("file : %s frame=%3d after=%3d save=%3d\r", filelist[writebuffer],frame, aftertime, savetime);
            if((tmp[4]&0x1f)==0x07)
            {
                frame++;
                if(frame >= (Config.fps))
                {
                    fclose(fp);
                    aftertime++;
                    writebuffer = (writebuffer + 1) % TIME_FRAME;
                    lengthlist[writebuffer] = 0;
                    fp = fopen(filelist[writebuffer], "wb");
                    frame = 0;
                    if (aftertime == savetime && fTriger == 2)
                    {
                        fTriger = 3;
                        gSaveEv.SetEvent();
                    }
                }
                fwrite(cap.m_pbuffers[cap.m_out_buf].start,1,cap.m_pbuffers[cap.m_out_buf].used,fp);
            }
            else if((tmp[4]&0x1f)==0x05)
            {
                frame++;
                fwrite(cap.m_pbuffers[cap.m_out_buf].start,1,cap.m_pbuffers[cap.m_out_buf].used,fp);
            }
            else if((tmp[4]&0x1f)==0x01)
            {
                frame++;
                fwrite(cap.m_pbuffers[cap.m_out_buf].start,1,cap.m_pbuffers[cap.m_out_buf].used,fp);
            }

        }
        
    }
    fflush(fp);
    fclose(fp);
    cap.Stop();
    cap.stop_capturing();
    cap.uninit_device();
    cap.close_device();
    ev.DestroyEvent();
    printf("capture thread exit\n");
    return pParam;
}

void *TrigerThread(void *pParam)
{
    int firstF = 0;
    int endF = 0;
    int length = 0;
    int savetime = 0;
    unsigned char *Wbuffer;
    CAVIWriter writer;
    FILE *tmp = NULL;
    FILE* fp = NULL;
    LOG_INFO << "Triger save thread Start";
    while (fControl)
    {
        if (gSaveEv.WaitForSingleObject(1000) == 1)
        {
            continue;
        }
        if(fControl==0)
        {
            return pParam;
        }
        if (fTriger == 3)
        {
            savetime = Config.nBeforeTime + Config.nAfterTime;
            endF = writebuffer - 1;
            firstF = endF - savetime;
            if (firstF < 0)
                firstF += TIME_FRAME;
            LOG_INFO << "save start end=" << firstF << " begin=" << endF << " time=" << savetime;;
            char path[256];
            createDir(SAVEPATH,save,path);
            LOG_INFO << path;
            if(Config.useH264==false)
                writer.Open(path,Config.fps,Config.Width,Config.Height,savetime*Config.fps);
            else
                fp = fopen("./tmp/buffer.h264","wb");
            for (int i = firstF, j = 1; j <= savetime; j++, i++)
            {
                if (i >= TIME_FRAME)
                    i -= TIME_FRAME;
                tmp = fopen(filelist[i], "rb");
                fseek(tmp,0,SEEK_END);
                length = ftell(tmp);
                fseek(tmp,0,SEEK_SET);
                Wbuffer = new unsigned char[length];
                fread(Wbuffer, 1, length, tmp);
                fclose(tmp);
                if(Config.useH264==false)
                {
                    LOG_INFO << "save mjpeg in : "<< filelist[i] << " size=" << length;
                    writer.WriteSegment(Wbuffer,length);
                }
                else
                {
                    LOG_INFO << "save mp4 in : "<< filelist[i] << " size=" << length;
                    fwrite(Wbuffer,1,length,fp);
                }
                delete[] Wbuffer;
            }
            if(Config.useH264==false)
                writer.Finish();
            else
            {
                fflush(fp);
                fclose(fp);
                char command[512];
                sprintf(command,"sudo MP4Box -fps %d -add ./tmp/buffer.h264 %s;sudo rm ./tmp/buffer.h264;sync",Config.fps,path);
                LOG_DEBUG << command;
                system(command);
            }
            fTriger = 0;
            LOG_INFO << "save end";
        }
    }
    LOG_INFO << "triger thread exit";
    return pParam;
}

void* PoolingThread(void* pParam)
{
    WIOPort wgpio;
    int read = 0;
    int old = 0;
    int out = 28;
    int in = 29;
    int count = 0;
    wgpio.PinMode(out,GPIO_OUT);
    wgpio.PinMode(in,GPIO_IN);
    wgpio.WritePort(out,GPIO_ON);
    LOG_INFO << "Detection Thread Start";
    while(fControl)
    {
        read = wgpio.ReadPort(in);
        if(Config.isRising && read==GPIO_ON && old==GPIO_OFF && count == 0)
        {
            count = 1;
            LOG_INFO << "rising";
        }
        else if(!Config.isRising && old==GPIO_ON && read==GPIO_OFF && count == 0)
        {
            count = 1;
            LOG_INFO << "falling";
        }
        else if(count==1 && old==read)
        {
            count = 2;
        }
        else
        {
            count = 0;
        }
        old = read;
        if(count == 2 && fTriger == 0)
        {
            LOG_INFO << "triger";
            printf("triger\n");
            fTriger = 1;
            count = 0;
        }
        usleep(5000);
    }
    wgpio.WritePort(out,GPIO_OFF);
    printf("pooling thread exit\n");
    return pParam;
}

NAN_METHOD(Start)
{
    int i = 0;
    char msg[256];
    sprintf(msg,"recoder.log");
	plog::init(plog::info,msg,1024*1024,10);
    LoadConf(true);
    if(Config.isRec==false)
    {
        Config.fps = 30;
        Config.Width = 320;
        Config.Height = 180;
    }
    fControl = 1;
    gSendEv.CreateEvent();
    gCapEv.CreateEvent();
    gTriEv.CreateEvent();
    gSaveEv.CreateEvent();
    if(Config.useH264){
        for (i = 0; i < TIME_FRAME; i++)
        {
            sprintf(filelist[i], "./tmp/buffer%02d.h264", i);
        }
        for(i = 0;i<10;i++)
        {
            h264data[i] = NULL;
            h264length[i] = 0;
        }
    }
    else
    {
        for (i = 0; i < TIME_FRAME; i++)
        {
            sprintf(filelist[i], "./tmp/buffer%02d.avi", i);
        }
    }

    LOG_INFO << "Start Camera Control Module";
    if(checkLicense())
    {
        FILE* fp = popen("ls /dev | grep video","r");
        errorcode = 0x01;
        while(fgets(msg,strlen(msg),fp)!=NULL)
        {
            if(strstr(msg,"video0") != NULL)
            {
                errorcode &= 0x0e;
            }
        }
        if(errorcode & 0x01 == 0x00){
            pthread_create(&thread, NULL, CaptureThread, NULL);
        }
        else
        {
            printf("not find camera module\n");
        }
        pthread_create(&triger, NULL, TrigerThread, NULL);
        pthread_create(&pool,NULL,PoolingThread,NULL);
    }
    else
    {
        printf("license fail\n");
    }

}

NAN_METHOD(Stop)
{
    fControl = 0;
    printf("exit process\n");
    gSendEv.SetEvent();
    gSaveEv.SetEvent();
    gTriEv.SetEvent();
    sleep(1);
    char command[256];
    sprintf(command,"sudo umount %s",MOUNTPATH);
    system(command);
    printf("all thread exit\n");
}

NAN_METHOD(Triger)
{
    if (fTriger == 0)
    {
        printf("save triger activate\n");
        fTriger = 1;
    }
}

NAN_METHOD(Live)
{
    Config.fps = 30;
    Config.Width = 320;
    Config.Height = 180;
    mode = MODE_LIVE;
    fChangeMode = true;
}

NAN_METHOD(Check)
{
    char command[256];
    sprintf(command,"sudo umount %s",MOUNTPATH);
    system(command);
    LoadConf(false);
    mode = MODE_CHECK;
    fChangeMode = true;
}

NAN_METHOD(UpdateConf)
{
//    char command[256];
//    sprintf(command,"sudo umount %s",MOUNTPATH);
//    system(command);
//    LoadConf(true);
//    mode = MODE_CHECK;
//    fChangeMode = true;
    LOG_INFO << "ini file update";
}

NAN_METHOD(FirstFrame)
{
    //   fFirstFrame = 1;
}

NAN_METHOD(SetFrameBack)
{
    auto callback = new Nan::Callback(info[0].As<v8::Function>());
    FrameLoop *command = new FrameLoop(callback);
    // 非同期処理を開始
    //printf("thread stat\n");
    Nan::AsyncQueueWorker(command);
}

NAN_METHOD(GetError)
{
    info.GetReturnValue().Set(Nan::New<Boolean>(errorcode!=-1).ToLocal());
}

NAN_METHOD(GetErrorInfo)
{
    info.GetReturnValue().Set(Nan::New(errorinfo[errorcode]).ToLocalChecked());
}

NAN_METHOD(GetHWKey)
{

	UINT64 llhcode;
	UINT64 llscode;
	char hwcode[32] = {0};
	BYTE mac[6];

	getMacAddress(mac);
	calcLicenseFromMac(mac, &llhcode, &llscode);
	calcCode2String(llhcode, hwcode);

    info.GetReturnValue().Set(Nan::New(hwcode).ToLocalChecked());
}

NAN_METHOD(SetLicense)
{
    v8::String::Utf8Value segment(info[0]->ToString());
	UINT64 llhcode;
	UINT64 llscode;
	char hwcode[32] = {0};
	char swcode[32] = {0};
	char productkey[256] = {0};
	bool ret = false;
	BYTE mac[6];

	getMacAddress(mac);
	calcLicenseFromMac(mac, &llhcode, &llscode);
	calcCode2String(llhcode, hwcode);
	calcCode2String(llscode, swcode);
	sprintf(productkey, "%s-%s", hwcode, ToCString(segment));
    printf(productkey);
    if (strstr(productkey, hwcode) != NULL)
		if (strstr(productkey, swcode) != NULL)
			ret = true;

    if(ret)
    {
        setProtectKey(productkey);
        pthread_create(&thread, NULL, CaptureThread, NULL);
        pthread_create(&triger, NULL, TrigerThread, NULL);
        pthread_create(&pool,NULL,PoolingThread,NULL);
    }
    info.GetReturnValue().Set(ret);
}

NAN_METHOD(IsLicense)
{
    info.GetReturnValue().Set(checkLicense());
}

NAN_MODULE_INIT(init)
{
    NAN_EXPORT(target, Start);
    NAN_EXPORT(target, Stop);
    NAN_EXPORT(target, SetFrameBack);
    NAN_EXPORT(target, Triger);
    NAN_EXPORT(target, FirstFrame);
    NAN_EXPORT(target,Live);
    NAN_EXPORT(target,Check);
    NAN_EXPORT(target,UpdateConf);
    NAN_EXPORT(target,GetHWKey);
    NAN_EXPORT(target,SetLicense);
    NAN_EXPORT(target,IsLicense);
    NAN_EXPORT(target,GetError);
    NAN_EXPORT(target,GetErrorInfo);
}

NODE_MODULE(control, init);