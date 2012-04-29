#ifndef __DESKTOPRENAMEWND_H__
#define __DESKTOPRENAMEWND_H__

#include "FastWindow.h"

class DesktopRenameWnd : public FastWindow
{
public:
	DesktopRenameWnd()
	{}
	virtual ~DesktopRenameWnd()
	{}
	bool DoModal(HWND parent, TCHAR* name);
};

#endif //__DESKTOPRENAMEWND_H__
