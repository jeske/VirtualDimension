#ifndef __SCALING_H__
#define __SCALING_H__

namespace Scaling
{
	void GetDefaultBitmapInfo(BITMAPINFO& bi, int w, int h);
	int BestFit(int w, int h, int maxW, int maxH);
	void Scale(const void* buf, int w, int h, void* outBuf, int outW, int outH);
}

#endif //__SCALING_H__
