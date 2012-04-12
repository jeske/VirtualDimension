#include "stdafx.h"
#include <memory.h>
#include <math.h>
#include "Scaling.h"


void Scaling::GetDefaultBitmapInfo(BITMAPINFO& bi, int w, int h)
{
	static BITMAPINFOHEADER bih = {
		sizeof(BITMAPINFOHEADER), // DWORD      biSize;
        0, // LONG       biWidth;
        0, // LONG       biHeight;
        1, // WORD       biPlanes;
        24, // WORD       biBitCount;
        BI_RGB, // DWORD      biCompression;
        0, // DWORD      biSizeImage;
        0, // LONG       biXPelsPerMeter;
        0, // LONG       biYPelsPerMeter;
        0, // DWORD      biClrUsed;
        0 // DWORD      biClrImportant;
	};
	memmove(&bi, &bih, sizeof(bih));
	bi.bmiHeader.biWidth = w;
	bi.bmiHeader.biHeight = h;
	bi.bmiHeader.biSizeImage = w*h*3;
}

int Scaling::BestFit(int w, int h, int maxW, int maxH)
{
	float inAR = float(w)/h;
	float outAR = float(maxW)/maxH;
	float diff = (inAR-outAR)/outAR;

	if (diff < -0.01f)
		return int( float(maxH) * inAR );
	return maxW;
}


namespace
{
	void ScaleHorizontally(const void* buf, int w, int h, void* outBuf, int outW)
	{
		float factor = float(w) / outW;
		
		int depth = int(ceil(factor)) + 1;
		int** dist;
		int* weights = new int[outW * depth];

		//float* centers = new float[outW];


		for (int i=0; i<outW; ++i) {
			float center = factor * (0.5f + float(i));
		}

		//delete [] centers;
		delete [] weights;


		dist = new int*[5];
		dist[0] = new int[outW];
		dist[1] = new int[outW];
		dist[2] = new int[outW];
		dist[3] = new int[outW];
		dist[4] = new int[outW];
	//	m_distW[2] = new int[m_outW];

		float stretchW = (float)w/outW;
		for (int i=0; i<outW; ++i)
		{
			float center = stretchW*(0.5f + (float)i) - 0.5f;
			float base = floor(center);
			dist[0][i] = (int)(base + 0.5f);
			float baseDist = center - base;

			float weights[4];
			//int n=0;
			float sum = 0.0f;
			for (int j=0; j<4; ++j)
			{
				if (i==0 && j==0 || i==outW-1 && j==3)
					weights[j] = -1;
				else
				{
					weights[j] = WeightFunc(baseDist - j + 1);
					sum += weights[j];
				//	++n;
				}
			}

			for (int j=1; j<5; ++j)
			{
				dist[j][i] = IntWeight(weights[j-1]/sum);
			}
		}
	}

	void ScaleVertically(const void* buf, int w, int h, void* outBuf, int outH)
	{
	}
}

void Scaling::Scale(const void* buf, int w, int h, void* outBuf, int outW, int outH)
{
}
