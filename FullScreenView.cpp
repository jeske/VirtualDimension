#include "stdafx.h"
#include "DesktopManager.h"
#include "FullScreenView.h"

static LPCTSTR s_className = "VirtualDimensionFSV";

static const int s_padding = 16;
static const int s_spacing = 16;
static const int s_border = 8;

bool FullScreenView::s_initialized = false;

FullScreenView::FullScreenView()
	: m_dm(0)
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
{}

void FullScreenView::RegisterClass()
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= GetModuleHandle(0);
	wcex.hIcon			= 0;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= s_className;
	wcex.hIconSm      = NULL;

	FastWindow::RegisterClassEx(&wcex);
}

bool FullScreenView::Start(HWND parent, DesktopManager* dm)
{
	int n = dm->GetNbDesktops();
	if (n <= 1)
		return false;
	
	m_dm = dm;

	RECT workarea;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workarea, 0);
	
	int W = workarea.right - workarea.left;
	int H = workarea.bottom - workarea.top;
	
	std::vector<POINT> coords;
	int w, h;
	CalculateThumbnailsLayout(W, H, n, coords, w, h);

	HWND hwnd = FastWindow::Create(
		WS_EX_TOPMOST, s_className, "", WS_POPUP | WS_MAXIMIZE, 
		workarea.left, workarea.top, W, H, 
		parent, 0, GetModuleHandle(0)
	);
	if (!hwnd)
		return false;

	for (int i=0; i<n; ++i) {
		FullScreenViewThumbnail th;
		if (!th.Create(hwnd, s_border, coords[i].x, coords[i].y, w, h, i, dm->GetDesktop(i)->GetPicture())) {
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return false;
		}
	}

	ShowWindow(hwnd, SW_MAXIMIZE);
	UpdateWindow(hwnd);
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
	}
}

LRESULT FullScreenView::OnLeftButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PostMessage(*this, WM_CLOSE, 0, 0);
	return 0;
}

LRESULT FullScreenView::OnThumbnailClicked(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int index = (int)wParam;
	SendMessage(*this, WM_CLOSE, 0, 0); //???? 
	m_dm->SwitchToDesktop(m_dm->GetDesktop(index));
	return 0;
}

LRESULT FullScreenView::OnPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	RECT rect;
	HDC hdc;

	GetClientRect(hWnd, &rect);
	hdc = BeginPaint(hWnd, &ps);

	HBRUSH br = CreateSolidBrush(0x00202040);
	FillRect(hdc, &rect, br);
	
	EndPaint(hWnd, &ps);
	return 0;
}

