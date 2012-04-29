#include "stdafx.h"
#include "DesktopRenameWnd.h"
#include "Resource.h"
#include "Locale.h"

namespace
{
	BOOL CALLBACK DialogFunc(HWND hwnd, UINT message, WPARAM wP, LPARAM lP)
	{
		TCHAR* name = 0;
		int id = -1;

		switch(message)
		{
		case WM_INITDIALOG:
			name = (TCHAR*)lP;
			assert(name);
			SetWindowLongPtr(hwnd, GWL_USERDATA, lP);
			SetWindowText(GetDlgItem(hwnd, IDC_EDIT_DESKTOPRENAME), name);
			break;
		case WM_COMMAND:
			id = LOWORD(wP);
			if (id == IDOK) {
				name = (TCHAR*)GetWindowLongPtr(hwnd, GWL_USERDATA);
				assert(name);
				GetDlgItemText(hwnd, IDC_EDIT_DESKTOPRENAME, name, 80);
				EndDialog(hwnd, id);
			}
			else if (id == IDCANCEL) {
				EndDialog(hwnd, id);
			}
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
}

bool DesktopRenameWnd::DoModal(HWND parent, TCHAR* name)
{
	INT_PTR ret = DialogBoxParam(Locale::GetInstance(), MAKEINTRESOURCE(IDD_DESKTOPRENAME), parent, DialogFunc, (LPARAM)name);
	//if (ret == -1)
	//{
	//	DWORD error = GetLastError();
	//}
	return (IDOK == ret);
}
