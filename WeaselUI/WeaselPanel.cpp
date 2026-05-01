#include "stdafx.h"
#include "WeaselPanel.h"
#include <WeaselCommon.h>
#include <Usp10.h>

#include "VerticalLayout.h"
#include "HorizontalLayout.h"
#include "FullScreenLayout.h"

// for IDI_ZH, IDI_EN
#include <resource.h>

using namespace weasel;

WeaselPanel::WeaselPanel(weasel::UI &ui)
	: m_layout(NULL), 
	  m_ctx(ui.ctx()), 
	  m_status(ui.status()), 
	  m_style(ui.style())
{
	m_iconDisabled.LoadIconW(IDI_RELOAD, STATUS_ICON_SIZE, STATUS_ICON_SIZE, LR_DEFAULTCOLOR);
	m_iconEnabled.LoadIconW(IDI_ZH, STATUS_ICON_SIZE, STATUS_ICON_SIZE, LR_DEFAULTCOLOR);
	m_iconAlpha.LoadIconW(IDI_EN, STATUS_ICON_SIZE, STATUS_ICON_SIZE, LR_DEFAULTCOLOR);
}

WeaselPanel::~WeaselPanel()
{
	if (m_layout != NULL)
		delete m_layout;
}

void WeaselPanel::_ResizeWindow()
{
	CDCHandle dc = GetDC();
	long fontHeight = -MulDiv(m_style.font_point, dc.GetDeviceCaps(LOGPIXELSY), 72);
	CFont font;
	font.CreateFontW(fontHeight, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, m_style.font_face.c_str());
	dc.SelectFont(font);

	CSize size = m_layout->GetContentSize();
	SetWindowPos(NULL, 0, 0, size.cx, size.cy, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
	ReleaseDC(dc);
}

void WeaselPanel::_CreateLayout()
{
	if (m_layout != NULL)
		delete m_layout;

	Layout* layout = NULL;
	if (m_style.layout_type == UIStyle::LAYOUT_VERTICAL ||
		m_style.layout_type == UIStyle::LAYOUT_VERTICAL_FULLSCREEN)
	{
		layout = new VerticalLayout(m_style, m_ctx, m_status);
	}
	else if (m_style.layout_type == UIStyle::LAYOUT_HORIZONTAL ||
		m_style.layout_type == UIStyle::LAYOUT_HORIZONTAL_FULLSCREEN)
	{
		layout = new HorizontalLayout(m_style, m_ctx, m_status);
	}
	if (m_style.layout_type == UIStyle::LAYOUT_VERTICAL_FULLSCREEN ||
		m_style.layout_type == UIStyle::LAYOUT_HORIZONTAL_FULLSCREEN)
	{
		layout = new FullScreenLayout(m_style, m_ctx, m_status, m_inputPos, layout);
	}
	m_layout = layout;
}

//更新界面
void WeaselPanel::Refresh()
{
	_CreateLayout();

	CDCHandle dc = GetDC();
	long fontHeight = -MulDiv(m_style.font_point, dc.GetDeviceCaps(LOGPIXELSY), 72);
	CFont font;
	font.CreateFontW(fontHeight, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, m_style.font_face.c_str());
	dc.SelectFont(font);
	m_layout->DoLayout(dc);
	ReleaseDC(dc);

	_ResizeWindow();
	_RepositionWindow();
	RedrawWindow();
}

void WeaselPanel::_HighlightText(CDCHandle dc, CRect rc, COLORREF color)
{
	rc.InflateRect(m_style.hilite_padding, m_style.hilite_padding);
	CBrush brush;
	brush.CreateSolidBrush(color);
	CBrush oldBrush = dc.SelectBrush(brush);
	CPen pen;
	pen.CreatePen(PS_SOLID, 0, color);
	CPen oldPen = dc.SelectPen(pen);
	CPoint ptRoundCorner(m_style.round_corner, m_style.round_corner);
	dc.RoundRect(rc, ptRoundCorner);
	dc.SelectBrush(oldBrush);
	dc.SelectPen(oldPen);
}

bool WeaselPanel::_DrawPreedit(Text const& text, CDCHandle dc, CRect const& rc)
{
	bool drawn = false;
	std::wstring const& t = text.str;
	if (!t.empty())
	{
		weasel::TextRange range;
		std::vector<weasel::TextAttribute> const& attrs = text.attributes;
		for (size_t j = 0; j < attrs.size(); ++j)
			if (attrs[j].type == weasel::HIGHLIGHTED)
				range = attrs[j].range;

		if (range.start < range.end)
		{
			CSize selStart, selEnd;
			dc.GetTextExtent(t.c_str(), range.start, &selStart);
			dc.GetTextExtent(t.c_str(), range.end, &selEnd);
			int x = rc.left;
			if (range.start > 0)
			{
				// zzz
				std::wstring str_before(t.substr(0, range.start));
				CRect rc_before(x, rc.top, rc.left + selStart.cx, rc.bottom);
				_TextOut(dc, x, rc.top, rc_before, str_before.c_str(), str_before.length());
				x += selStart.cx + m_style.hilite_spacing;
			}
			{
				// zzz[yyy]
				std::wstring str_highlight(t.substr(range.start, range.end - range.start));
				CRect rc_hi(x, rc.top, x + (selEnd.cx - selStart.cx), rc.bottom);
				_HighlightText(dc, rc_hi, m_style.hilited_back_color);
				dc.SetTextColor(m_style.hilited_text_color);
				dc.SetBkColor(m_style.hilited_back_color);
				_TextOut(dc, x, rc.top, rc_hi, str_highlight.c_str(), str_highlight.length());
				dc.SetTextColor(m_style.text_color);
				dc.SetBkColor(m_style.back_color);
				x += (selEnd.cx - selStart.cx);
			}
			if (range.end < static_cast<int>(t.length()))
			{
				// zzz[yyy]xxx
				x += m_style.hilite_spacing;
				std::wstring str_after(t.substr(range.end));
				CRect rc_after(x, rc.top, rc.right, rc.bottom);
				_TextOut(dc, x, rc.top, rc_after, str_after.c_str(), str_after.length());
			}
		}
		else
		{
			CRect rcText(rc.left, rc.top, rc.right, rc.bottom);
			_TextOut(dc, rc.left, rc.top, rcText, t.c_str(), t.length());
		}
		drawn = true;
	}
	return drawn;
}

bool WeaselPanel::_DrawCandidates(CDCHandle dc)
{
	bool drawn = false;
	const std::vector<Text> &candidates(m_ctx.cinfo.candies);
	const std::vector<Text> &comments(m_ctx.cinfo.comments);
	const std::vector<Text> &labels(m_ctx.cinfo.labels);

	for (size_t i = 0; i < candidates.size() && i < MAX_CANDIDATES_COUNT; ++i)
	{
		CRect rect;
		bool isHovered = (m_style.hover_type != UIStyle::HOVER_NONE && 
			m_hoveredCandidate >= 0 && static_cast<size_t>(m_hoveredCandidate) == i);
		bool isKeyboardHighlighted = (i == m_ctx.cinfo.highlighted);
		bool shouldHighlight = isKeyboardHighlighted || isHovered;
		bool isSemiHilite = (isHovered && m_style.hover_type == UIStyle::HOVER_SEMI_HILITE && !isKeyboardHighlighted);

		if (shouldHighlight)
		{
			if (isSemiHilite)
			{
				CBrush brush;
				brush.CreateSolidBrush(m_style.hilited_candidate_back_color);
				CRgn rgn;
				rgn.CreateRectRgnIndirect(m_layout->GetHighlightRect());
				BLENDFUNCTION blend = { AC_SRC_OVER, 0, 128, 0 };
				CDCHandle memDC = CreateCompatibleDC(dc);
				CBitmap bmp;
				bmp.CreateCompatibleBitmap(dc, m_layout->GetHighlightRect().Width(), m_layout->GetHighlightRect().Height());
				CBitmapHandle oldBmp = memDC.SelectBitmap(bmp);
				memDC.FillRgn(rgn, brush);
				AlphaBlend(dc, m_layout->GetHighlightRect().left, m_layout->GetHighlightRect().top,
					m_layout->GetHighlightRect().Width(), m_layout->GetHighlightRect().Height(),
					memDC, 0, 0, m_layout->GetHighlightRect().Width(), m_layout->GetHighlightRect().Height(), blend);
				memDC.SelectBitmap(oldBmp);
				brush.DeleteObject();
				rgn.DeleteObject();
			}
			else
			{
				_HighlightText(dc, m_layout->GetHighlightRect(), m_style.hilited_candidate_back_color);
			}
			dc.SetTextColor(m_style.hilited_label_text_color);
		}
		else
			dc.SetTextColor(m_style.label_text_color);

		// Draw label
		std::wstring label = m_layout->GetLabelText(labels, i, m_style.label_text_format.c_str());
		rect = m_layout->GetCandidateLabelRect(i);
		_TextOut(dc, rect.left, rect.top, rect, label.c_str(), label.length());

		// Draw text
		std::wstring text = candidates.at(i).str;
		if (m_style.candidate_abbreviate_length > 0 && 
			static_cast<int>(text.length()) > m_style.candidate_abbreviate_length)
		{
			text = text.substr(0, m_style.candidate_abbreviate_length) + L"...";
		}
		if (shouldHighlight)
		{
			if (isSemiHilite)
				dc.SetTextColor(RGB(
					(GetRValue(m_style.candidate_text_color) + GetRValue(m_style.hilited_candidate_text_color)) / 2,
					(GetGValue(m_style.candidate_text_color) + GetGValue(m_style.hilited_candidate_text_color)) / 2,
					(GetBValue(m_style.candidate_text_color) + GetBValue(m_style.hilited_candidate_text_color)) / 2));
			else
				dc.SetTextColor(m_style.hilited_candidate_text_color);
		}
		else
			dc.SetTextColor(m_style.candidate_text_color);
		rect = m_layout->GetCandidateTextRect(i);
		_TextOut(dc, rect.left, rect.top, rect, text.c_str(), text.length());
		
		// Draw comment
		std::wstring comment = comments.at(i).str;
		if (!comment.empty())
		{
			if (shouldHighlight)
			{
				if (isSemiHilite)
					dc.SetTextColor(RGB(
						(GetRValue(m_style.comment_text_color) + GetRValue(m_style.hilited_comment_text_color)) / 2,
						(GetGValue(m_style.comment_text_color) + GetGValue(m_style.hilited_comment_text_color)) / 2,
						(GetBValue(m_style.comment_text_color) + GetBValue(m_style.hilited_comment_text_color)) / 2));
				else
					dc.SetTextColor(m_style.hilited_comment_text_color);
			}
			else
				dc.SetTextColor(m_style.comment_text_color);
			rect = m_layout->GetCandidateCommentRect(i);
			_TextOut(dc, rect.left, rect.top, rect, comment.c_str(), comment.length());
		}
		drawn = true;
	}
	dc.SetTextColor(m_style.text_color);
	return drawn;
}

void WeaselPanel::_DrawShadow(CDCHandle dc, CRect const& rc)
{
	if (m_style.shadow_radius <= 0)
		return;

	int r = GetRValue(m_style.shadow_color);
	int g = GetGValue(m_style.shadow_color);
	int b = GetBValue(m_style.shadow_color);
	int radius = m_style.shadow_radius;
	int offsetX = m_style.shadow_offset_x;
	int offsetY = m_style.shadow_offset_y;

	CDCHandle memDC = CreateCompatibleDC(dc);
	CBitmap bmp;
	int shadowWidth = rc.Width() + radius * 2;
	int shadowHeight = rc.Height() + radius * 2;
	bmp.CreateCompatibleBitmap(dc, shadowWidth, shadowHeight);
	CBitmapHandle oldBmp = memDC.SelectBitmap(bmp);

	memDC.SetBkMode(TRANSPARENT);

	for (int i = 0; i < radius; ++i)
	{
		int alpha = (radius - i) * 60 / radius;
		if (alpha < 1) alpha = 1;
		if (alpha > 255) alpha = 255;
		COLORREF shadowColor = RGB(r * alpha / 255, g * alpha / 255, b * alpha / 255);
		CPen pen;
		pen.CreatePen(PS_SOLID, 1, shadowColor);
		CPenHandle oldPen = memDC.SelectPen(pen);
		CBrush brush;
		brush.CreateSolidBrush(shadowColor);
		CBrushHandle oldBrush = memDC.SelectBrush(brush);

		CRect shadowRect(i, i, shadowWidth - i, shadowHeight - i);
		CPoint ptRound(m_style.round_corner, m_style.round_corner);
		memDC.RoundRect(shadowRect, ptRound);

		memDC.SelectPen(oldPen);
		memDC.SelectBrush(oldBrush);
		pen.DeleteObject();
		brush.DeleteObject();
	}

	BLENDFUNCTION blend = { AC_SRC_OVER, 0, 180, AC_SRC_ALPHA };
	AlphaBlend(dc, rc.left - radius + offsetX, rc.top - radius + offsetY,
		shadowWidth, shadowHeight,
		memDC, 0, 0, shadowWidth, shadowHeight, blend);

	memDC.SelectBitmap(oldBmp);
}

//draw client area
void WeaselPanel::DoPaint(CDCHandle dc)
{
	CRect rc;
	GetClientRect(&rc);

	_DrawShadow(dc, rc);

	// background
	{
		CBrush brush;
		brush.CreateSolidBrush(m_style.back_color);
		CRgn rgn;
		rgn.CreateRectRgnIndirect(&rc);
		dc.FillRgn(rgn, brush);

		CPen pen;
		pen.CreatePen(PS_SOLID | PS_INSIDEFRAME, m_style.border, m_style.border_color);
		CPenHandle oldPen = dc.SelectPen(pen);
		CBrushHandle oldBrush = dc.SelectBrush(brush);
		dc.Rectangle(&rc);
		dc.SelectPen(oldPen);
		dc.SelectBrush(oldBrush);
	}

	long height = -MulDiv(m_style.font_point, dc.GetDeviceCaps(LOGPIXELSY), 72);

	CFont font;
	font.CreateFontW(height, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, m_style.font_face.c_str());
	CFontHandle oldFont = dc.SelectFont(font);

	dc.SetTextColor(m_style.text_color);
	dc.SetBkColor(m_style.back_color);
	dc.SetBkMode(TRANSPARENT);
	
	bool drawn = false;

	// draw preedit string
	if (!m_layout->IsInlinePreedit())
		drawn |= _DrawPreedit(m_ctx.preedit, dc, m_layout->GetPreeditRect());
	
	// draw auxiliary string
	drawn |= _DrawPreedit(m_ctx.aux, dc, m_layout->GetAuxiliaryRect());

	// status icon (I guess Metro IME stole my idea :)
	if (m_layout->ShouldDisplayStatusIcon())
	{
		const CRect iconRect(m_layout->GetStatusIconRect());
		CIcon* pIcon = NULL;
		if (m_status.disabled)
		{
			pIcon = &m_iconDisabled;
		}
		else if (m_status.ascii_mode)
		{
			pIcon = m_iconHalf.IsNull() ? &m_iconAlpha : &m_iconHalf;
		}
		else
		{
			pIcon = m_iconFull.IsNull() ? &m_iconEnabled : &m_iconFull;
		}
		dc.DrawIconEx(iconRect.left, iconRect.top, *pIcon, 0, 0);
		drawn = true;
	}

	// draw candidates
	drawn |= _DrawCandidates(dc);

	/* Nothing drawn, hide candidate window */
	if (!drawn)
		ShowWindow(SW_HIDE);

	dc.SelectFont(oldFont);	
}

LRESULT WeaselPanel::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_hoveredCandidate = -1;
	m_trackingMouse = false;
	if (!m_style.full_icon.empty())
		_LoadCustomIcon(m_iconFull, m_style.full_icon);
	if (!m_style.half_icon.empty())
		_LoadCustomIcon(m_iconHalf, m_style.half_icon);
	Refresh();
	GetWindowRect(&m_inputPos);
	return TRUE;
}

LRESULT WeaselPanel::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (!m_iconFull.IsNull())
		m_iconFull.DestroyIcon();
	if (!m_iconHalf.IsNull())
		m_iconHalf.DestroyIcon();
	return 0;
}

LRESULT WeaselPanel::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_style.hover_type == UIStyle::HOVER_NONE)
	{
		bHandled = FALSE;
		return 0;
	}

	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	_UpdateHoverFromPoint(pt);
	_StartTrackingMouse();
	return 0;
}

LRESULT WeaselPanel::OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_trackingMouse = false;
	if (m_hoveredCandidate != -1)
	{
		m_hoveredCandidate = -1;
		RedrawWindow();
	}
	return 0;
}

LRESULT WeaselPanel::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_hoveredCandidate >= 0)
	{
		::SendMessage(GetParent(), WM_CANDIDATE_CLICKED, m_hoveredCandidate, 0);
	}
	return 0;
}

void WeaselPanel::_StartTrackingMouse()
{
	if (!m_trackingMouse)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		tme.dwHoverTime = 0;
		TrackMouseEvent(&tme);
		m_trackingMouse = true;
	}
}

void WeaselPanel::_UpdateHoverFromPoint(POINT pt)
{
	int newHover = -1;
	const std::vector<Text>& candidates = m_ctx.cinfo.candies;
	for (size_t i = 0; i < candidates.size() && i < MAX_CANDIDATES_COUNT; ++i)
	{
		CRect rect = m_layout->GetCandidateRect(i);
		if (rect.PtInRect(pt))
		{
			newHover = static_cast<int>(i);
			break;
		}
	}
	if (newHover != m_hoveredCandidate)
	{
		m_hoveredCandidate = newHover;
		RedrawWindow();
	}
}

bool WeaselPanel::_LoadCustomIcon(CIcon& icon, const std::wstring& path)
{
	HICON hIcon = (HICON)LoadImageW(NULL, path.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	if (hIcon)
	{
		icon.Attach(hIcon);
		return true;
	}
	return false;
}

long WeaselPanel::_GetFontHeight(CDCHandle dc) const
{
	return -MulDiv(m_style.font_point, dc.GetDeviceCaps(LOGPIXELSY), 72);
}

void WeaselPanel::CloseDialog(int nVal)
{
	
}

void WeaselPanel::MoveTo(RECT const& rc)
{
	const int distance = 6;
	m_inputPos = rc;
	m_inputPos.OffsetRect(0, distance);
	_RepositionWindow();
}

void WeaselPanel::_RepositionWindow()
{
	RECT rcWorkArea;
	//SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0);
	memset(&rcWorkArea, 0, sizeof(rcWorkArea));
	HMONITOR hMonitor = MonitorFromRect(m_inputPos, MONITOR_DEFAULTTONEAREST);
	if (hMonitor)
	{
		MONITORINFO info;
		info.cbSize = sizeof(MONITORINFO);
		if (GetMonitorInfo(hMonitor, &info))
		{
			rcWorkArea = info.rcWork;
		}
	}
	RECT rcWindow;
	GetWindowRect(&rcWindow);
	int width = (rcWindow.right - rcWindow.left);
	int height = (rcWindow.bottom - rcWindow.top);
	// keep panel visible
	rcWorkArea.right -= width;
	rcWorkArea.bottom -= height;
	int x = m_inputPos.left;
	int y = m_inputPos.bottom;
	if (x > rcWorkArea.right)
		x = rcWorkArea.right;
	if (x < rcWorkArea.left)
		x = rcWorkArea.left;
	// show panel above the input focus if we're around the bottom
	if (y > rcWorkArea.bottom)
		y = m_inputPos.top - height;
	if (y > rcWorkArea.bottom)
		y = rcWorkArea.bottom;
	if (y < rcWorkArea.top)
		y = rcWorkArea.top;
	// memorize adjusted position (to avoid window bouncing on height change)
	m_inputPos.bottom = y;
	SetWindowPos(HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE|SWP_NOACTIVATE);
}

static HRESULT _TextOutWithFallback(CDCHandle dc, int x, int y, CRect const& rc, LPCWSTR psz, int cch)
{
    SCRIPT_STRING_ANALYSIS ssa;
    HRESULT hr;

    hr = ScriptStringAnalyse(
        dc,
        psz, cch,
        2 * cch + 16,
        -1,
        SSA_GLYPHS|SSA_FALLBACK|SSA_LINK,
        0,
        NULL, // control
        NULL, // state
        NULL, // piDx
        NULL,
        NULL, // pbInClass
        &ssa);

    if (SUCCEEDED(hr))
    {
        hr = ScriptStringOut(
            ssa, x, y, 0,
            &rc,
            0, 0, FALSE);
    }

	ScriptStringFree(&ssa);
	return hr;
}

void WeaselPanel::_TextOut(CDCHandle dc, int x, int y, CRect const& rc, LPCWSTR psz, int cch)
{
	if (FAILED(_TextOutWithFallback(dc, x, y, rc, psz, cch))) {
		dc.TextOutW(x, y, psz, cch);
	}
}
