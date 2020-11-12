/*
  -- datatype.h --
*/

#ifndef __DATATYPE_H__

#define __DATATYPE_H__

#ifndef _WIN32

#include <linux/types.h>

#endif

#ifndef NULL

#define NULL  0

#endif
//#pragma pack(2)
//#define FAR			far
//#define NEAR			near

typedef int BOOL;
//typedef int near *	PBOOL;
//typedef int far *		LPBOOL;

typedef unsigned char BYTE;
typedef BYTE *PBYTE;
typedef BYTE *LPBYTE;

typedef unsigned short WORD;
typedef WORD *PWORD;
typedef WORD *LPWORD;

typedef unsigned long DWORD;
typedef DWORD *PDWORD;
typedef DWORD *LPDWORD;

typedef int *LPINT;

typedef unsigned int UINT;
typedef UINT *LPUINT;

typedef signed long LONG;

typedef long long INT64;
typedef INT64 *LPINT64;

typedef unsigned long long UINT64;
typedef UINT64 *LPUINT64;

#define LOBYTE(w)			((BYTE)(w))
#define HIBYTE(w)			((BYTE)(((UINT)(w) >> 8) & 0xFF))
#define LOWORD(l)			((WORD)(DWORD)(l))
#define HIWORD(l)			((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))

#define MAKELONG(low, high) ((LONG)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))

#ifndef NOMINMAX
#ifndef max
#define max(a,b)			(((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)			(((a) < (b)) ? (a) : (b))
#endif
#endif	/* NOMINMAX */

#define PI (double)3.141592653589793238462643383279
#define FPI (float)3.141592653589793238462643383279
#define RAD (double)3.141592653589793238462643383279 / (double)180.0
#define FRAD (float)3.141592653589793238462643383279 / (float)180.0

typedef enum _labelmode {
	NORMAL,
	AREA,
	LEFT,
	TOP,
	LENGTH,
	LAREA,
	LLEFT,
	LTOP,
	FEATUREONLY
} LABELMODE;

typedef struct tagPOINT {
   LONG x;				// 4
   LONG y;				// 4
} POINT;				// 8

typedef POINT * LPPOINT;

typedef struct tagSIZE
{
	long        cx;
	long        cy;
} SIZE, *PSIZE, *LPSIZE;

typedef struct tagRECT {
   LONG left;			// 4
   LONG top;			// 4
   LONG right;			// 4
   LONG bottom;			// 4
} RECT, *PRECT, *LPRECT;// 16

typedef struct tagRECTdw {
   DWORD left;			// 4
   DWORD top;			// 4
   DWORD right;			// 4
   DWORD bottom;			// 4
} RECTDW;					// 16

typedef RECTDW * LPRECTDW;

typedef struct _FEATURE
{
	int order;
	int x;
	int y;
} FEATURE;
typedef FEATURE *LPFEATURE;

typedef struct _count
{
	int nNo;
	int nCount;
	POINT ptCenter;
	RECT rect;
	POINT ptLS;
	POINT ptLE;
	POINT ptWS;
	POINT ptWE;
	POINT ptGravityCenter;
	double length;
	double width;
	double angle;
	double angler;
	int nFeatureCount;
	LPFEATURE lpFeature;
	int nSyuui;
	int nData;
} COUNT;
typedef COUNT *LPCOUNT;

typedef struct _PNTS
{
	POINT pt;
	struct _PNTS *next;
} POINTSF;
typedef POINTSF *LPPOINTSF;

typedef struct _recivedata {
	BYTE	bData[2];
	BYTE	bDummy[2];
	int		nSize;
	int		nCommand;
	BYTE	bData0[4];
} RECIVEDATA;
typedef RECIVEDATA * LPRECIVEDATA;

typedef struct _trance_data {
	BYTE bHead[2];
	BYTE bDummy[2];
	int  nSize;
	int  nCommand;
	BYTE bData0[4];
} TRANCEDATA;
typedef TRANCEDATA * LPTRANCEDATA;

typedef struct cameradata {
	int	nType;
	int nClock;
	int nStartCycle;
	int nMaxLineCount;
	int nBackLineCount;
	int nAriThresh;
	int nAriBit;
	int nTrigBit;
} CAMERADATA;
typedef CAMERADATA * LPCAMERADATA;

typedef struct _field_param_ocr {
	int	nImageSave;
	int	nRotationMode;
	int	nLength;	// ���
	int	nThresh1;	// ���ͣ�
	int	nThresh2;	// ���ͣ�
	int	nMatrixCenter;	// �ޥȥ�å�����
	int nMachLimit;
	int	nUnMachLimit;
	int	nWHLimit;
	int	nPointLimit;
	int	nCrossLimit;
	int nOCRMode;
	char cOcrFontName[256];
	char cOcrFontName0[256];
} FPOCR, *LPFPOCR;

typedef struct _field_param_hanko {
	int		nImageSave;
	int		nThV;		// �ϥ� ����1
	int		nThK;		// �ϥ� ����2
	int		nThV_Area;	// �ϥ� ���ꥢ����1
	int		nThK_Area;	// �ϥ� ���ꥢ����2
//	int		nDummy[5];
//	BYTE	bDummy[256];
} FPHANKO, *LPFPHANKO;

typedef struct _field_param_color {
	int		nImageSave;
	int		nThCUMax;	//
	int		nThCUMin;	//
	int		nThCVMax;	//
	int		nThCVMin;	//
//	int		nDummy[5];
//	BYTE	bDummy[256];
} FPCOLOR, *LPFPCOLOR;

typedef union _field_params {
	FPOCR	ocr;
	FPHANKO	hanko;
	FPCOLOR color;
} FPS, *LPFPS;

#define MODE_IMAGE	1
#define MODE_OCR	2
#define MODE_COLOR	3
#define MODE_BCR	4
#define MODE_HANKO	5
#define MODE_SELECT	6

typedef struct _field_param {
	int		nMode;		// 1:Image 2:OCR 3:Color 4:BCR 5:Hanko 6:Select
	FPS		p;			//
	RECTDW	rc;			// �ե�������ϰ�
} FEILDPARAM, *LPFIELDPARAM;

typedef struct _paper_param {
	int		nPaperNumber;
	int		nWidth;
	int		nHeight;
	int		nWHLimit;
	int		nFieldCount;
	int		nMode;
	int		nAgreement;
	int		nMaxV0[25];
	int		nMaxU0[25];
	int		nMaxV1[25];
	int		nMaxU1[25];
	FEILDPARAM	Field[100];
} PAPERPARAM, *LPPARERPARAM;

typedef struct _paper_param2 {
	PAPERPARAM	PaperParam;
	int			dicno[100];
} PAPERPARAM2, *LPPAPERPARAM2;

#define MODE_IMAGE	1
#define MODE_OCR	2
#define MODE_COLOR	3
#define MODE_BCR	4
#define MODE_HANKO	5
#define MODE_SELECT	6

/*
typedef struct areadata {
	int nStart;
	int nEnd;
} AREADATA;

typedef AREADATA *	LPAREADATA;

typedef struct facebackarea {
	int	nArea[4];
	int	nFilter[4];
	int	nPickup[4];
	int	nThreshA[4];
	int	nThreshB[4];
} FACEBACKAREA;

typedef struct facebackarealimit {
	int	nMinSize[4];
	int	nMaxSize[4];
	int	nTotalCount[4];
	int	nTotalAreaSize[4];
} FACEBACKAREALIMIT;

typedef struct facebackareadata {
	FACEBACKAREA		XUp;
	int					nCAreaSel;
	int					nThreshC;
	int					nPrintSize;
	FACEBACKAREA		XDown;
	FACEBACKAREA		YUp;
	FACEBACKAREA		YDown;
	FACEBACKAREALIMIT	UpLimit;
	FACEBACKAREALIMIT	DownLimit;
	int					nCenterThresh;
//	int					nCenterLine;
	int					nLightSet;
	int					nMask;
	int					nWidthMinLimit;
	int					nWidthMaxLimit;
	int					nHeightMinLimit;
	int					nHeightMaxLimit;
	int					nLightMinLimit;
	int					nLightMaxLimit;
	int					nTotalMinSize;
} FACEBACKAREADATA;

typedef FACEBACKAREADATA * LPFBAREADATA;

typedef struct sideareadata {
	int	nWidthLevel;
	int	nWidthSabunStep;
	int	nWidthDeterminLevel;
	int	nWidthDeterminCount;
	int	nWidthMinLimit;
	int	nWidthMaxLimit;
	int	nUmuLevel;
	int	nUmuDeterminSize;
	int	nCenterThresh;
	int nMaskThresh;
	int nSyuukiEdit;
	int nMaskEdit;
	int nMaskStart;
	int nHLM;
	int nLLM;
	int nLeftHLV;
	int nLeftLLV;
	int nLeftRLV;
	int nCenterHLV;
	int nCenterLLV;
	int nCenterRLV;
	int nRightHLV;
	int nRightLLV;
	int nRightRLV;
	int nArea0;
	int nArea1;
	int nArea0X;
	int nArea1X;
	int nArea0Y;
	int nArea1Y;
	int nLeftXSub;
	int nCenterXSub;
	int nRightXSub;
	int nLeftYSub;
	int nCenterYSub;
	int nRightYSub;
	int nLeftXLV;
	int nCenterXLV;
	int nRightXLV;
	int nLeftYLV;
	int nCenterYLV;
	int nRightYLV;
	int nMaskWidthMin;
	int nNGCountLineL;
	int nNGCountLineC;
	int nNGCountLineR;
	int nNGCountAllL;
	int nNGCountAllC;
	int nNGCountAllR;
	int nMaskCount;
	int nNGMaskCountMax;
	int nMaskUse;
} SIDEAREADATA;

typedef SIDEAREADATA * LPSIDEAREADATA;
*/
#pragma pack(2)
typedef struct tagRGBQUAD {
	BYTE	rgbBlue;
	BYTE	rgbGreen;
	BYTE	rgbRed;
	BYTE	rgbReserved;
} RGBQUAD;
typedef RGBQUAD * LPRGBQUAD;

typedef struct tagBITMAPINFOHEADER {
	DWORD      biSize;
	long       biWidth;
	long       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	long       biXPelsPerMeter;
	long       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagBITMAPINFO {
	BITMAPINFOHEADER    bmiHeader;
	RGBQUAD             bmiColors[1];
} BITMAPINFO, *LPBITMAPINFO, *PBITMAPINFO;

typedef struct tagBITMAPFILEHEADER {
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;
#pragma pack()

//typedef struct _JpegErrorManageStruct {
//	struct jpeg_error_mgr pub;
//	jmp_buf jb;
//} JpegErrorManageStruct, *PJpegErrorManageStruct;
/*
typedef struct _dibitmap {
	int width;
	int height;
	int bpp;
	int pitch;
	int bitmapsize;
	LPBYTE lpbData;
	int labelmemsize;
	int labelCount;
	RECT rcLabelArea;
	LPINT lpnLabelMem;
	LPCOUNT lpLabelCount;
} DIBITMAP, *LPDIBITMAP;
*/
#if !defined FALSE

#define FALSE  0
#define TRUE   1

#endif

#endif



