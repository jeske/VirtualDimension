#ifndef __THUMBNAIL_H__
#define __THUMBNAIL_H__
#include "FastWindow.h"

class Thumbnail : public FastWindow
{
	LRESULT OnLeftButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif //__THUMBNAIL_H__
