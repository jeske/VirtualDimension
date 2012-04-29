#include "stdafx.h"
#include "DesktopManager.h"
#include "Resource.h"
#include "FullScreenView.h"

FullScreenView* fullScreenView;

static LPCTSTR s_className = "VirtualDimensionFSV";

static const int s_padding = 16;
static const int s_spacing = 16;
static const int s_border = 8;

bool FullScreenView::s_initialized = false;

FullScreenView::FullScreenView(DesktopManager* dm)
	: m_dm(dm)
	, m_parent(0)
{
	if (!s_initialized) {
		RegisterClass();
		s_initialized = true;
	}
	SetMessageHandler(WM_PAINT, this, &FullScreenView::OnPaint);
	SetMessageHandler(WM_LBUTTONDOWN, this, &FullScreenView::OnLeftButtonDown);
	SetMessageHandler(MSG_FSV_THUMBNAIL, this, &FullScreenView::OnThumbnailClicked);
}

FullScreenView::~FullScreenView()
{
	DestroyThumbnails();
}

void FullScreenView::RegisterClass()
{
	HINSTANCE hinst = GetModuleHandle(0);
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hinst;
	wcex.hIcon			= LoadIcon(hinst, (LPCTSTR)IDI_VIRTUALDIMENSION);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= s_className;
	wcex.hIconSm      = NULL;

	FastWindow::RegisterClassEx(&wcex);
}

bool FullScreenView::Create(HWND parent)
{
	m_parent = parent;
	
	//RECT workarea;
	//SystemParametersInfo(SPI_GETWORKAREA, 0, &workarea, 0);
	
	int W = GetSystemMetrics(SM_CXSCREEN); //workarea.right - workarea.left;
	int H = GetSystemMetrics(SM_CYSCREEN); //workarea.bottom - workarea.top;

	HINSTANCE hinst = GetModuleHandle(0);

	TCHAR title[80];
	LoadString(hinst, IDS_APP_TITLE, title, 80);

	HWND hwnd = FastWindow::Create(
		WS_EX_TOPMOST, s_className, title, WS_POPUP | WS_MAXIMIZE, 
		//workarea.left, workarea.top, 
		0, 0,
		W, H, 
		0, 0, hinst
	);
	if (!hwnd)
		return false;
		
	ShowWindow(hwnd, SW_MINIMIZE);

	SetMessageHandler(WM_SIZE, this, &FullScreenView::OnSize);

	return true;
}

bool FullScreenView::CreateThumbnails()
{
	m_dm->GetCurrentDesktop()->RefreshPicture();
	DestroyThumbnails();

	int n = m_dm->GetNbDesktops();
	if (n <= 1)
		return false;
	
	//RECT workarea;
	//SystemParametersInfo(SPI_GETWORKAREA, 0, &workarea, 0);
	
	int W = GetSystemMetrics(SM_CXSCREEN); //workarea.right - workarea.left;
	int H = GetSystemMetrics(SM_CYSCREEN); //workarea.bottom - workarea.top;
	
	std::vector<POINT> coords;
	int w, h;
	CalculateThumbnailsLayout(W, H, n, coords, w, h);
		

	Desktop* d = m_dm->GetFirstDesktop();
	for (int i=0; i<n, d; ++i) {
		FullScreenViewThumbnail* th = new FullScreenViewThumbnail;
		m_thumbnails.push_back(th);
		if (!th->Create(m_hWnd, s_border, coords[i].x, coords[i].y, w, h, d)) {
			DestroyThumbnails();
			return false;
		}
		d = m_dm->GetNextDesktop();
	}
	return true;
}

void FullScreenView::DestroyThumbnails()
{
	for (size_t i=0; i<m_thumbnails.size(); ++i)
		delete m_thumbnails[i];
	m_thumbnails.clear();
}

namespace
{
	void GetRowsCols(int n, int& nrows, int& ncols)
	{
		nrows = 1;
		ncols = 1;
		for (;;) {
			if (n <= nrows * ncols)
				break;
			if (nrows == ncols)
				++ncols;
			else
				++nrows;
		}
	}
}

void FullScreenView::CalculateThumbnailsLayout(
	int W, int H, int n, std::vector<POINT>& coords, int& w, int& h
)
{
	int nrows, ncols;
	GetRowsCols(n, nrows, ncols);

	int hpadding = s_padding;
	
	w = (W - 2*hpadding - ncols*2*s_border - s_spacing*(ncols-1))/ncols;
	w &= ~7; //round to 8 px
	hpadding = (W - ncols*(2*s_border + w) - s_spacing*(ncols-1))/2;
	h = w*H/W;
	int vpadding = (H - nrows*(2*s_border + h) - s_spacing*(nrows-1))/2;
	int cols_last_row = ncols - (nrows*ncols - n)%ncols;
	int hpadding_last_row = (W - cols_last_row*(2*s_border + w) - s_spacing*(cols_last_row-1))/2;

	POINT p;
	int i=0;
	for (; i<nrows-1; ++i) {
		p.y = vpadding + i*(2*s_border + h + s_spacing);
		for (int j=0; j<ncols; ++j) {
			p.x = hpadding + j*(2*s_border + w + s_spacing);
			coords.push_back(p);
		}
	}
	p.y = vpadding + i*(2*s_border + h + s_spacing);
	for (int j=0; j<cols_last_row; ++j) {
		p.x = hpadding_last_row + j*(2*s_border + w + s_spacing);
		coords.push_back(p);
	}
}

LRESULT FullScreenView::OnLeftButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ShowWindow(*this, SW_MINIMIZE);
	return 0;
}

LRESULT FullScreenView::OnThumbnailClicked(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Desktop* d = (Desktop*)lParam;
	ShowWindow(m_hWnd, SW_MINIMIZE);
	m_dm->SwitchToDesktop(d);
	return 0;
}

LRESULT FullScreenView::OnPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (IsIconic(m_hWnd)) {
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	
	PAINTSTRUCT ps;
	RECT rect;
	HDC hdc;

	GetClientRect(hWnd, &rect);
	
	hdc = BeginPaint(hWnd, &ps);

	HBRUSH br = CreateSolidBrush(0x00402020);
	FillRect(hdc, &rect, br);
	
	EndPaint(hWnd, &ps);
	
	DeleteObject(br);
	return 0;
}

LRESULT FullScreenView::OnSize(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (wParam == SIZE_MINIMIZED) {
		ShowWindow(m_parent, SW_SHOW);
	}
	if (wParam == SIZE_MAXIMIZED) {
		CreateThumbnails();
		ShowWindow(m_parent, SW_HIDE);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


