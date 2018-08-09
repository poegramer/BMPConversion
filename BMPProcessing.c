#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma warning(disable:4996)
#pragma pack(2)

#ifdef _WIN32
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef long           LONG;
typedef unsigned char  BYTE;
#else
typedef unsigned short WORD;
typedef unsigned int   DWORD;
typedef int            LONG;
typedef unsigned char  BYTE;
#endif

// BMP文件头
typedef struct tagBITMAPFILEHEADER
{
        WORD    bfType;        // 2Bytes，必须为"BM"，即0x424D 才是Windows位图文件
        DWORD   bfSize;        // 4Bytes，整个BMP文件的大小
        WORD    bfReserved1;   // 2Bytes，保留，为0
        WORD    bfReserved2;   // 2Bytes，保留，为0
        DWORD   bfOffBits;     // 4Bytes，文件起始位置到图像像素数据的字节偏移量
} BITMAPFILEHEADER;

// BMP信息头
typedef struct tagBITMAPINFOHEADER
{
        DWORD      biSize;           // 4Bytes，INFOHEADER结构体大小，存在其他版本INFOHEADER，用作区分
        LONG       biWidth;          // 4Bytes，图像宽度（以像素为单位）
        LONG       biHeight;         // 4Bytes，图像高度，+：图像存储顺序为Bottom2Top，-：Top2Bottom
        WORD       biPlanes;         // 2Bytes，图像数据平面，BMP存储RGB数据，因此总为1
        WORD       biBitCount;       // 2Bytes，图像像素位数
        DWORD      biCompression;    // 4Bytes，0：不压缩，1：RLE8，2：RLE4
        DWORD      biSizeImage;      // 4Bytes，4字节对齐的图像数据大小
        LONG       biXPelsPerMeter;  // 4Bytes，用像素/米表示的水平分辨率
        LONG       biYPelsPerMeter;  // 4Bytes，用像素/米表示的垂直分辨率
        DWORD      biClrUsed;        // 4Bytes，实际使用的调色板索引数，0：使用所有的调色板索引
        DWORD      biClrImportant;   // 4Bytes，重要的调色板索引数，0：所有的调色板索引都重要
} BITMAPINFOHEADER;

// BMP调色板
typedef struct tagRGBQUAD
{
        BYTE    rgbBlue;      // 指定蓝色强度
        BYTE    rgbGreen;     // 指定绿色强度
        BYTE    rgbRed;       // 指定红色强度
        BYTE    rgbReserved;  // 保留，设置为0
} RGBQUAD;


BITMAPFILEHEADER bitFileHead;
BITMAPINFOHEADER bitInfoHead;
RGBQUAD *quad;
unsigned char *pColor;

/**
* 函数名：PxlToBMP1
* 功  能：处理像素点函数，将所有像素点的灰度值值按阈值进行二值化，转化为二进制的0或1，即1bit位图的像素点值
* 参  数：img        --- 所有的像素点数据
         width      --- 位图宽度
         height     --- 位图高度
         nThreshold --- 二值化阈值
* 返回值：转换后1bit位图的像素值
**/
unsigned char* PxlToBMP1(unsigned char* img, int width, int height, int nThreshold)
{
    unsigned char* dst=NULL;  
    unsigned char* ptr = img;  
    unsigned char* p;
	unsigned char  buf;
    unsigned char pxl;  
	
    int offset;  
    int i;
    int j;
    int k;
    unsigned int size;  
	
    int src_row_bytes;  
    int dst_row_bytes;  
    int dst_width;  
    int dst_height;  
	
    src_row_bytes = (width*bitInfoHead.biBitCount+31)/8/4*4;
    dst_row_bytes = (width+31)/8/4*4;

    size = dst_row_bytes * height;  
    dst = (unsigned char*)malloc(size);  
    if(!dst)  
        return NULL;  
    memset(dst, 0, size);  
	
    p = dst;  
    dst_height = height;  
    dst_width = dst_row_bytes;

	int m = 0;
	int n = 0;
	int tmp = 0;

	if (bitInfoHead.biBitCount == 1)
	{
		return img;
	}
	if (bitInfoHead.biBitCount == 4)
	{
		for (i = 0; i < height; i++)
		{
			offset = 0;
			for (j = 0; j < dst_width; j++)
			{
				pxl = 0;
				n = 0;
				for (k = 7; k >= 0; k--)
				{
					n++;
					if (n%2)
					{
						tmp = (*ptr >> 4) & 0x0F;
						if(pColor[tmp] > nThreshold)
						{
							pxl = pxl | (0x01 << k);  
						}
						//printf("%-3d ", pColor[*ptr]);
						offset++;
						if (offset >= width)
						{
							ptr++;
							break;
						}
					}
					else
					{
						tmp = *ptr & 0x0F;
						if(pColor[tmp] > nThreshold)
						{
							pxl = pxl | (0x01 << k);  
						}
						//printf("%-3d ", pColor[*ptr]);
						ptr++;
						offset++;
						if (offset >= width)
							break;
					}
				}
				p[m++] = pxl;
				//printf("-> %d\n", pxl);
			}
		}
	}
	else if (bitInfoHead.biBitCount == 8)
	{
		for (i = 0; i < height; i++)
		{
			offset = 0;
			for (j = 0; j < dst_width; j++)
			{
				pxl = 0;
				for (k = 7; k >= 0; k--)
				{
					tmp = *ptr;
					if(pColor[tmp] > nThreshold)
					{
						pxl = pxl | (0x01 << k);  
					}
					//printf("%-3d ", pColor[*ptr]);
					ptr++;
					offset++;
					if (offset >= width)
						break;
				}
				p[m++] = pxl;
				//printf("-> %d\n", pxl);
			}
		}
	}
	else
	{
		for (i = 0; i < height; i++)
		{
			offset = 0;
			for (j = 0; j < dst_width; j++)
			{
				pxl = 0;  
				for(k = 7; k >= 0;k--)  
				{
					//printf("%02X  %02X  %02X\n", *ptr, *(ptr + 1), *(ptr + 2));
					buf = (BYTE)(0.114*(*ptr) + 0.587*(*(ptr+1)) + 0.299*(*(ptr+2)));//灰度化
					if(buf > nThreshold)  
					{
						pxl = pxl | (0x01 << k);  
					}
					//printf("%-3d ", buf);
					ptr += 3;
					offset++;
					if(offset >= width)  
						break;
				}  
				p[m++] = pxl;
				//printf("-> %d\n", pxl);
			}
		}
	}

    return dst;
}

/**
* 函数名：WriteBMP1File
* 功  能：将新的像素值写入1bit位图文件中
* 参  数：pxldata  --- 新的像素点数据
         fileName --- 写入的文件名
         width    --- 位图宽度
         height   --- 位图高度
* 返回值：0   --- 成功
        其他 --- 失败
**/
int WriteBMP1File(unsigned char* pxldata, const char* fileName, int width,int height)
{
	BITMAPFILEHEADER fileHead;  
    BITMAPINFOHEADER head;  
    RGBQUAD quad[2];  
    int colorTablesize;  
    FILE *file = NULL;  
    int rowBytes;  
    int offset;  
	
	file = fopen(fileName, "wb");  
    if(!file)  
        return -1;  
	
	rowBytes = (width+31)/8/4*4;
    colorTablesize = 8;  
	
    //申请位图文件头结构变量，填写文件头信息  
    fileHead.bfType = 0x4D42;//bmp类型  
    //bfSize是图像文件4个组成部分之和  
    fileHead.bfSize= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)+ colorTablesize + rowBytes* height;  
    fileHead.bfReserved1 = 0;  
    fileHead.bfReserved2 = 0;  
    //bfOffBits是图像文件前3个部分所需空间之和  
    fileHead.bfOffBits=54+colorTablesize;  
	
	unsigned char *data = (unsigned char *)malloc(fileHead.bfSize);
	
    //写文件头进文件  
    memcpy(data, &fileHead, sizeof(BITMAPFILEHEADER));  
    //申请位图信息头结构变量，填写信息头信息  
    offset = sizeof(BITMAPFILEHEADER);  
	fwrite(data, offset, 1, file);
	
    head.biBitCount= 1;  
    head.biClrImportant=0;  
    head.biClrUsed=0;  
    head.biCompression=0;  
    head.biHeight= height;  
    head.biPlanes=1;  
    head.biSize=40;  
    head.biSizeImage= rowBytes * height;  
    head.biWidth= width;  
    head.biXPelsPerMeter=bitInfoHead.biXPelsPerMeter;  
    head.biYPelsPerMeter=bitInfoHead.biYPelsPerMeter;
    //写位图信息头进内存  
    //fwrite(&head, sizeof(BitmapHeader),1, file);  
    memcpy(data, &head, sizeof(BITMAPINFOHEADER));  
    offset = sizeof(BITMAPINFOHEADER);  
	fwrite(data, offset, 1, file);
	
    //如果灰度图像，有颜色表，写入文件   	
    quad[0].rgbBlue = 0;  
    quad[0].rgbGreen = 0;  
    quad[0].rgbRed = 0;  
    quad[0].rgbReserved = 0;  
	
    memcpy(data, &quad[0], sizeof(RGBQUAD));  
    offset = sizeof(RGBQUAD);  
	fwrite(data, offset, 1, file);
	
    quad[1].rgbBlue = 255;  
    quad[1].rgbGreen = 255;  
    quad[1].rgbRed = 255;  
    quad[1].rgbReserved = 0;  
	
    memcpy(data,&quad[1],sizeof(RGBQUAD));
	fwrite(data, offset, 1, file);
	
    fwrite(pxldata, head.biSizeImage, 1, file);  
    fclose(file);

	return 0;
}

/**
* 函数名：ChangeToBmp1
* 功  能：位图转换主函数
* 参  数：srcBmp_file --- 源文件路径
         dstBmp_file --- 目的文件路径
         nThreshold  --- 二值化阈值
* 返回值：0   --- 成功
        其他 --- 失败
**/
int ChangeToBmp1(const char *srcBmp_file, const char *dstBmp_file, int nThreshold)
{
	FILE* file;
	
	if ((file=fopen(srcBmp_file, "rb")) == NULL)
	{
		return -1;
	}
	
	fread(&bitFileHead,1,sizeof(BITMAPFILEHEADER),file);
	fread(&bitInfoHead,1,sizeof(BITMAPINFOHEADER),file);

	if (bitFileHead.bfType != 0x4D42)
	{
		return -2;
	}

	
	//图片信息展示
	printf("****************************************************\n");
	printf("BITMAPFILEHEADER.bfType          = %04X\n", bitFileHead.bfType);
	printf("BITMAPFILEHEADER.bfSize          = %08X\n", bitFileHead.bfSize);
	printf("BITMAPFILEHEADER.bfSize          = %04X\n", bitFileHead.bfReserved1);
	printf("BITMAPFILEHEADER.bfSize          = %04X\n", bitFileHead.bfReserved2);
	printf("BITMAPFILEHEADER.bfOffBits       = %08X\n", bitFileHead.bfOffBits);
	printf("****************************************************\n");
	printf("BITMAPINFOHEADER.biSize          = %08X\n", bitInfoHead.biSize);
	printf("BITMAPINFOHEADER.biWidth         = %08X\n", bitInfoHead.biWidth);
	printf("BITMAPINFOHEADER.biHeight        = %08X\n", bitInfoHead.biHeight);
	printf("BITMAPINFOHEADER.biPlanes        = %04X\n", bitInfoHead.biPlanes);
	printf("BITMAPINFOHEADER.biBitCount      = %04X\n", bitInfoHead.biBitCount);
	printf("BITMAPINFOHEADER.biCompression   = %08X\n", bitInfoHead.biCompression);
	printf("BITMAPINFOHEADER.biSizeImage     = %08X\n", bitInfoHead.biSizeImage);
	printf("BITMAPINFOHEADER.biXPelsPerMeter = %08X\n", bitInfoHead.biXPelsPerMeter);
	printf("BITMAPINFOHEADER.biYPelsPerMeter = %08X\n", bitInfoHead.biYPelsPerMeter);
	printf("BITMAPINFOHEADER.biClrUsed       = %08X\n", bitInfoHead.biClrUsed);
	printf("BITMAPINFOHEADER.biClrImportant  = %08X\n", bitInfoHead.biClrImportant);
	printf("****************************************************\n");
	
	
	int nColorUsed = 0;
    int i = 0, j = 0;
    if (bitInfoHead.biBitCount < 16)
    {
        // 读取使用的调色板颜色数量,biClrUsed为0时，代表位数默认的调色板值
        nColorUsed = bitInfoHead.biClrUsed ? bitInfoHead.biClrUsed : (1 << bitInfoHead.biBitCount);
        if (nColorUsed > 255)
            nColorUsed = 0;
    }
	// 读取调色板
    if (nColorUsed > 0)//排除24位和32位没有调色板的
    {
        quad = (RGBQUAD*)malloc(sizeof(RGBQUAD)*nColorUsed);
        fread(quad, sizeof(RGBQUAD)*nColorUsed, 1, file);
		
		pColor = (BYTE*)malloc(nColorUsed);
        for (i = 0; i < nColorUsed; i++)
        {
			pColor[i]= (BYTE)(0.299 * quad[i].rgbRed + 0.587 * quad[i].rgbGreen + 0.114 * quad[i].rgbBlue);//灰度化
        }
    }
    DWORD srcW = bitInfoHead.biWidth;
    DWORD srcH = bitInfoHead.biHeight;
	
	//图像数据区域
	int BufSize = bitInfoHead.biSizeImage;
	BYTE* Buf = (BYTE*)malloc(BufSize);
    fread(Buf, BufSize, 1, file);
	
	BYTE* DestBuf = (BYTE*)malloc(BufSize);
	int dest_width = srcW;
	int dest_height = srcH;
	DestBuf = PxlToBMP1(Buf, srcW, srcH, nThreshold);
	if (!DestBuf)
	{
		return -3;
	}
	
	if (WriteBMP1File(DestBuf, dstBmp_file, dest_width, dest_height))
	{
		return -4;
	}
	
	return 0;
}


int main(int argc, char const *argv[])
{
	int nThreshold = 190;
	char szSrcFile[260] = {0};
	char szDstFile[260] = {0};

	if (argc < 3)
	{
		if (argc == 2 && strcmp(argv[1], "--help") == 0)
		{
			printf("Usage: %s srcFile dstFile [options]\n", argv[0]);
			printf("Options:\n");
			printf("  --help          Display this information\n");
			printf("  -t <threshold>  Assume the threshold of pixels, the default value is 190\n");
		}
		else
		{
			printf("error: unrecognized command line option '%s'\n", argv[1]);
			printf("\"%s --help\" can help you to find the correct usage.\n", argv[0]);
		}

		return 0;
	}
	else if (argc == 3)
	{
		strcpy(szSrcFile, argv[1]);
		strcpy(szDstFile, argv[2]);
	}
	else if (argc == 5)
	{
	 	if (strcmp(argv[3], "-t") != 0)
	 	{
			printf("%s\n", argv[3]);
	 		printf("error: unrecognized command line option '%s'\n", argv[3]);
			printf("\"%s --help\" can help you to find the correct usage.\n", argv[0]);
	 		return 0;
	 	}

	 	nThreshold = atoi(argv[4]);
	 	strcpy(szSrcFile, argv[1]);
	 	strcpy(szDstFile, argv[2]);
	}
	else
	{
		printf("error: unrecognized command line option.\n");
		printf("\"%s --help\" can help you to find the correct usage.\n", argv[0]);
		return 0;
	}

	int nLen = strlen(szDstFile);
	if (strcmp(&szDstFile[nLen-3], "bmp") != 0)
	{
		printf("format of the destination file is incorrect, the suffix should be '.bmp'.\n");
		return 0;
	}

	if (nThreshold < 0 || nThreshold > 255)
	{
		printf("threshold value is invalid, it should be between 0 to 255.\n");
		return 0;
	}

	int nRet = 0;
	nRet = ChangeToBmp1(szSrcFile, szDstFile, nThreshold);
	if (nRet == -1)
	{
		printf("fail to open the source bmp file.\n");
	}
	else if (nRet == -2)
	{
		printf("format of the source file is incorrect，it's not a bmp file.\n");
	}
	else if (nRet == -3)
	{
		printf("fail to deal with the pixels.\n");
	}
	else if (nRet == -4)
	{
		printf("fail to create the destination file.\n");
	}

	return 0;
}
