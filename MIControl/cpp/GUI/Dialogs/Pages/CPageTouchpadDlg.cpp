/*
	MI Control Utility — v1.2.5 [public version]
	Copyright (C) 2025 ALXR aka loginsin
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "precomp.h"
#include <Common.h>

CPageTouchpadDlg::CPageTouchpadDlg(HINSTANCE hInstance, SendCommand_t fn, HOST_DATA& status)
    : CPageDlg(hInstance, fn, status)
    , m_tb(hInstance, 0x10, 0, 0, 0, 0, TBSTYLE_FLAT | CCS_NODIVIDER | TBSTYLE_TOOLTIPS | WS_TABSTOP | WS_VISIBLE, WS_EX_STATICEDGE)
    , m_st(hInstance, TEXT("Press Fn+J to change mode"), 7, 75, 100, 10)
{
    pushctl(m_tb);
    pushctl(m_st);
    AutosizeCtl(m_tb);
}

BOOL CPageTouchpadDlg::OnPageInit(LPARAM lParam)
{
    CBitmap bmTb;
    DeleteBitmap(m_tb.SetImageList(MergeBitmap(bmTb, { IDB_TOUCHPAD_OFF, IDB_TOUCHPAD_ON }).Bitmap()));

    m_tb.AddRadio(0, IDB_TOUCHPAD_OFF);
    m_tb.AddRadio(1, IDB_TOUCHPAD_ON);

    return true;
}

void CPageTouchpadDlg::OnShow()
{
    auto val = IsTouchpadEnabled();

    if (val)
        m_tb.cSendMessage(TB_CHECKBUTTON, IDB_TOUCHPAD_ON, true);
    else
        m_tb.cSendMessage(TB_CHECKBUTTON, IDB_TOUCHPAD_OFF, true);
}

void CPageTouchpadDlg::OnButton(USHORT uId)
{
    switch (uId)
    {
        case IDB_TOUCHPAD_ON:
        {
            EnableTouchPad(true);
            SendMessage(*Parent(), WM_CUSTOM_SHOW_SPLASH, IDB_TOUCHPAD_ON, 0);
        }
        break;
        case IDB_TOUCHPAD_OFF: 
        {
            EnableTouchPad(false);
            SendMessage(*Parent(), WM_CUSTOM_SHOW_SPLASH, IDB_TOUCHPAD_OFF, 0);
        }
    }

}

bool CPageTouchpadDlg::GetLineColor(COLORREF& clr, UINT& iconId) const
{
    iconId = 0;
    if (m_status && !IsTouchpadEnabled())
        iconId = 2;

    return true;
}

bool CPageTouchpadDlg::IsTouchpadEnabled()
{
    CRegKey keyTouchpad;
    if (ERROR_SUCCESS != keyTouchpad.Open(HKEY_CURRENT_USER, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\PrecisionTouchPad\\Status"), KEY_QUERY_VALUE))
        return false;

    DWORD val = 0;
    if (ERROR_SUCCESS != keyTouchpad.QueryDWORDValue(TEXT("Enabled"), val))
        return false;

    return val;
}