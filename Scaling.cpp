#include "stdafx.h"
#include <map>
#include <memory.h>
#include <math.h>
#include "Scaling.h"

using namespace std;

static const int BPP = 3;

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
		Weights(int _depth, int* _shifts, int* _weights)
			: depth(_depth)
			, shifts(_shifts)
			, weights(_weights)
		{}

		~Weights() {
			delete [] shifts;
			delete [] weights;
		}
		
		void Put(char* outPix, const char* inLine, int n, int stride) const {
			int* w = weights + n * depth;
			int shift = shifts[n];
			for (int i=0; i<depth; ++i) {
				int x = w[i];
				if (x <=0)
					continue;
				*outPix += (*(inLine + (shift + i)*stride)) * x;
				*(outPix + 1) += (*(inLine + (shift + i)*stride) + 1) * x;
				*(outPix + 2) += (*(inLine + (shift + i)*stride) + 2) * x;
			}
		}
	
		int depth;
		int* shifts;
		int* weights; //[outW][depth]
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
			return (float)exp(-1.44 * radius * (fabs(pixcenter - center)));
		}

		static int WtoI(float weight)
		{
			if (weight < 0.005f)
				return 0;
			return (int)(weight * 255.0f + 0.5f);
		}

		static Weights* CalculateWeights(int inW, int outW) {
			float factor = float(inW) / outW;
			int depth = int(ceil(factor)) + 1;
			int radius = depth / 2;
			int* shifts = new int[outW];
			int* weights = new int[outW * depth];
			float* wf = new float[depth];
			
			for (int i=0; i<outW; ++i) {
				float center = factor * (0.5f + float(i));
				int shift = (int)floor(center) - radius;
				shifts[i] = shift;

				int* w = weights + i*depth;
				float sum = 0.0f;
				for (int j=0; j<depth; ++j) {
					int x = j + shift;
					if (x < 0 || x >= inW) {
						wf[j] = 0.0;
					}
					else {
						wf[j] = WFunc(center, x, radius);
						sum += wf[j];
					}
				}
				for (int j=0; j<depth; ++j) {
					w[j] = WtoI(wf[j]/sum);
				}
			}

			delete [] wf;

			return new Weights(depth, shifts, weights);			
		}

		typedef map<pair<int, int>, Weights*> TCache;
		TCache m_cache;
		CRITICAL_SECTION m_cs;
	};


	static WeightsCache s_wcache;

	void ScaleHorizontally(const char* buf, int w, int h, char* outBuf, int outW)
	{
		const Weights* weights = s_wcache.GetWeights(w, outW);
		for (int i=0; i<h; ++i) {
			int offset = i*outW*BPP;
			const char* inLine = buf + i*w*BPP;
			for (int j=0; j<outW; ++j) {
				weights->Put(outBuf + offset + j*BPP, inLine, j, BPP);
			}
		}
	}

	void ScaleVertically(const char* buf, int w, int h, char* outBuf, int outH)
	{
		const Weights* weights = s_wcache.GetWeights(h, outH);
		int stride = w*BPP;
		for (int i=0; i<outH; ++i) {
			for (int j=0; j<w; ++j) {
				const char* inLine = buf + j*BPP;
				char* outPix = outBuf + (i*w + j)*BPP;
				weights->Put(outPix, inLine, i, stride);
			}
		}
	}
}

void Scaling::Scale(const void* buf, int w, int h, void* outBuf, int outW, int outH)
{
	char* tempBuf = new char[outW * h * BPP];
	ScaleHorizontally((const char*)buf, w, h, tempBuf, outW);
	ScaleVertically(tempBuf, outW, h, (char*)outBuf, outH);
}
