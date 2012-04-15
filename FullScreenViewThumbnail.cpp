#include "stdafx.h"
#include "FullScreenViewThumbnail.h"
#include "Scaling.h"
#include "Desktop.h"

static LPCTSTR s_className = "VirtualDimensionFSVTH";
static const DWORD s_inactiveBC = 0x00603030;
static const DWORD s_activeBC = 0x00C06060;

bool FullScreenViewThumbnail::s_initialized = false;

FullScreenViewThumbnail::FullScreenViewThumbnail()
	: m_desktop(0)
	, m_border(0)
	, m_picW(0)
	, m_picH(0)
	, m_picture(0)
	, m_hover(false)
{
	if (!s_initialized) {
		RegisterClass();
		s_initialized = true;
	}

	Scaling::GetDefaultBitmapInfo(m_bi, 0, 0);

	SetMessageHandler(WM_MOUSEMOVE, this, &FullScreenViewThumbnail::OnMouseMove);
	SetMessageHandler(WM_MOUSELEAVE, this, &FullScreenViewThumbnail::OnMouseLeave);
	SetMessageHandler(WM_PAINT, this, &FullScreenViewThumbnail::OnPaint);
	SetMessageHandler(WM_LBUTTONDOWN, this, &FullScreenViewThumbnail::OnLeftButtonDown);
}

FullScreenViewThumbnail::~FullScreenViewThumbnail()
{
	if (m_picture)
		delete [] m_picture;
}

void FullScreenViewThumbnail::RegisterClass()
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

bool FullScreenViewThumbnail::Create(HWND parent, int border, int x, int y, int w, int h, Desktop* desk)
{
	if (!desk)
		return false;
	m_desktop = desk;
	m_border = border;
	m_picW = w;
	m_picH = h;

	m_picture = new char[w*h*3];
	memset(m_picture, 0, w*h*3);
	int W = GetSystemMetrics(SM_CXSCREEN);
	int H = GetSystemMetrics(SM_CYSCREEN);
	Scaling::Scale(m_desktop->GetPicture(), W, H, m_picture, w, h);

	m_bi.bmiHeader.biWidth = w;
    m_bi.bmiHeader.biHeight = h;
	m_bi.bmiHeader.biSizeImage = w*h*3;

	HWND hwnd = FastWindow::Create(s_className, "", WS_CHILD | WS_VISIBLE, 
		x, y, w + 2*border, h + 2*border, parent, 0, GetModuleHandle(0));

	if (!hwnd)
		return false;

	//TRACKMOUSEEVENT tme;
	//tme.cbSize = sizeof(tme);
	//tme.dwFlags = TME_HOVER;
	//tme.hwndTrack = hwnd;
	//tme.dwHoverTime = HOVER_DEFAULT;
	//TrackMouseEvent(&tme);

	return true;
}

LRESULT FullScreenViewThumbnail::OnMouseMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!m_hover) {
		m_hover = true;
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = hWnd;
		tme.dwHoverTime = HOVER_DEFAULT;
		tme.dwFlags = TME_LEAVE;
		TrackMouseEvent(&tme);
		RECT rc;
		GetClientRect(hWnd, &rc);
		InvalidateRect(hWnd, &rc, FALSE);
		//UpdateWindow(*this);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT FullScreenViewThumbnail::OnMouseLeave(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	m_hover = false;
	RECT rc;
	GetClientRect(hWnd, &rc);
	InvalidateRect(hWnd, &rc, FALSE);
	//UpdateWindow(*this);
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT FullScreenViewThumbnail::OnPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	RECT rect;
	HDC hdc;

	GetClientRect(hWnd, &rect);
	hdc = BeginPaint(hWnd, &ps);

	HPEN pen = CreatePen(PS_NULL,0,0);
	HBRUSH br = CreateSolidBrush(m_hover ? s_activeBC : s_inactiveBC);
	SelectObject(hdc, pen);
	SelectObject(hdc, br);
	RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 8, 8);
	
	SetStretchBltMode(hdc, COLORONCOLOR);
	StretchDIBits(
		hdc,
		rect.left + m_border, rect.top + m_border, 
		m_picW, m_picH,
		0, 0, m_picW, m_picH, m_picture, &m_bi, DIB_RGB_COLORS, SRCCOPY
	);

	EndPaint(hWnd, &ps);
	DeleteObject(br);
	DeleteObject(pen);
	return 0;
}

LRESULT FullScreenViewThumbnail::OnLeftButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PostMessage(GetParent(hWnd), MSG_FSV_THUMBNAIL, 0, (LPARAM)m_desktop);
	return 0;
}
