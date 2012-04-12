#include "stdafx.h"
#include <memory.h>
#include "Capture.h"
#include "Scaling.h"

void* Capture::CaptureWindow(HWND hwnd, RECT& rect)
{
	if (hwnd != 0)
		return 0; // TODO! PrintWindow or so....

	int W = GetSystemMetrics(SM_CXSCREEN);
	int H = GetSystemMetrics(SM_CYSCREEN);

	rect.left = rect.top = 0;
	rect.right = W;
	rect.bottom = H;

	BITMAPINFO bi;
	Scaling::GetDefaultBitmapInfo(bi, W, H);
	
	size_t sz = bi.bmiHeader.biSizeImage;
	char* ret = new char[sz];

	HWND desktopWnd = GetDesktopWindow();
	HDC desktopDC = GetDC(desktopWnd);
	HDC compDC = CreateCompatibleDC(desktopDC);
	void* buffer = 0;
	HBITMAP compBmp = CreateDIBSection(compDC, &bi, DIB_RGB_COLORS, &buffer, 0,0);
	
	if (compBmp) {
		SelectObject(compDC, compBmp);
		BitBlt(compDC, 0, 0, W, H, desktopDC, 0, 0, SRCCOPY);
		if (buffer) {
			memmove(ret, buffer, sz);
		}
		else {
			delete [] ret;
			ret = 0;
		}
	}

	if (compDC)
		DeleteDC(compDC);
	if (compBmp)
		DeleteObject(compBmp);
	if (desktopDC)
		ReleaseDC(desktopWnd, desktopDC);

	return ret;
}

