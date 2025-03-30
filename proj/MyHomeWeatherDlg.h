#pragma once
#include "WeatherWidget.h"


class CMyHomeWeatherDlg : public CDialog
{
public:
	CMyHomeWeatherDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MYHOMEWEATHER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd( CDC * pDC );
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnSize( UINT nType, int cx, int cy );

// Implementation
protected:
	HICON m_hIcon;

private:
	CPoint _dragPoint;
	CRect _rDrag;
	
	bool _firstTimerStart;
	WeatherWidget _weatherWidget;

};
