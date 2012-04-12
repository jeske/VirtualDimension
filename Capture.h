#ifndef __CAPTURE_H__
#define __CAPTURE_H__

namespace Capture
{
	// hwnd = 0 for the whole screen
	void* CaptureWindow(HWND hwnd, RECT& rect);
}

#endif //__CAPTURE_H__
