#pragma once
#include <WeaselUI.h>
#include <WeaselIPC.h>
#include "SystemTraySDK.h"

#define	WM_WEASEL_TRAY_NOTIFY (WEASEL_IPC_LAST_COMMAND + 100)


class WeaselTrayIcon : public CSystemTray
{
public:
	enum WeaselTrayMode {
		INITIAL, ZHUNG, ASCII, DISABLED,
	};

	WeaselTrayIcon(weasel::UI &ui);
	~WeaselTrayIcon();

	BOOL Create(HWND hTargetWnd);
	void Refresh();

protected:
	virtual void CustomizeMenu(HMENU hMenu);
	void _ShowBalloonNearCursor(LPCTSTR szText, LPCTSTR szTitle);
	void _BuildColorSchemeMenu(HMENU hMenu);

	weasel::UIStyle &m_style;
	weasel::Status &m_status;
	WeaselTrayMode m_mode;
	HWND m_hTipWnd;
	std::vector<std::wstring> m_color_scheme_ids;
};

