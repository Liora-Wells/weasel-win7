#include "stdafx.h"
#include "WeaselTrayIcon.h"

// nasty
#include <resource.h>

static UINT mode_icon[] = { IDI_ZH, IDI_ZH, IDI_EN, IDI_RELOAD };
static const WCHAR *mode_label[] = { NULL, /*L"中文"*/ NULL, /*L"西文"*/ NULL, L"維護中" };

WeaselTrayIcon::WeaselTrayIcon(weasel::UI &ui)
	: m_style(ui.style()), m_status(ui.status()), m_mode(INITIAL), m_hTipWnd(NULL)
{
}

WeaselTrayIcon::~WeaselTrayIcon()
{
	if (m_hTipWnd != NULL)
		DestroyWindow(m_hTipWnd);
}

void WeaselTrayIcon::CustomizeMenu(HMENU hMenu)
{
	_BuildColorSchemeMenu(hMenu);
}

void WeaselTrayIcon::_BuildColorSchemeMenu(HMENU hMenu)
{
	m_color_scheme_ids.clear();

	RimeApi* rime = rime_get_api();
	if (!rime) return;

	RimeConfig config;
	if (!rime->config_open("weasel", &config)) return;

	RimeConfigIterator iter;
	if (!rime->config_begin_map(&iter, &config, "preset_color_schemes"))
	{
		rime->config_close(&config);
		return;
	}

	int count = 0;
	while (rime->config_next(&iter))
	{
		if (count >= 20) break;

		std::string name_key(iter.path);
		name_key += "/name";
		const char* name = rime->config_get_cstring(&config, name_key.c_str());
		if (!name) name = iter.key;

		std::wstring wname;
		wname.assign(name, name + strlen(name));

		UINT menu_id = ID_WEASELTRAY_COLOR_SCHEME_BASE + count;
		AppendMenuW(hMenu, MF_STRING, menu_id, wname.c_str());

		m_color_scheme_ids.push_back(utf8towcs(iter.key));
		count++;
	}
	rime->config_end(&iter);
	rime->config_close(&config);

	if (count > 0)
	{
		AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
	}
}

BOOL WeaselTrayIcon::Create(HWND hTargetWnd)
{
	HMODULE hModule = GetModuleHandle(NULL);
	CIcon icon;
	icon.LoadIconW(IDI_ZH);
	BOOL bRet = CSystemTray::Create(hModule, NULL, WM_WEASEL_TRAY_NOTIFY, 
		WEASEL_IME_NAME, icon, IDR_MENU_POPUP);
	if (hTargetWnd)
	{
		SetTargetWnd(hTargetWnd);
	}
	if (!m_style.display_tray_icon)
	{
		RemoveIcon();
	}
	return bRet;
}

void WeaselTrayIcon::Refresh()
{
	if (!m_style.display_tray_icon && !m_status.disabled)
	{
		if (m_mode != INITIAL)
		{
			RemoveIcon();
			m_mode = INITIAL;
		}
		return;
	}
	WeaselTrayMode mode = m_status.disabled ? DISABLED : 
		m_status.ascii_mode ? ASCII : ZHUNG;
	if (mode != m_mode)
	{
		m_mode = mode;
		ShowIcon();
		SetIcon(mode_icon[mode]);
		if (mode_label[mode])
		{
			if (m_style.ascii_tip_follow_cursor)
			{
				_ShowBalloonNearCursor(mode_label[mode], WEASEL_IME_NAME);
			}
			else
			{
				ShowBalloon(mode_label[mode], WEASEL_IME_NAME);
			}
		}
	}
	else if (!Visible())
	{
		ShowIcon();
	}
}

void WeaselTrayIcon::_ShowBalloonNearCursor(LPCTSTR szText, LPCTSTR szTitle)
{
	if (m_hTipWnd == NULL)
	{
		m_hTipWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
			TOOLTIPS_CLASS, NULL,
			WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, NULL, NULL);
	}
	if (m_hTipWnd != NULL)
	{
		POINT pt;
		GetCursorPos(&pt);

		TOOLINFO ti;
		ZeroMemory(&ti, sizeof(ti));
		ti.cbSize = sizeof(ti);
		ti.uFlags = TTF_ABSOLUTE | TTF_TRACK;
		ti.hwnd = NULL;
		ti.lpszText = (LPTSTR)szText;

		SendMessage(m_hTipWnd, TTM_ADDTOOL, 0, (LPARAM)&ti);
		SendMessage(m_hTipWnd, TTM_SETTITLE, (WPARAM)1, (LPARAM)szTitle);
		SendMessage(m_hTipWnd, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(pt.x + 16, pt.y + 16));
		SendMessage(m_hTipWnd, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);

		SetTimer(m_hTipWnd, 1, 2000, NULL);
	}
}
