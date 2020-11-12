//
//	class CCap
//
//	capture from camara device
//
//  create:2017/08/13
//        :
//

#include <errno.h>
//#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <termios.h>
#include <malloc.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <plog/Log.h>

#include <assert.h>

#include <getopt.h> /* getopt_long() */

#include <linux/videodev2.h>
#include <linux/fb.h>

#include "Event.h"
#include "cap.h"

#define BUFF_COUNT 4

struct bitfield2fmt fb_formats[] = {
    {11, 5, 5, 6, 0, 5, 0, 0, V4L2_PIX_FMT_RGB565},
    {1, 5, 6, 5, 11, 5, 0, 1, V4L2_PIX_FMT_RGB555X},
    {0, 5, 5, 6, 11, 5, 0, 0, V4L2_PIX_FMT_RGB565X},
    {16, 8, 8, 8, 0, 8, 0, 0, V4L2_PIX_FMT_BGR24},
    {0, 8, 8, 8, 16, 8, 0, 0, V4L2_PIX_FMT_RGB24},
    {}
};

CCap::CCap()
    : m_io(IO_METHOD_MMAP), m_fd(-1), m_pbuffers(NULL), m_ubuffers(0), m_out_buf(0), m_force_format(0), m_frame_count(0), m_bRun(false), m_nFPS(4), m_bAutoExposure(true), m_nExposure(100), m_nCameraErrorNo(0), m_lpbOverlayBuf(NULL), m_nAWBTarget(200), m_bSoftAWB(false)
//, m_bGetFrame(false)
{
}

CCap::~CCap()
{
}

bool CCap::SetEventHandle(HANDLEEV h)
{
    m_ev.Attach(h);
}

void CCap::errno_exit(const char *s)
{
    // check
    //fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
    fprintf(stderr, "%s error %d\n", s, errno);
    LOG_ERROR << s << " error " << errno;
    if ( m_nCameraErrorNo > 2 )
        m_nCameraErrorNo = 2;

//    exit(EXIT_FAILURE);
}

int CCap::xioctl(int request, void *arg)
{
    int ret;

    do
    {
        ret = ioctl(m_fd, request, arg);
    } while (ret == -1 && errno == EINTR);

    return ret;
}

int CCap::read_frame(void)
{
    char log[256];
//    sprintf(log, "read_frame : %08x", m_fd);
//    LOG_DEBUG << log;

    struct v4l2_buffer buf;
    unsigned int i;
    int l = 0;

    switch (m_io)
    {
    case IO_METHOD_READ:
        if (read(m_fd, m_pbuffers[0].start, m_pbuffers[0].length) == -1)
        {
            switch (errno)
            {
            case EAGAIN:
                //ノンブロッキングI/Oは、O_NONBLOCKを使用して選択されており、すぐにデータを読み取ることはできませんでした。
                m_nCameraErrorNo = 0;
                return 0;

            case EBADF:
                //fdが有効なファイル記述子ではないか、または読み込み用にオープンされていないか、またはプロセスがすでに最大数のファイルを開いています。
                if (m_nCameraErrorNo == 0)
                    LOG_ERROR << "read error EBADF";
                m_nCameraErrorNo = 1;
                return 0;

            case EBUSY:
                //ドライバは複数の読み込みストリームをサポートしておらず、デバイスはすでに使用されています。
//                if (m_nCameraErrorNo == 0)
//                    LOG_ERROR << "read error EBUSY";
                m_nCameraErrorNo = 0;
                return 0;

            case EFAULT:
                //bufは、アクセスできないメモリ領域を参照します。
                if (m_nCameraErrorNo == 0)
                    LOG_ERROR << "read error EFAULT";
                m_nCameraErrorNo = 1;
                return 0;

            case EINTR:
                //すべてのデータが読み取られる前に、信号によってコールが中断されました。
                if (m_nCameraErrorNo == 0)
                    LOG_ERROR << "read error EINTR";
                m_nCameraErrorNo = 0;
                return 0;

            case EIO:
                //入出力エラーです。これは、ハードウェアの問題や、リモートデバイス（USBカメラなど）との通信に失敗したことを示します。
                if (m_nCameraErrorNo == 0)
                    LOG_ERROR << "read error EIO";
                m_nCameraErrorNo = 1;
                return 0;

            case EINVAL:
                //read（）関数は、このドライバではサポートされていません。このデバイスではサポートされていません。
                if (m_nCameraErrorNo == 0)
                    LOG_ERROR << "read error EINVAL";
                m_nCameraErrorNo = 1;
                return 0;

            default:
                if (m_nCameraErrorNo == 0)
                    LOG_ERROR << "read error " << errno;
                m_nCameraErrorNo = 1;
                return 0;
            }
        }

        m_nCameraErrorNo = 0;
        m_out_buf = 0;
        //process_image(m_pbuffers[0].start, m_pbuffers[0].length);
        break;

    case IO_METHOD_MMAP:
        memset((void *)&buf, 0, (size_t)sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (xioctl(VIDIOC_DQBUF, &buf) == -1)
        {
            switch (errno)
            {
            case EAGAIN:
                return 0;

            case EIO:
            /* Could ignore EIO, see spec. */

            /* fall through */

            default:
                LOG_ERROR << "VIDIOC_DQBUF 1";
                errno_exit("VIDIOC_DQBUF");
            }
        }

        assert(buf.index < m_ubuffers);

        //process_image(m_pbuffers[buf.index].start, buf.bytesused);

        if (xioctl(VIDIOC_QBUF, &buf) == -1)
        {
            LOG_ERROR << "VIDIOC_DQBUF 2";
            errno_exit("VIDIOC_QBUF");
        }
        break;

    case IO_METHOD_USERPTR:
        memset((void *)&buf, 0, (size_t)sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;

        if (xioctl(VIDIOC_DQBUF, &buf) == -1)
        {
            switch (errno)
            {
            case EAGAIN:
                return 0;

            case EIO:
            /* Could ignore EIO, see spec. */

            /* fall through */

            default:
                LOG_ERROR << "VIDIOC_QBUF 1";
                errno_exit("VIDIOC_QBUF");
            }
        }

        for (i = 0; i < m_ubuffers; ++i)
        {
            if (buf.m.userptr == (DWORD)m_pbuffers[i].start && buf.length == m_pbuffers[i].length)
                break;
        }

        assert(i < m_ubuffers);

        //(void *)buf.m.userptr;
        m_out_buf = i;
        m_pbuffers[i].used = buf.bytesused;

        if (xioctl(VIDIOC_QBUF, &buf) == -1)
        {
            LOG_ERROR << "VIDIOC_QBUF 2";
            errno_exit("VIDIOC_QBUF");
        }
        break;
    }

    return 1;
}

void CCap::Go(void)
{
    LOG_INFO << "go";
    fprintf(stderr, "go\n");
    pthread_create(&m_capThread, NULL, threadProc, (void *)this);
}

void CCap::Stop(void)
{
    LOG_INFO << "stop";
    fprintf(stderr, "stop\n");
    void *res;
    m_bRun = false;

    pthread_join(m_capThread, &res);

    free(res);
}

int CCap::getFrameCount(void)
{
    return m_frame_count;
}

void *CCap::threadProc(void *pParam)
{
    fprintf(stderr, "threadProc\n");
    CCap *pCap = (CCap *)pParam;
    pCap->Run();
    return NULL;
}

int CCap::Run(void)
{
    LOG_INFO << "run";
    fprintf(stderr, "run\n");
    UINT count = 0;
    fd_set fds;
    struct timeval tv;
    int r = 0;
    int ret = 0;
    UINT nAWBAvgArrey[10] = {0,0,0,0,0,0,0,0,0,0};
    UINT nAWBAvg = 0;
    UINT nAWBAvgTotal = 0;
    int nAWBFi = 0;
    int nAWBFo = 0;
    bool fifofull = false;

    struct timeval Start, End;
    gettimeofday(&Start, NULL);
    m_bRun = true;
    int nMS = 0;
    LPBYTE lpbAWBBuffer = NULL;
    int nAvgOld = 0;
    int nAWBFramCount = 0;
    bool bExposureChangeOK;

    while (m_bRun)
    {
        FD_ZERO(&fds);
        FD_SET(m_fd, &fds);

        /* Timeout. */
        tv.tv_sec = 10;
        tv.tv_usec = 0;

        r = select(m_fd + 1, &fds, NULL, NULL, &tv);
//        fprintf(stderr, "select end\n");

        if (r == -1)
        {
            LOG_INFO << "select error : " << errno;
            fprintf(stderr, "select error : %d\n", errno);
            continue;
        }

        if (r == 0)
        {
            LOG_INFO << "select timeout";
            fprintf(stderr, "select timeout\n");
            //            close_device();
            //            open_device();
            continue;
        }

        ret = read_frame();

#if 1
        gettimeofday(&End, NULL);

        if (ret == 1)
        {
            m_ev.SetEvent();
            m_frame_count++;
        }
        else
            continue;

//        fprintf(stderr, "frame count : %d", m_frame_count);
#if 1
        UINT nAWBTotal = 0;
        lpbAWBBuffer = (LPBYTE)m_pbuffers[0].start;
        for( int i = 1; i < m_pbuffers[0].length; i += 3)
        {
            nAWBTotal += (UINT)lpbAWBBuffer[i];
        }

        nAWBTotal /= (m_pbuffers[0].length / 3);
//        LOG_DEBUG << "nAWBTotal : " << nAWBTotal;

        if (!fifofull)
        {
            nAWBAvgArrey[nAWBFi] = nAWBTotal;
            nAWBAvgTotal += nAWBTotal;
//            LOG_DEBUG << "nAWBAvgTotal : " << nAWBAvgTotal << " / " << nAWBFi;
            nAWBFi++;
            nAWBAvg = nAWBAvgTotal / nAWBFi;
//            LOG_DEBUG << "nAWBAvg : " << nAWBAvg;

            if (nAWBFi >= 10)
            {
                nAWBFi = 0;
                fifofull = true;
            }
        }
        else
        {
            nAWBAvgTotal -= nAWBAvgArrey[nAWBFi];
            nAWBAvgArrey[nAWBFi] = nAWBTotal;
            nAWBAvgTotal += nAWBTotal;
//            LOG_DEBUG << "nAWBAvgTotal : " << nAWBAvgTotal << " / 10";
            nAWBFi++;
            nAWBAvg = nAWBAvgTotal / 10;
//            LOG_DEBUG << "nAWBAvg : " << nAWBAvg;

            if (nAWBFi >= 10)
                nAWBFi = 0;
        }

        if ( abs(nAWBTotal - nAWBAvg) < 3 )
            bExposureChangeOK = true;

//        LOG_DEBUG << "AWB avg : " << nAWBAvg << " target : " << m_nAWBTarget << " check : " << bExposureChangeOK << " on/off : " << m_bAutoExposure << " " << m_bSoftAWB;

        if (m_bSoftAWB && bExposureChangeOK)
        {
            bool bUp = false;
//            LOG_DEBUG << "AWB avg : " << nAWBAvg << " target : " << m_nAWBTarget;

            int lev = m_nAWBTarget - nAWBAvg;
            int exposure = m_nExposure;
            if(lev > 0)
                bUp = true;

            if (lev > 80)
                exposure += 50;
            else if (lev > 50)
                exposure += 20;
            else if (lev > 20)
                exposure += 5;
            else if (lev > 10)
                exposure += 1;
            else if (lev < -100)
            {
                if (exposure > 500)
                    exposure -= 150;
                else if (exposure < 200)
                    exposure -= 10;
                else
                    exposure -= 20;
            }
            else if (lev < -50)
                if (exposure > 40)
                    exposure -= 20;
                else if (exposure < 20)
                    exposure -= 1;
                else
                    exposure -= 5;
            else if (lev < -25)
                if (exposure > 20)
                    exposure -= 10;
                else if (exposure < 10)
                    exposure -= 1;
                else
                    exposure -= 5;
            else if (lev < -15)
                exposure -= 1;

            if (exposure > 2500)
                exposure = 2500;
            else if (exposure < 0)
                exposure = 0;

            LOG_DEBUG << "bUp : " << bUp << " / nAWBAvg : " << nAWBAvg << " / exposure : " << exposure << " / m_nExposure : " << m_nExposure;
            if (m_nExposure != exposure)
            {
                if (bUp && (nAWBAvg < 20 || exposure > 1000))
                    setSceneMode(8);
                else if (!bUp && exposure <= 1000)
                    setSceneMode(0);

                setExposure(exposure);
                bExposureChangeOK = false;
            }
        }
#endif
#else
        nMS += (End.tv_sec - Start.tv_sec) * 1000 + (End.tv_usec - Start.tv_usec) / 1000;

        if (nMS - 250 > 0)
        {
            nMS -= 250;

            if (ret == 1)
                m_ev.SetEvent();
        }
#endif

#if 1
        if (m_frame_count % 100 == 0)
        {
            char log[256];
            sprintf(log,
                    "%ld.%ld - %ld.%ld  %ld ms %d frame",
                    Start.tv_sec, Start.tv_usec,
                    End.tv_sec, End.tv_usec,
                    (End.tv_sec - Start.tv_sec) * 1000 + (End.tv_usec - Start.tv_usec) / 1000,
                    m_frame_count);

            LOG_DEBUG << log;
        }
#endif
        Start.tv_sec = End.tv_sec;
        Start.tv_usec = End.tv_usec;
    }

    return 1;
}

void CCap::start_capturing(void)
{
    LOG_INFO << "start capture";
    fprintf(stderr, "start capture\n");

    UINT i;
    enum v4l2_buf_type type;
    m_frame_count = 0;

    setOverlay(false);

    switch (m_io)
    {
    case IO_METHOD_READ:
        /* Nothing to do. */
        break;

    case IO_METHOD_MMAP:
        for (i = 0; i < m_ubuffers; ++i)
        {
            struct v4l2_buffer buf;

            memset((void *)&buf, 0, (size_t)sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

            if (xioctl(VIDIOC_QBUF, &buf) == -1)
                errno_exit("VIDIOC_QBUF");
        }

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (xioctl(VIDIOC_STREAMON, &type) == -1)
            errno_exit("VIDIOC_STREAMON");

        break;

    case IO_METHOD_USERPTR:
        for (i = 0; i < m_ubuffers; ++i)
        {
            struct v4l2_buffer buf;

            memset((void *)&buf, 0, (size_t)sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;
            buf.index = i;
            buf.m.userptr = (unsigned long)m_pbuffers[i].start;
            buf.length = m_pbuffers[i].length;

            if (xioctl(VIDIOC_QBUF, &buf) == -1)
                errno_exit("VIDIOC_QBUF");
        }

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (xioctl(VIDIOC_STREAMON, &type) == -1)
            errno_exit("VIDIOC_STREAMON");

        break;
    }
}

void CCap::restart_capturing(void)
{
LOG_INFO << "restart capture";
    fprintf(stderr, "restart capture\n");

    UINT i;
    enum v4l2_buf_type type;
    m_frame_count = 0;

    setOverlay(false);

    switch (m_io)
    {
    case IO_METHOD_READ:
        /* Nothing to do. */
        break;

    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if(xioctl(VIDIOC_STREAMOFF,&type)==-1)
            errno_exit("VIDIOC_STREAMOFF");
        if (xioctl(VIDIOC_STREAMON, &type) == -1)
            errno_exit("VIDIOC_STREAMON");
    }
}

void CCap::stop_capturing(void)
{
    fprintf(stderr, "stop capture\n");
    LOG_INFO << "stop capture";

    enum v4l2_buf_type type;
    setOverlay(false);

    switch (m_io)
    {
    case IO_METHOD_READ:
        /* Nothing to do. */
        break;

    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (xioctl(VIDIOC_STREAMOFF, &type) == -1)
            errno_exit("VIDIOC_STREAMOFF");

        break;
    }
}

void CCap::init_read(UINT buffer_size)
{
    fprintf(stderr, "init_read\n");
    LOG_INFO << "buffer size : " << buffer_size;

    m_pbuffers = (LPBUFFER)calloc(1, sizeof(*m_pbuffers));
    //    m_pbuffers = new BUFFER[1];

    if (!m_pbuffers)
    {
        fprintf(stderr, "Out of memory\n");
        LOG_ERROR << "Out of memory";
        exit(EXIT_FAILURE);
    }

    m_pbuffers[0].length = buffer_size;
    m_pbuffers[0].start = malloc(buffer_size); //    new BYTE[buffer_size];

    if (!m_pbuffers[0].start)
    {
        fprintf(stderr, "Out of memory\n");
        LOG_ERROR << "Out of memory";
        exit(EXIT_FAILURE);
    }
}

void CCap::init_mmap(void)
{
    fprintf(stderr, "init_mmap\n");
    LOG_INFO << "init_mmap";

    struct v4l2_requestbuffers req;

    memset((void *)&req, 0, (size_t)sizeof(req));

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    char log[256];

    if (xioctl(VIDIOC_REQBUFS, &req) == -1)
    {
        if (EINVAL == errno)
        {
            sprintf(log, "%s does not support memory mapping", m_dev_name);
            fprintf(stderr, "%s\n", log);
            LOG_ERROR << log;
            exit(EXIT_FAILURE);
        }
        else
        {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2)
    {
        sprintf(log, "Insufficient buffer memory on %s", m_dev_name);
        fprintf(stderr, "%s\n", log);
        LOG_ERROR << log;
        exit(EXIT_FAILURE);
    }

    m_pbuffers = (BUFFER *)calloc(req.count, sizeof(*m_pbuffers));
    //    m_pbuffers = new BUFFER[req.count];

    if (!m_pbuffers)
    {
        fprintf(stderr, "Out of memory\n");
        LOG_ERROR << "Out of memory";
        exit(EXIT_FAILURE);
    }

    for (m_ubuffers = 0; m_ubuffers < req.count; ++m_ubuffers)
    {
        struct v4l2_buffer buf;

        memset((void *)&buf, 0, (size_t)sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = m_ubuffers;

        if (xioctl(VIDIOC_QUERYBUF, &buf) == -1)
        {
            errno_exit("VIDIOC_QUERYBUF");
        }

        m_pbuffers[m_ubuffers].length = buf.length;
        m_pbuffers[m_ubuffers].start =
            (LPBYTE)mmap(NULL /* start anywhere */,
                         buf.length,
                         PROT_READ | PROT_WRITE /* required */,
                         MAP_SHARED /* recommended */,
                         m_fd, buf.m.offset);

        if (m_pbuffers[m_ubuffers].start == MAP_FAILED)
            errno_exit("mmap");
    }
}

void CCap::init_userp(unsigned int buffer_size)
{
    char log[256];
//    fprintf(stderr, "init_userp\n");

    struct v4l2_requestbuffers req;

    memset((void *)&req, 0, (size_t)sizeof(req));

    req.count = 8;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    if (xioctl(VIDIOC_REQBUFS, &req) == -1)
    {
        if (errno == EINVAL)
        {
            fprintf(stderr, "%s does not support user pointer i/o\n", m_dev_name);
            exit(EXIT_FAILURE);
        }
        else
        {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    m_pbuffers = (BUFFER *)calloc(8, sizeof(*m_pbuffers));
    //    m_pbuffers = new BUFFER[4];

    if (!m_pbuffers)
    {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    for (m_ubuffers = 0; m_ubuffers < 8; ++m_ubuffers)
    {
        m_pbuffers[m_ubuffers].length = buffer_size;
        m_pbuffers[m_ubuffers].start = malloc(buffer_size); // new BYTE[buffer_size];

        if (!m_pbuffers[m_ubuffers].start)
        {
            fprintf(stderr, "Out of memory\n");
            exit(EXIT_FAILURE);
        }
    }
}

void CCap::init_vread(unsigned int buffer_size)
{
        char log[256];
//    fprintf(stderr, "init_userp\n");

    struct v4l2_requestbuffers req;

    memset((void *)&req, 0, (size_t)sizeof(req));

    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    if (xioctl(VIDIOC_REQBUFS, &req) == -1)
    {
        if (errno == EINVAL)
        {
            fprintf(stderr, "%s does not support user pointer i/o\n", m_dev_name);
            exit(EXIT_FAILURE);
        }
        else
        {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    m_pbuffers = (BUFFER *)calloc(1, sizeof(*m_pbuffers));
    //    m_pbuffers = new BUFFER[4];

    if (!m_pbuffers)
    {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    for (m_ubuffers = 0; m_ubuffers < 1; ++m_ubuffers)
    {
        m_pbuffers[m_ubuffers].length = buffer_size;
        m_pbuffers[m_ubuffers].start = malloc(buffer_size); // new BYTE[buffer_size];

        if (!m_pbuffers[m_ubuffers].start)
        {
            fprintf(stderr, "Out of memory\n");
            exit(EXIT_FAILURE);
        }
    }
}

bool CCap::changeSize(int width, int height)
{
    LOG_INFO << "chane img size : w " << width << " / h " << height;

    struct v4l2_format fmt;
    memset((void *)&fmt, 0, (size_t)sizeof(fmt));

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (xioctl(VIDIOC_G_FMT, &fmt) == -1)
        errno_exit("VIDIOC_G_FMT");
    else
        LOG_DEBUG << "VIDIOC_G_FMT OK";

    if (fmt.fmt.pix.width == width && fmt.fmt.pix.height == height)
    {
        LOG_INFO << "no change size.";
        return true;
    }

    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;

    if (xioctl(VIDIOC_S_FMT, &fmt) == -1)
        errno_exit("VIDIOC_S_FMT");

    if (xioctl(VIDIOC_G_FMT, &fmt) == -1)
        errno_exit("VIDIOC_G_FMT");

    char log[256];
    sprintf(log, "size : %d x %d  bpl = %d", fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.bytesperline);
//    fprintf(stderr, "%s\n", log);
    LOG_INFO << log;
    return true;
}

void CCap::init_device(io_method mode, int width, int height, DWORD vformat)
{
    char log[256];
    LOG_INFO << "init device : io = " << mode << " w = " << width << " h = " << height << " fmt = " << vformat;

    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;
    bool change = false;

    if (m_io == mode)
        change = true;

    m_io = mode;

    if (xioctl(VIDIOC_QUERYCAP, &cap) == -1)
    {
        if (errno == EINVAL)
        {
            sprintf(log, "%s is no V4L2 device", m_dev_name);
            fprintf(stderr, "%s\n", log);
            LOG_ERROR << log;
            exit(EXIT_FAILURE);
        }
        else
        {
            errno_exit("VIDIOC_QUERYCAP");
        }
    }
    else
        LOG_DEBUG << "VIDIOC_QUERYCAP OK";

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        sprintf(log, "%s is no video capture device", m_dev_name);
        fprintf(stderr, "%s\n", log);
        LOG_ERROR << log;
        exit(EXIT_FAILURE);
    }
    else
        LOG_DEBUG << "V4L2_CAP_VIDEO_CAPTURE OK";
    
    switch (m_io)
    {
    case IO_METHOD_READ:
        if (!(cap.capabilities & V4L2_CAP_READWRITE))
        {
            sprintf(log, "%s does not support read i/o", m_dev_name);
            fprintf(stderr, "%s\n", log);
            LOG_ERROR << log;
            exit(EXIT_FAILURE);
        }
        else
            LOG_DEBUG << "V4L2_CAP_READWRITE OK";
        break;

    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
        if (!(cap.capabilities & V4L2_CAP_STREAMING))
        {
            sprintf(log, "%s does not support streaming i/o", m_dev_name);
            fprintf(stderr, "%s\n", log);
            LOG_ERROR << log;
            exit(EXIT_FAILURE);
        }
        else
            LOG_DEBUG << "V4L2_CAP_STREAMING OK";
        break;
    }

    /* Select video input, video standard and tune here. */

    memset((void *)&cropcap, 0, (size_t)sizeof(cropcap));

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl(VIDIOC_CROPCAP, &cropcap))
    {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */
        fprintf(stderr, "crop set : %d,%d,%d,%d\n", crop.c.top, crop.c.left, crop.c.width, crop.c.height);

        if (xioctl(VIDIOC_S_CROP, &crop) == -1)
        {
            switch (errno)
            {
            case EINVAL:
                /* Cropping not supported. */
                break;
            default:
                /* Errors ignored. */
                break;
            }
        }
    }
    else
    {
        /* Errors ignored. */
    }

    memset((void *)&fmt, 0, (size_t)sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (xioctl(VIDIOC_G_FMT, &fmt) == -1)
        errno_exit("VIDIOC_G_FMT");
    else
        LOG_DEBUG << "VIDIOC_G_FMT OK";

    sprintf(log, "size : %d x %d  bpl = %d", fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.bytesperline);
    LOG_INFO << log;

    if (fmt.fmt.pix.width == width && fmt.fmt.pix.height == height && fmt.fmt.pix.pixelformat == vformat)
    {
        LOG_INFO << "no change size.";
    }
    else
    {
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = width;
        fmt.fmt.pix.height = height;
        fmt.fmt.pix.pixelformat = vformat;
        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

        if (xioctl(VIDIOC_S_FMT, &fmt) == -1)
            errno_exit("VIDIOC_S_FMT");
        else
            LOG_DEBUG << "VIDIOC_S_FMT OK";

        if (xioctl(VIDIOC_G_FMT, &fmt) == -1)
            errno_exit("VIDIOC_G_FMT");
        else
            LOG_DEBUG << "VIDIOC_G_FMT OK";

        sprintf(log, "size : %d x %d  bpl = %d", fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.bytesperline);
        fprintf(stderr, "%s\n", log);
        LOG_INFO << log;
    }
    /* Note VIDIOC_S_FMT may change width and height. */

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;

    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;

    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;

    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

    switch (m_io)
    {
    case IO_METHOD_READ:
        init_read(fmt.fmt.pix.sizeimage);
        break;

    case IO_METHOD_MMAP:
        init_mmap();
        break;

    case IO_METHOD_USERPTR:
        init_userp(fmt.fmt.pix.sizeimage);
        break;        
    case IO_METHOD_VREAD:

        init_read(fmt.fmt.pix.sizeimage);
        break;
    }
}

void CCap::uninit_device(void)
{
    fprintf(stderr, "uninit_device\n");
    LOG_INFO << "uninit_device";
    unsigned int i;

    switch (m_io)
    {
    case IO_METHOD_READ:
        free(m_pbuffers[0].start);
        //        delete[](m_pbuffers[0].start);
        break;

    case IO_METHOD_MMAP:
        for (i = 0; i < m_ubuffers; ++i)
        {
            if (munmap(m_pbuffers[i].start, m_pbuffers[i].length) == -1)
                errno_exit("munmap");
        }
        break;

    case IO_METHOD_USERPTR:
        for (i = 0; i < m_ubuffers; ++i)
        {
            free(m_pbuffers[i].start);
            //            delete[](m_pbuffers[i].start);
        }
        break;
    }

    free(m_pbuffers);
    //    delete[] m_pbuffers;
}

void CCap::close_device(void)
{
    fprintf(stderr, "close_device\n");
    LOG_INFO << "close_device";

    if (close(m_fd) == -1)
        errno_exit("close");

    m_fd = -1;
}

void CCap::open_device(int no)
{
    fprintf(stderr, "open_device\n");
    LOG_INFO << "open device";
    struct stat st;
    char log[256];
    sprintf(m_dev_name, "/dev/video%d", no);

    if (stat(m_dev_name, &st) == -1)
    {
        // check
        //        fprintf(stderr, "Cannot identify '%s': %d, %s\n", m_dev_name, errno, std::strerror(errno));
        sprintf(log, "Cannot identify '%s': %d", m_dev_name, errno);
        LOG_ERROR << log;
        exit(EXIT_FAILURE);
    }
    else
        LOG_DEBUG << "identify OK";

    if (!S_ISCHR(st.st_mode))
    {
        sprintf(log, "%s is no device", m_dev_name);
        fprintf(stderr, "%s\n", log);
        LOG_ERROR << log;
        exit(EXIT_FAILURE);
    }
    else
        LOG_DEBUG << "device OK";

    m_fd = open(m_dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

    if (m_fd == -1)
    {
        sprintf(log, "Cannot open '%s': %d", m_dev_name, errno);
        fprintf(stderr, "%s\n", log);
        LOG_ERROR << log;
        exit(EXIT_FAILURE);
    }
    else
        LOG_DEBUG << "device open";
}

int CCap::enumerate_menu(int id)
{
    struct v4l2_queryctrl queryctrl;
    struct v4l2_querymenu querymenu;
    char log[256];

    memset(&queryctrl, 0, sizeof(queryctrl));
    queryctrl.id = id;

    if (xioctl(VIDIOC_QUERYCTRL, &queryctrl) < 0)
    {
        fprintf(stderr, "enumerate_menu error\n");
        LOG_ERROR << "enumerate_menu error";
        return -1;
    }

    sprintf(log, "Control %s\n", queryctrl.name);
    fprintf(stderr, "%s\n", log);
    LOG_INFO << log;

    if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
    {
        fprintf(stderr, "  Menu items:\n");
        LOG_INFO << "  Menu items:";

        memset(&querymenu, 0, sizeof(querymenu));
        querymenu.id = queryctrl.id;

        for (querymenu.index = queryctrl.minimum;
             querymenu.index <= queryctrl.maximum;
             querymenu.index++)
        {
            if (xioctl(VIDIOC_QUERYMENU, &querymenu) == 0)
            {
                sprintf(log, "  %s\n", querymenu.name);
                fprintf(stderr, "%s\n", log);
                LOG_INFO << log;
            }
            else
            {
                fprintf(stderr, "VIDIOC_QUERYMENU\n");
                return -1;
            }
        }

        return querymenu.index;
    }

    return 0;
}

int CCap::get_ctrl(const char *name, int id, int *value)
{
    struct v4l2_control ctrl;
    char log[256];

    ctrl.id = id;

    if (xioctl(VIDIOC_G_CTRL, &ctrl) < 0)
    {
        sprintf(log, "now %s = not data", name);
        fprintf(stderr, "%s\n", log);
        LOG_ERROR << log;
        return -1;
    }

    sprintf(log, "now %s = %d", name, ctrl.value);
    fprintf(stderr, "%s\n", log);
    LOG_INFO << log;
    *value = ctrl.value;

    return 0;
}

int CCap::set_ctrl(const char *name, int id, int value)
{
    int now;
    char log[256];
    struct v4l2_control ctrl;

    if (get_ctrl(name, id, &now) != 0)
    {
        sprintf(log, "now %s = not data", name);
        fprintf(stderr, "%s\n", log);
        LOG_ERROR << log;
        return -1;
    }

    ctrl.id = id;
    ctrl.value = value;

    if (xioctl(VIDIOC_S_CTRL, &ctrl) < 0)
    {
        sprintf(log, "now %s = not data", name);
        fprintf(stderr, "%s\n", log);
        LOG_ERROR << log;
        return -1;
    }

    sprintf(log, "change %s = %d", name, ctrl.value);
    fprintf(stderr, "%s\n", log);
    LOG_INFO << log;

    return 0;
}

bool CCap::setSceneMode(int mode)
{
    char name[256];
    int old = 0;
    strcpy(name, "SceneMode");
    if (get_ctrl(name, V4L2_CID_SCENE_MODE, &old) < 0)
        return false;

    if (old == mode)
        return true;

    if (set_ctrl(name, V4L2_CID_SCENE_MODE, mode) < 0)
        return false;
}

bool CCap::setEV(int ev)
{
    char name[256];
    int old = 0;
    //    struct v4l2_queryctrl parm;

    //    memset(&parm, 0, sizeof(parm));
    //    parm.type = V4L2_CID_AUTO_EXPOSURE_BIAS

    strcpy(name, "EV");
    if (get_ctrl(name, V4L2_CID_AUTO_EXPOSURE_BIAS, &old) < 0)
        return false;

    if (old == ev)
        return true;

    if (set_ctrl(name, V4L2_CID_AUTO_EXPOSURE_BIAS, ev) < 0)
        return false;
}

bool CCap::setRotate(int rotate)
{
    char name[256];
    int old = 0;

    strcpy(name, "rotate");
    if (get_ctrl(name, V4L2_CID_ROTATE, &old) < 0)
        return false;

    if (old == rotate)
        return true;

    if (set_ctrl(name, V4L2_CID_ROTATE, rotate) < 0)
        return false;
}

bool CCap::getFPS(int &fps)
{
    char log[256];
    struct v4l2_streamparm parm;
    memset(&parm, 0, sizeof(parm));
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    int ret = xioctl(VIDIOC_G_PARM, &parm);
    if (ret < 0)
    {
        fprintf(stderr, "get error\n");
        LOG_ERROR << "get error";
        return false;
    }

    sprintf(log, "now FPS = %d/%d", parm.parm.capture.timeperframe.denominator, parm.parm.capture.timeperframe.numerator);
    fprintf(stderr, "%s\n", log);
    LOG_INFO << log;

    fps = parm.parm.capture.timeperframe.denominator / parm.parm.capture.timeperframe.numerator;

    return true;
}

bool CCap::setFPSSub(int fps)
{
    char log[256];
    int old = 0;

    sprintf(log, "set FPS = %d", fps);
    fprintf(stderr, "%s\n", log);
    LOG_DEBUG << log;

    if (getFPS(old) == false)
        return false;

    sprintf(log, "1:now FPS = %d", old);
    fprintf(stderr, "%s\n", log);
    LOG_INFO << log;

    if (fps == old)
        return true;

    sprintf(log, "2:now FPS = %d", fps);
    fprintf(stderr, "%s\n", log);
    LOG_INFO << log;
    struct v4l2_streamparm parm;
    memset(&parm, 0, sizeof(parm));
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.capture.timeperframe.numerator = 1000;
    parm.parm.capture.timeperframe.denominator = fps * parm.parm.capture.timeperframe.numerator;

    if (xioctl(VIDIOC_S_PARM, &parm) < 0)
    {
        fprintf(stderr, "set error\n");
        LOG_ERROR << "set error";
        return false;
    }

    if (getFPS(old) == false)
        return false;

    if (fps != old)
    {
        sprintf(log, "set error now fps = %d", old);
        fprintf(stderr, "%s\n", log);
        LOG_ERROR << log;
        return false;
    }

    sprintf(log, "set FPS = %d -> %d", old, fps);
    fprintf(stderr, "%s\n", log);
    LOG_INFO << log;
    m_nFPS = fps;

    return true;
}

bool CCap::setFPS(int fps)
{
    int old = 0;
    Stop();
    stop_capturing();
    //	uninit_device();
    //	close_device();

    bool ret = setFPSSub(fps);

    //	open_device();
    //	init_device(IO_METHOD_READ, m_nCapW, m_nCapH, V4L2_PIX_FMT_RGB24);
    start_capturing();
    Go();
    return ret;
}

void CCap::setExposure(int exposure)
{
    int value = 0;
    char name[256];
    char log[256];

    sprintf(log, "set exposure = %d", exposure);
    LOG_INFO << log;

    if (exposure == 0 && !m_bSoftAWB)
    {
        set_ctrl("exposure auto", V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_AUTO);
        set_ctrl("exposure auto priority", V4L2_CID_EXPOSURE_AUTO_PRIORITY, 0);
        get_ctrl("exposure", V4L2_CID_EXPOSURE_ABSOLUTE, &value);
        m_bAutoExposure = true;
    }
    else
    {
        set_ctrl("exposure auto", V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_MANUAL);
        m_bAutoExposure = false;

        if (exposure > 300)
            set_ctrl("exposure auto priority", V4L2_CID_EXPOSURE_AUTO_PRIORITY, 1);
        else
            set_ctrl("exposure auto priority", V4L2_CID_EXPOSURE_AUTO_PRIORITY, 0);

        set_ctrl("exposure absolute", V4L2_CID_EXPOSURE_ABSOLUTE, exposure);
        m_nExposure = exposure;
    }
}

void CCap::setSoftExposure(bool mode, UINT target)
{
    m_bSoftAWB = mode;
    m_nAWBTarget = target;
}

void CCap::camerainfo(void)
{
    int value = 0;
    char name[256];

    enumerate_menu(V4L2_CID_EXPOSURE_AUTO);
    strcpy(name, "Exposure Auto");
    get_ctrl(name, V4L2_CID_EXPOSURE_AUTO, &value);

    enumerate_menu(V4L2_CID_POWER_LINE_FREQUENCY);
    strcpy(name, "power line frequency");
    get_ctrl(name, V4L2_CID_POWER_LINE_FREQUENCY, &value);

    enumerate_menu(V4L2_CID_COLORFX);
    strcpy(name, "colorfx");
    get_ctrl(name, V4L2_CID_COLORFX, &value);

    enumerate_menu(V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE);
    strcpy(name, "auto n preset white balance");
    get_ctrl(name, V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE, &value);

    enumerate_menu(V4L2_CID_ISO_SENSITIVITY_AUTO);
    strcpy(name, "iso sensitivity auto");
    get_ctrl(name, V4L2_CID_ISO_SENSITIVITY_AUTO, &value);

    enumerate_menu(V4L2_CID_EXPOSURE_METERING);
    strcpy(name, "exposure metering");
    get_ctrl(name, V4L2_CID_EXPOSURE_METERING, &value);

    enumerate_menu(V4L2_CID_SCENE_MODE);
    strcpy(name, "exposure metering");
    get_ctrl(name, V4L2_CID_EXPOSURE_METERING, &value);

    strcpy(name, "exposure");
    get_ctrl(name, V4L2_CID_EXPOSURE_ABSOLUTE, &value);
    strcpy(name, "sharpness");
    get_ctrl(name, V4L2_CID_SHARPNESS, &value);
    strcpy(name, "brightness");
    get_ctrl(name, V4L2_CID_BRIGHTNESS, &value);
    strcpy(name, "contrast");
    get_ctrl(name, V4L2_CID_CONTRAST, &value);
    strcpy(name, "saturation");
    get_ctrl(name, V4L2_CID_SATURATION, &value);
    strcpy(name, "rotate");
    get_ctrl(name, V4L2_CID_ROTATE, &value);

    int fps = 0;
    getFPS(fps);
}

bool CCap::setOverlay(bool mode)
{
    struct v4l2_framebuffer fb;
    int overlay = (mode == true) ? 1 : 0;

    if (xioctl(VIDIOC_G_FBUF, &fb) == 0)
    {
        LOG_DEBUG << "V4L2_FBUF_CAP_EXTERNOVERLAY    : " << ((V4L2_FBUF_CAP_EXTERNOVERLAY & fb.capability) ? 1 : 0);
        LOG_DEBUG << "V4L2_FBUF_CAP_CHROMAKEY        : " << ((V4L2_FBUF_CAP_CHROMAKEY & fb.capability) ? 1 : 0);
        LOG_DEBUG << "V4L2_FBUF_CAP_LIST_CLIPPING    : " << ((V4L2_FBUF_CAP_LIST_CLIPPING & fb.capability) ? 1 : 0);
        LOG_DEBUG << "V4L2_FBUF_CAP_BITMAP_CLIPPING  : " << ((V4L2_FBUF_CAP_BITMAP_CLIPPING & fb.capability) ? 1 : 0);
        LOG_DEBUG << "V4L2_FBUF_CAP_LOCAL_ALPHA      : " << ((V4L2_FBUF_CAP_LOCAL_ALPHA & fb.capability) ? 1 : 0);
        LOG_DEBUG << "V4L2_FBUF_CAP_GLOBAL_ALPHA     : " << ((V4L2_FBUF_CAP_GLOBAL_ALPHA & fb.capability) ? 1 : 0);
        LOG_DEBUG << "V4L2_FBUF_CAP_LOCAL_INV_ALPHA  : " << ((V4L2_FBUF_CAP_LOCAL_INV_ALPHA & fb.capability) ? 1 : 0);
        LOG_DEBUG << "V4L2_FBUF_CAP_SRC_CHROMAKEY    : " << ((V4L2_FBUF_CAP_SRC_CHROMAKEY & fb.capability) ? 1 : 0);
        LOG_DEBUG << "V4L2_FBUF_FLAG_PRIMARY         : " << ((V4L2_FBUF_FLAG_PRIMARY & fb.flags) ? 1 : 0);
        LOG_DEBUG << "V4L2_FBUF_FLAG_OVERLAY         : " << ((V4L2_FBUF_FLAG_OVERLAY & fb.flags) ? 1 : 0);
        LOG_DEBUG << "V4L2_FBUF_FLAG_CHROMAKEY       : " << ((V4L2_FBUF_FLAG_CHROMAKEY & fb.flags) ? 1 : 0);
        LOG_DEBUG << "V4L2_FBUF_FLAG_LOCAL_ALPHA     : " << ((V4L2_FBUF_FLAG_LOCAL_ALPHA & fb.flags) ? 1 : 0);
        LOG_DEBUG << "V4L2_FBUF_FLAG_GLOBAL_ALPHA    : " << ((V4L2_FBUF_FLAG_GLOBAL_ALPHA & fb.flags) ? 1 : 0);
        LOG_DEBUG << "V4L2_FBUF_FLAG_LOCAL_INV_ALPHA : " << ((V4L2_FBUF_FLAG_LOCAL_INV_ALPHA & fb.flags) ? 1 : 0);
        LOG_DEBUG << "V4L2_FBUF_FLAG_SRC_CHROMAKEY   : " << ((V4L2_FBUF_FLAG_SRC_CHROMAKEY & fb.flags) ? 1 : 0);
        LOG_DEBUG << "base address                   : " << std::hex << fb.base;
        LOG_DEBUG << "width                          : " << fb.fmt.width;
        LOG_DEBUG << "height                         : " << fb.fmt.height;
        LOG_DEBUG << "width                          : " << fb.fmt.width;
        LOG_DEBUG << "height                         : " << fb.fmt.height;
        //        fb.flags &= ~m_set_fbuf;
        //        fb.flags |= fbuf.flags;
#if 0
        if (!fillFBUFFromFB(fb))
        {
            if ( xioctl(VIDIOC_S_FBUF, &fb) < 0 )
            {
                LOG_ERROR << "VIDIOC_S_FBUF error";
                return false;
            }
        }
#endif
                

    }
    else
    {
        LOG_ERROR << "VIDIOC_G_FBUF error";
        return false;
    }

    if (xioctl(VIDIOC_OVERLAY, &overlay) < 0)
    {
        LOG_ERROR << "VIDIOC_OVERLAY error";
        return false;
    }

    LOG_INFO << "overlay : " << ((overlay == 1) ? "start" : "stop");
    return true;
}

bool CCap::matchBitField(const struct fb_bitfield &bf, unsigned off, unsigned len)
{
    if (bf.msb_right || bf.length != len)
        return false;
    return !len || bf.offset == off;
}

int CCap::fillFBUFFromFB(struct v4l2_framebuffer &fb)
{
    struct fb_fix_screeninfo si;
    struct fb_var_screeninfo vi;
    int fb_fd;

    if (m_lpbOverlayBuf == NULL)
        return 0;

    fb_fd = open(m_lpbOverlayBuf, O_RDWR);

    if (fb_fd == -1)
    {
        LOG_ERROR << "cannot open " << m_lpbOverlayBuf;
        return -1;
    }

    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &si))
    {
        LOG_ERROR << "could not obtain fscreeninfo from ", m_lpbOverlayBuf;
        close(fb_fd);
        return -1;
    }

    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vi))
    {
        LOG_INFO << "could not obtain vscreeninfo from ", m_lpbOverlayBuf;
        close(fb_fd);
        return -1;
    }

    fb.base = (void *)si.smem_start;
    fb.fmt.sizeimage = si.smem_len;
    fb.fmt.width = vi.xres;
    fb.fmt.height = vi.yres;
    fb.fmt.bytesperline = si.line_length;

    if (fb.fmt.height * fb.fmt.bytesperline > fb.fmt.sizeimage)
    {
        LOG_ERROR << "height * bytesperline > sizeimage?!";
        close(fb_fd);
        return -1;
    }

    fb.fmt.pixelformat = 0;

    if ((si.capabilities & FB_CAP_FOURCC) && vi.grayscale > 1)
    {
        fb.fmt.pixelformat = vi.grayscale;
        fb.fmt.colorspace = vi.colorspace;
    }
    else
    {
        if (vi.grayscale == 1)
        {
            if (vi.bits_per_pixel == 8)
                fb.fmt.pixelformat = V4L2_PIX_FMT_GREY;
        }
        else
        {
            for (int i = 0; fb_formats[i].pixfmt; i++)
            {
                const struct bitfield2fmt &p = fb_formats[i];

                if (matchBitField(vi.blue, p.blue_off, p.blue_len) &&
                    matchBitField(vi.green, p.green_off, p.green_len) &&
                    matchBitField(vi.red, p.red_off, p.red_len) &&
                    matchBitField(vi.transp, p.transp_off, p.transp_len))
                {
                    fb.fmt.pixelformat = p.pixfmt;
                    break;
                }
            }
        }

        fb.fmt.colorspace = V4L2_COLORSPACE_SRGB;
    }

    close(fb_fd);
    return 0;
}

#if 0
void CCap::setFrameBuffer(int framecount)
{
    struct v4l2_format fmt;
    if (xioctl(VIDIOC_G_FMT, &fmt) == -1)
        errno_exit("VIDIOC_G_FMT");

    fmt.fmt.pix.widt
    fmt.fmt.pix.height;
    fmt.fmt.pix.pixelformat;



}

int CCap::getFrameData(LPBYTE lpdata, UINT event)
{
    struct v4l2_format fmt;
    if (xioctl(VIDIOC_G_FMT, &fmt) == -1)
        errno_exit("VIDIOC_G_FMT");

    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = vformat;

    m_lpFrameData = ;
    m_bGetFrame = true;
}
#endif
/*
    open_device();
    init_device();
    start_capturing();
    gettimeofday(&Start, NULL);
    mainloop();
    gettimeofday(&End, NULL);
    stop_capturing();
    uninit_device();
    close_device();
*/