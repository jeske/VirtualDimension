#ifndef __FULLSCREENVIEW_H__
#define __FULLSCREENVIEW_H__
#include <vector>
#include "FastWindow.h"
#include "FullScreenViewThumbnail.h"

class DesktopManager;

class FullScreenView : public FastWindow
{
public:
	FullScreenView();
	virtual ~FullScreenView();

	bool Start(HWND parent, DesktopManager* dm);

private:
	void CalculateThumbnailsLayout(int W, int H, int n, std::vector<POINT>& coords, int& w, int& h);

	LRESULT OnLeftButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnThumbnailClicked(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	std::vector<FullScreenViewThumbnail> m_thumbnails;
	DesktopManager* m_dm;

	static bool s_initialized;
	static void RegisterClass();
};

#endif //__FULLSCREENVIEW_H__
