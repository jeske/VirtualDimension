#include "stdafx.h"
#include <map>
#include <memory.h>
#include <math.h>
#include "Scaling.h"

using namespace std;

static const int BPP = 3;

typedef unsigned char uchar;

void Scaling::GetDefaultBitmapInfo(BITMAPINFO& bi, int w, int h)
{
	static BITMAPINFOHEADER bih = {
		sizeof(BITMAPINFOHEADER), // DWORD      biSize;
        0, // LONG       biWidth;
        0, // LONG       biHeight;
        1, // WORD       biPlanes;
        BPP * 8, // WORD       biBitCount;
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
	bi.bmiHeader.biSizeImage = w*h*BPP;
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
	struct Weights
	{
		Weights(int _depth, int* _shifts, float* _weights)
			: depth(_depth)
			, shifts(_shifts)
			, weights(_weights)
		{}

		~Weights() {
			delete [] shifts;
			delete [] weights;
		}
		
		void Put(uchar* outPix, const uchar* inLine, int n, int stride) const {
			float* w = weights + n * depth;
			int shift = shifts[n];
			for (int i=0; i<depth; ++i) {
				float x = w[i];
				if (x < 0.005)
					continue;
				*outPix += uchar(float(*(inLine + (shift + i)*stride)) * x);
				*(outPix + 1) += uchar(float(*(inLine + (shift + i)*stride + 1)) * x);
				*(outPix + 2) += uchar(float(*(inLine + (shift + i)*stride + 2)) * x);
			}
		}
	
		int depth;
		int* shifts;
		float* weights; //[outW][depth]
	};

	class WeightsCache
	{
	public:
		WeightsCache() {
			InitializeCriticalSection(&m_cs);
		}

		~WeightsCache() {
			DeleteCriticalSection(&m_cs);			
			for (TCache::iterator it = m_cache.begin(), end = m_cache.end();
				it != end; ++it) {
					delete it->second;
			}
		}

		const Weights* GetWeights(int inW, int outW) {
			Weights* pw = 0;
			pair<int, int> key = make_pair(inW, outW);
			EnterCriticalSection(&m_cs);
			TCache::iterator it = m_cache.find(key);
			if (it == m_cache.end()) {
				pw = CalculateWeights(inW, outW);
				m_cache.insert(make_pair(key, pw));
			}
			else {
				pw = it->second;
			}
			LeaveCriticalSection(&m_cs);
			return pw;
		}

	private:
		static float WFunc(float center, int x, int radius) {
			float pixcenter = float(x) + 0.5f;
			return (float)exp(-1.44 * (fabs(pixcenter - center)));
		}

		static int WtoI(float weight)
		{
			if (weight < 0.005f)
				return 0;
			return (int)(weight * 255.0f);
		}

		static Weights* CalculateWeights(int inW, int outW) {
			float factor = float(inW) / outW;
			int depth = int(ceil(factor)) + 2;
			int radius = depth / 2;
			int* shifts = new int[outW];
			float* weights = new float[outW * depth];
			//float* wf = new float[depth];
			
			for (int i=0; i<outW; ++i) {
				float center = factor * (0.5f + float(i));
				int shift = (int)floor(center) - radius;
				shifts[i] = shift;

				float* w = weights + i*depth;
				float sum = 0.0f;
				for (int j=0; j<depth; ++j) {
					int x = j + shift;
					if (x < 0 || x >= inW) {
						w[j] = 0.0;
					}
					else {
						w[j] = WFunc(center, x, radius);
						sum += w[j];
					}
				}
				for (int j=0; j<depth; ++j) {
					//w[j] = WtoI(wf[j]/sum);
					w[j] /= sum;
				}
			}

			//delete [] wf;

			return new Weights(depth, shifts, weights);			
		}

		typedef map<pair<int, int>, Weights*> TCache;
		TCache m_cache;
		CRITICAL_SECTION m_cs;
	};


	static WeightsCache s_wcache;

	void ScaleHorizontally(const uchar* buf, int w, int h, uchar* outBuf, int outW)
	{
		const Weights* weights = s_wcache.GetWeights(w, outW);
		for (int i=0; i<h; ++i) {
			int offset = i*outW*BPP;
			const uchar* inLine = buf + i*w*BPP;
			for (int j=0; j<outW; ++j) {
				weights->Put(outBuf + offset + j*BPP, inLine, j, BPP);
			}
		}
		//for (int i=0; i<h; ++i)
		//{
		//	memmove(outBuf + i*outW*BPP, buf + i*w*BPP, outW*BPP);
		//}
	}

	void ScaleVertically(const uchar* buf, int w, int h, uchar* outBuf, int outH)
	{
		const Weights* weights = s_wcache.GetWeights(h, outH);
		int stride = w*BPP;
		for (int i=0; i<outH; ++i) {
			for (int j=0; j<w; ++j) {
				const uchar* inLine = buf + j*BPP;
				uchar* outPix = outBuf + (i*w + j)*BPP;
				weights->Put(outPix, inLine, i, stride);
			}
		}
		//memmove(outBuf, buf, w*outH*BPP);
	}
}

void Scaling::Scale(const void* buf, int w, int h, void* outBuf, int outW, int outH)
{
	size_t sz = outW * h * BPP;
	uchar* tempBuf = new uchar[sz];
	memset(tempBuf, 0, sz);
	ScaleHorizontally((const uchar*)buf, w, h, tempBuf, outW);
	ScaleVertically(tempBuf, outW, h, (uchar*)outBuf, outH);
	delete [] tempBuf;
}
