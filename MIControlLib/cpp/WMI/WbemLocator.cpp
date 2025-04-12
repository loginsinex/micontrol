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
#pragma comment(lib, "wbemuuid.lib")

WbemLocator::WbemLocator()
{
    auto hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&m_pLoc);

    if (FAILED(hres))
        throw COMException(hres);
}

WbemLocator::~WbemLocator()
{
    m_pLoc->Release();
}

IWbemLocator* WbemLocator::operator->()
{
    return m_pLoc;
}

WbemLocator::operator IWbemLocator* ()
{
    return operator->();
}