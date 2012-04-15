#ifndef __FULLSCREENVIEWTHUMBNAIL_H__
#define __FULLSCREENVIEWTHUMBNAIL_H__
#include "FastWindow.h"

const UINT MSG_FSV_THUMBNAIL = WM_USER+1;

class Desktop;

class FullScreenViewThumbnail : public FastWindow
{
public:
	FullScreenViewThumbnail();
	virtual ~FullScreenViewThumbnail();
	bool Create(HWND parent, int border, int x, int y, int w, int h, Desktop* desk);
private:

	LRESULT OnMouseMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseLeave(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnLeftButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	BITMAPINFO m_bi;
	Desktop* m_desktop;
	int m_border;
	int m_picW;
	int m_picH;
	char* m_picture;
	bool m_hover;
	
	static bool s_initialized;
	static void RegisterClass();
};

#endif //__FULLSCREENVIEWTHUMBNAIL_H__
