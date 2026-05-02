#pragma once
#include <WeaselCommon.h>
#include <WeaselUI.h>
#include "Layout.h"

typedef CWinTraits<WS_POPUP|WS_CLIPSIBLINGS|WS_DISABLED, WS_EX_TOOLWINDOW|WS_EX_TOPMOST> CWeaselPanelTraits;

class WeaselPanel : 
	public CWindowImpl<WeaselPanel, CWindow, CWeaselPanelTraits>,
	CDoubleBufferImpl<WeaselPanel>
{
public:
	DECLARE_WND_CLASS_EX(L"WeaselPanel", CS_DROPSHADOW, COLOR_WINDOW)

	BEGIN_MSG_MAP(WeaselPanel)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		CHAIN_MSG_MAP(CDoubleBufferImpl<WeaselPanel>)
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void CloseDialog(int nVal);

	WeaselPanel(weasel::UI &ui);
	~WeaselPanel();

	void MoveTo(RECT const& rc);
	void Refresh();

	void DoPaint(CDCHandle dc);

private:
	void _CreateLayout();
	void _ResizeWindow();
	void _RepositionWindow();
	bool _DrawPreedit(weasel::Text const& text, CDCHandle dc, CRect const& rc);
	bool _DrawCandidates(CDCHandle dc);
	void _HighlightText(CDCHandle dc, CRect rc, COLORREF color);
	void _TextOut(CDCHandle dc, int x, int y, CRect const& rc, LPCWSTR psz, int cch);
	long _GetFontHeight(CDCHandle dc) const;
	void _StartTrackingMouse();
	void _UpdateHoverFromPoint(POINT pt);
	void _DrawShadow(CDCHandle dc, CRect const& rc);

	weasel::Layout *m_layout;
	weasel::Context &m_ctx;
	weasel::Status &m_status;
	weasel::UIStyle &m_style;

	CRect m_inputPos;
	CIcon m_iconDisabled;
	CIcon m_iconEnabled;
	CIcon m_iconAlpha;
	CIcon m_iconFull;
	CIcon m_iconHalf;
	int m_hoveredCandidate;
	bool m_trackingMouse;
	bool _LoadCustomIcon(CIcon& icon, const std::wstring& path);
};
