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

CPageRefreshRateDlg::CPageRefreshRateDlg(HINSTANCE hInstance, SendCommand_t fn, HOST_DATA& status)
	: CPageDlg(hInstance, fn, status)
	, m_tb(hInstance, 0x10, 0, 0, 0, 0, TBSTYLE_FLAT | CCS_NODIVIDER | TBSTYLE_TOOLTIPS | WS_TABSTOP | WS_VISIBLE, WS_EX_STATICEDGE)
	, m_lvListRates(hInstance, 0x11, 7, 90, 100, 100, WS_TABSTOP | WS_VISIBLE | LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SINGLESEL, WS_EX_STATICEDGE)
	, m_st(hInstance, TEXT("Press Fn+S (or Fn+2) to change mode"), 7, 75, 150, 10)
{
	pushctl(m_tb);
	pushctl(m_st);
	pushctl(m_lvListRates);
	AutosizeCtl(m_tb);

	try
	{
		ResolveDisplayDevName();
	}
	catch (COMException&)
	{}
}

void CPageRefreshRateDlg::ResolveDisplayDevName()
{
	WMIQuery q(TEXT("root\\WMI"));
	std::vector<WbemClassItem> rows;
	if (!q.Query(TEXT("SELECT * FROM WmiMonitorConnectionParams"), rows))
		return;

	for (auto& row : rows)
	{
		auto it = row.find(TEXT("InstanceName"));
		if (it == row.end())
			continue;

		auto& dev = it->second;
		if (VT_BSTR != dev->vt)
			continue;

		m_strDisplay = dev->bstrVal;
		break;
	}
}

BOOL CPageRefreshRateDlg::OnPageInit(LPARAM lParam)
{
	CBitmap bmTb;
	DeleteBitmap(m_tb.SetImageList(MergeBitmap(bmTb, 
		{ 
			IDB_RATE_FAIL,
			IDB_RATE_48,
			IDB_RATE_60,
			IDB_RATE_72,
			IDB_RATE_75,
			IDB_RATE_90,
			IDB_RATE_100,
			IDB_RATE_120,
			IDB_RATE_144,
			IDB_RATE_165,
			IDB_RATE_240
		}).Bitmap()));

	m_lvListRates.ExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_DOUBLEBUFFER);
	m_lvListRates.AddColumn(TEXT(""));

	return true;
}

void CPageRefreshRateDlg::OnButton(USHORT uId)
{
	DEVMODE devMode({ 0 });
	devMode.dmSize = sizeof(devMode);
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);

	switch (uId)
	{
	case IDB_RATE_48: devMode.dmDisplayFrequency = 48; break;
	case IDB_RATE_60: devMode.dmDisplayFrequency = 60; break;
	case IDB_RATE_72: devMode.dmDisplayFrequency = 72; break;
	case IDB_RATE_75: devMode.dmDisplayFrequency = 75; break;
	case IDB_RATE_90: devMode.dmDisplayFrequency = 90; break;
	case IDB_RATE_100: devMode.dmDisplayFrequency = 100; break;
	case IDB_RATE_120: devMode.dmDisplayFrequency = 120; break;
	case IDB_RATE_144: devMode.dmDisplayFrequency = 144; break;
	case IDB_RATE_165: devMode.dmDisplayFrequency = 165; break;
	case IDB_RATE_240: devMode.dmDisplayFrequency = 240; break;
	}

	if (ChangeDisplaySettings(&devMode, CDS_UPDATEREGISTRY) == DISP_CHANGE_SUCCESSFUL)
		SendMessage(*Parent(), WM_CUSTOM_SHOW_SPLASH, uId, 0);
	else
		SendMessage(*Parent(), WM_CUSTOM_SHOW_SPLASH, IDB_RATE_FAIL, 0);
}

void CPageRefreshRateDlg::OnShow()
{
	UpdateToolbar();
}

void CPageRefreshRateDlg::ListDeniedModes(std::vector<uint8_t>& deniedModes)
{
	CRegKey keyMI;
	deniedModes.clear();
	if (ERROR_SUCCESS != keyMI.Create(HKEY_CURRENT_USER, TEXT("Software\\MIControl"), nullptr, 0, KEY_QUERY_VALUE))
		return;

	ULONG uSize = 100;
	std::vector<uint8_t> buffer(uSize, 0);
	if (ERROR_SUCCESS != keyMI.QueryBinaryValue(TEXT("DeniedRates"), buffer.data(), &uSize))
		return;

	deniedModes.insert(deniedModes.end(), buffer.begin(), buffer.begin() + min(uSize, buffer.size()));

	std::sort(deniedModes.begin(), deniedModes.end());
	auto deniedModes_filtered = std::unique(deniedModes.begin(), deniedModes.end());
	deniedModes.erase(deniedModes_filtered, deniedModes.end());
}

void CPageRefreshRateDlg::ModifyDeniedModes(uint8_t mode, ModifyDeniedModeOp op)
{
	std::vector<uint8_t> vDeniedModes;
	ListDeniedModes(vDeniedModes);

	if (ModifyDeniedModeOp::Add == op)
	{
		auto it = std::find(vDeniedModes.begin(), vDeniedModes.end(), mode);
		if (it != vDeniedModes.end())
			return;

		vDeniedModes.push_back(mode);
		std::sort(vDeniedModes.begin(), vDeniedModes.end());
	}
	else if (ModifyDeniedModeOp::Remove == op)
	{
		auto it = std::find(vDeniedModes.begin(), vDeniedModes.end(), mode);
		if (it == vDeniedModes.end())
			return;

		vDeniedModes.erase(it);
	}

	CRegKey keyMI;
	if (ERROR_SUCCESS != keyMI.Create(HKEY_CURRENT_USER, TEXT("Software\\MIControl"), nullptr, 0, KEY_SET_VALUE))
		return;

	if (vDeniedModes.empty())
		keyMI.DeleteValue(TEXT("DeniedRates"));
	else
		keyMI.SetBinaryValue(TEXT("DeniedRates"), vDeniedModes.data(), static_cast<ULONG>(vDeniedModes.size()));
}

void CPageRefreshRateDlg::UpdateToolbar(bool fUpdateList)
{
	m_fListUpdatePending = fUpdateList;
	auto count = m_tb.cSendMessage(TB_BUTTONCOUNT);
	for (decltype(count)i = 0; i < count; ++i)
		m_tb.cSendMessage(TB_DELETEBUTTON, 0);

	int modeNum = 0;
	DEVMODE devMode;
	ZeroMemory(&devMode, sizeof(DEVMODE));
	devMode.dmSize = sizeof(DEVMODE);
	std::vector<DWORD> rates;

	while (EnumDisplaySettings(nullptr, modeNum++, &devMode))
	{
		if (std::find(rates.begin(), rates.end(), devMode.dmDisplayFrequency) != rates.end())
			continue;

		rates.emplace_back(devMode.dmDisplayFrequency);
	}

	std::sort(rates.begin(), rates.end());

	ZeroMemory(&devMode, sizeof(DEVMODE));
	devMode.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);
	DWORD k = 0;

	if (fUpdateList)
		m_lvListRates.ResetContent();

	std::vector<uint8_t> vDeniedModes;
	ListDeniedModes(vDeniedModes);

	for (const auto& rate : rates)
	{
		auto it = std::find(vDeniedModes.begin(), vDeniedModes.end(), rate);
		const bool fIsModeDenied = (vDeniedModes.end() != it);
		if (fUpdateList)
		{
			CString strRate;
			strRate.Format(TEXT("%d Hz"), rate);
			int id = static_cast<int>(m_lvListRates.AddItem(strRate, rate));
			m_lvListRates.Check(id, !fIsModeDenied);
		}

		if (fIsModeDenied)
			continue;

		switch (rate)
		{
		case 48: m_tb.AddRadio(1, IDB_RATE_48); k = ((devMode.dmDisplayFrequency == rate) ? IDB_RATE_48 : k); break;
		case 60: m_tb.AddRadio(2, IDB_RATE_60); k = ((devMode.dmDisplayFrequency == rate) ? IDB_RATE_60 : k); break;
		case 72: m_tb.AddRadio(3, IDB_RATE_72); k = ((devMode.dmDisplayFrequency == rate) ? IDB_RATE_72 : k); break;
		case 75: m_tb.AddRadio(4, IDB_RATE_75); k = ((devMode.dmDisplayFrequency == rate) ? IDB_RATE_75 : k); break;
		case 90: m_tb.AddRadio(5, IDB_RATE_90); k = ((devMode.dmDisplayFrequency == rate) ? IDB_RATE_90 : k); break;
		case 100: m_tb.AddRadio(6, IDB_RATE_100); k = ((devMode.dmDisplayFrequency == rate) ? IDB_RATE_100 : k); break;
		case 120: m_tb.AddRadio(7, IDB_RATE_120); k = ((devMode.dmDisplayFrequency == rate) ? IDB_RATE_120 : k); break;
		case 144: m_tb.AddRadio(8, IDB_RATE_144); k = ((devMode.dmDisplayFrequency == rate) ? IDB_RATE_144 : k); break;
		case 165: m_tb.AddRadio(9, IDB_RATE_165); k = ((devMode.dmDisplayFrequency == rate) ? IDB_RATE_165 : k); break;
		case 240: m_tb.AddRadio(10, IDB_RATE_240); k = ((devMode.dmDisplayFrequency == rate) ? IDB_RATE_240 : k); break;
		}

	}

	if (k > 0)
		m_tb.cSendMessage(TB_CHECKBUTTON, k, true);

	m_fListUpdatePending = false;
}

INT_PTR CPageRefreshRateDlg::OnNotify(LPNMHDR lpnm)
{
	switch (lpnm->idFrom)
	{
	case 0x11:
	{
		switch (lpnm->code)
		{
		case LVN_ITEMCHANGED: return LvControl((NMLISTVIEW*)lpnm); break;
		}
	}
	}

	return 0;
}

INT_PTR CPageRefreshRateDlg::LvControl(NMLISTVIEW* pnmlv)
{
	if (0x2000 == (pnmlv->uNewState & 0x2000) && !m_fListUpdatePending)
	{
		ModifyDeniedModes(LOBYTE(pnmlv->lParam), ModifyDeniedModeOp::Remove);
		UpdateToolbar(false);
	}
	else if (0x1000 == (pnmlv->uNewState & 0x1000) && !m_fListUpdatePending)
	{
		ModifyDeniedModes(LOBYTE(pnmlv->lParam), ModifyDeniedModeOp::Add);
		UpdateToolbar(false);
	}

	return 0;
}
