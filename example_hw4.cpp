// Student ID:
// Name      :
// Date      : 
#include "bmpReader.h"
#include "bmpReader.cpp"
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
using namespace std;

#define MYRED	2
#define MYGREEN 1
#define MYBLUE	0

int imgWidth, imgHeight;
int MEAN_FILTER_SIZE=9;
int SOBEL_FILTER_SIZE;
int FILTER_SCALE;
int *filter_gx, *filter_gy;

const char *inputfile_name[5] = {
	"input1.bmp",
	"input2.bmp",
	"input3.bmp",
	"input4.bmp",
	"input5.bmp"
};
const char *outputMed_name[5] = {
	"output1.bmp",
	"output2.bmp",
	"output3.bmp",
	"output4.bmp",
	"output5.bmp"
};


unsigned char *pic_in, *pic_grey, *pic_mean, *pic_gx, *pic_gy, *pic_sobel,*pic_final;

unsigned char RGB2grey(int w, int h)
{
	int tmp = (
		pic_in[3 * (h*imgWidth + w) + MYRED] +
		pic_in[3 * (h*imgWidth + w) + MYGREEN] +
		pic_in[3 * (h*imgWidth + w) + MYBLUE] )/3;

	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}

unsigned char MeanFilter(int w, int h)
{
	int tmp = 0;
	int a, b , window[9],k=0,sum=0;
	int ws = (int)sqrt((float)MEAN_FILTER_SIZE);
	for (int j = 0; j<ws; j++)
	for (int i = 0; i<ws; i++)
	{
		a = w + i - (ws / 2);
		b = h + j - (ws / 2);

		// detect for borders of the image
		if (a<0 || b<0 || a>=imgWidth || b>=imgHeight)continue;

		sum=sum+pic_grey[b*imgWidth + a];
	};

	tmp=sum/MEAN_FILTER_SIZE;
	
	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}

unsigned char gx_sobelFilter(int w, int h)
{
	int tmp = 0;
	int a, b;
	int ws = (int)sqrt((float)SOBEL_FILTER_SIZE);
	for (int j = 0; j<ws; j++)
	for (int i = 0; i<ws; i++)
	{
		a = w + i - (ws / 2);
		b = h + j - (ws / 2);

		// detect for borders of the image
		if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;

		tmp += filter_gx[j*ws + i] * pic_mean[b*imgWidth + a];
	};
	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}

unsigned char gy_sobelFilter(int w, int h)
{
	int tmp = 0;
	int a, b;
	int ws = (int)sqrt((float)SOBEL_FILTER_SIZE);
	for (int j = 0; j<ws; j++)
	for (int i = 0; i<ws; i++)
	{
		a = w + i - (ws / 2);
		b = h + j - (ws / 2);

		// detect for borders of the image
		if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;

		tmp += filter_gy[j*ws + i] * pic_mean[b*imgWidth + a];
	};
	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}

unsigned char sobelFilter(int w, int h)
{
	int tmp = 0;
	tmp = sqrt(pic_gx[h*imgWidth + w]*pic_gx[h*imgWidth + w] + pic_gy[h*imgWidth + w]*pic_gy[h*imgWidth + w]);
	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}


int main()
{
	// read mask file
	FILE* mask;

	mask = fopen("mask_Sobel.txt", "r");
	fscanf(mask, "%d", &SOBEL_FILTER_SIZE);

	filter_gx = new int[SOBEL_FILTER_SIZE];
	filter_gy = new int[SOBEL_FILTER_SIZE];

	for (int i = 0; i<SOBEL_FILTER_SIZE; i++)
		fscanf(mask, "%d", &filter_gx[i]);

	for (int i = 0; i<SOBEL_FILTER_SIZE; i++)
		fscanf(mask, "%d", &filter_gy[i]);

	fclose(mask);
	
	BmpReader* bmpReader = new BmpReader();
	for (int k = 0; k<5; k++){
		// read input BMP file
		pic_in = bmpReader->ReadBMP(inputfile_name[k], &imgWidth, &imgHeight);
		// allocate space for output image
		pic_grey = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_mean = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_gx = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_gy = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_sobel = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_final = (unsigned char*)malloc(3 * imgWidth*imgHeight*sizeof(unsigned char));
	
		//convert RGB image to grey image
		for (int j = 0; j<imgHeight; j++) {
			for (int i = 0; i<imgWidth; i++){
				pic_grey[j*imgWidth + i] = RGB2grey(i, j);
			}
		}
		
		//apply the Mean filter to the image
		for (int j = 0; j<imgHeight; j++) {
			for (int i = 0; i<imgWidth; i++){
				pic_mean[j*imgWidth + i] = MeanFilter(i, j);		
			}
		}

		//apply the gx_sobel filter to the image
		for (int j = 0; j<imgHeight; j++) {
			for (int i = 0; i<imgWidth; i++){
				pic_gx[j*imgWidth + i] = gx_sobelFilter(i, j);
			}
		}

		//apply the gy_sobel filter to the image 
		for (int j = 0; j < imgHeight; j++){
			for (int i = 0; i < imgWidth; i++){
				pic_gy[j*imgWidth + i] = gy_sobelFilter(i, j);
			}
		};

		//apply the sobel filter to the image 
		for (int j = 0; j < imgHeight; j++){
			for (int i = 0; i < imgWidth; i++){
				pic_sobel[j*imgWidth + i] = sobelFilter(i, j);
			}
		};

		//extend the size form WxHx1 to WxHx3
		for (int j = 0; j<imgHeight; j++) {
			for (int i = 0; i<imgWidth; i++){
				pic_final[3 * (j*imgWidth + i) + MYRED] = pic_sobel[j*imgWidth + i];
				pic_final[3 * (j*imgWidth + i) + MYGREEN] = pic_sobel[j*imgWidth + i];
				pic_final[3 * (j*imgWidth + i) + MYBLUE] = pic_sobel[j*imgWidth + i];
			}
		}

		bmpReader->WriteBMP(outputMed_name[k], imgWidth, imgHeight, pic_final);

		//free memory space
		free(pic_in);
		free(pic_grey);
		free(pic_mean);
		free(pic_final);
		free(pic_sobel);
		free(pic_gx);
		free(pic_gy);
	}

	return 0;
}