
#include "datatype.h"

struct bitfield2fmt
{
    unsigned red_off, red_len;
    unsigned green_off, green_len;
    unsigned blue_off, blue_len;
    unsigned transp_off, transp_len;
    __u32 pixfmt;
};

enum io_method
{
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
    IO_METHOD_VREAD
};

typedef struct _buffer
{
    void* start;
    size_t length;
    size_t used;
} BUFFER, *LPBUFFER;

class CCap
{
  public:
    char m_dev_name[256];
    enum io_method m_io;
    int m_fd;
    LPBUFFER m_pbuffers;
    UINT m_ubuffers;
    int m_out_buf;
    int m_force_format;
    int m_frame_count;
    bool m_bRun;
    pthread_t m_capThread;
    bool m_bGetFrame;
    int m_nFPS;
    bool m_bAutoExposure;
    int m_nExposure;
    int m_nCameraErrorNo;
    char *m_lpbOverlayBuf;
    UINT m_nAWBTarget;
    bool m_bSoftAWB;

    CEvent m_ev;

    CCap();
    ~CCap();

    void errno_exit(const char *s);
    int xioctl(int request, void *arg);
    void open_device(int no = 0);
    void close_device(void);
    void init_device(io_method mode, int width, int height, DWORD vformat);
    void init_read(UINT buffer_size);
    void uninit_device(void);
    int read_frame(void);
    void start_capturing(void);
    void stop_capturing(void);
    void restart_capturing(void);
    void init_mmap(void);
    void init_userp(unsigned int buffer_size);
    void init_vread(unsigned int buffer_size);
    void Go(void);
    void Stop(void);
    static void *threadProc(void *pParam);
    int Run(void);
    int getFrameCount(void);
    int getFrameData(LPBYTE lpdata, int &size);
    bool SetEventHandle(HANDLEEV h);
    int get_ctrl(const char *name, int id, int *value);
    int set_ctrl(const char *name, int id, int value);
    int enumerate_menu(int id);
    void camerainfo(void);
    void setExposure(int exposure);
    void setSoftExposure(bool mode, UINT target);
    bool setFPS(int fps);
    bool getFPS(int &fps);
    bool setFPSSub(int fps);
    bool setRotate(int rotate);
    bool setSceneMode(int mode);
    bool setEV(int ev);
    bool changeSize(int width, int height);

    int fillFBUFFromFB(struct v4l2_framebuffer &fb);
    bool setOverlay(bool mode);
    bool matchBitField(const struct fb_bitfield &bf, unsigned off, unsigned len);
};