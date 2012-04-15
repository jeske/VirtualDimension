#ifndef __FULLSCREENVIEW_H__
#define __FULLSCREENVIEW_H__
#include <vector>
#include "FastWindow.h"
#include "FullScreenViewThumbnail.h"

class DesktopManager;

class FullScreenView : public FastWindow
{
public:
	explicit FullScreenView(DesktopManager* dm);
	virtual ~FullScreenView();

	bool Create(HWND parent);

private:
	bool CreateThumbnails();
	void DestroyThumbnails();
	void CalculateThumbnailsLayout(int W, int H, int n, std::vector<POINT>& coords, int& w, int& h);

	LRESULT OnLeftButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnThumbnailClicked(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnSize(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		
	std::vector<FullScreenViewThumbnail*> m_thumbnails;
	DesktopManager* m_dm;
	HWND m_parent;

	static bool s_initialized;
	static void RegisterClass();
};

extern FullScreenView* fullScreenView;

#endif //__FULLSCREENVIEW_H__
