#include "pch.h"
#include "MyHomeWeatherDlg.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

enum
{
	UPDATE_TIME = 123,
	UPDATE_WEATHER,
};

CMyHomeWeatherDlg::CMyHomeWeatherDlg( CWnd* pParent /*=nullptr*/ )
	: CDialog( IDD_MYHOMEWEATHER_DIALOG, pParent )
{
	m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
}

void CMyHomeWeatherDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_MASTER_GROUP, _weatherWidget );
}

BEGIN_MESSAGE_MAP( CMyHomeWeatherDlg, CDialog )
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CMyHomeWeatherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon( m_hIcon, TRUE );			// Set big icon
	SetIcon( m_hIcon, FALSE );		// Set small icon

	CWaitCursor waiting;

	//EnumDisplayMonitors( nullptr, nullptr, MonitorSizes, (LPARAM)this );
	HWND hWnd = GetSafeHwnd();
	::SetWindowLong( hWnd, GWL_EXSTYLE, GetWindowLong( hWnd, GWL_EXSTYLE ) | WS_EX_LAYERED );
	::SetLayeredWindowAttributes( hWnd, RGB( 0, 0, 0 ), 210, LWA_ALPHA );

	const int sx = ::GetSystemMetrics( SM_XVIRTUALSCREEN );
	const int sy = ::GetSystemMetrics( SM_YVIRTUALSCREEN );
	const int sizeX = ::GetSystemMetrics( SM_CXSIZEFRAME );
	SetWindowPos( nullptr, sx - sizeX, sy, 690, 340, SWP_FRAMECHANGED | SWP_SHOWWINDOW );

	SYSTEMTIME sysTime = {};
	::GetLocalTime( &sysTime );


	{
		_weatherWidget.Initialize();

		_weatherWidget.UpdateClock();

		_weatherWidget.UpdateWeather();

		_weatherWidget.Invalidate();
	}


	SetTimer( UPDATE_TIME, (60 - sysTime.wSecond + 3) * 1000, nullptr );
	SetTimer( UPDATE_WEATHER, 10 * 60 * 1000, nullptr );


	return TRUE;
}

void CMyHomeWeatherDlg::OnPaint()
{
	if( IsIconic() )
	{
		CPaintDC dc( this );

		SendMessage( WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0 );

		int cxIcon = GetSystemMetrics( SM_CXICON );
		int cyIcon = GetSystemMetrics( SM_CYICON );
		CRect rect;
		GetClientRect( &rect );
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon( x, y, m_hIcon );
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CMyHomeWeatherDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CMyHomeWeatherDlg::OnEraseBkgnd( CDC* /*pDC*/ )
{
	return FALSE;
}

void CMyHomeWeatherDlg::OnTimer( UINT_PTR nIDEvent )
{
	try
	{
		switch( nIDEvent )
		{
			case UPDATE_TIME:
			{
				if( _firstTimerStart )
				{
					_firstTimerStart = false;
					KillTimer( UPDATE_TIME );
					SetTimer( UPDATE_TIME, 60 * 1000, nullptr );
				}

				_weatherWidget.UpdateClock();

				_weatherWidget.Invalidate();
				break;
			}
			case UPDATE_WEATHER:
			{
				CWaitCursor waiting;

				_weatherWidget.UpdateWeather();

				_weatherWidget.Invalidate();
				break;
			}
			default:
			{
				CDialog::OnTimer( nIDEvent );
				break;
			}
		}
	}
	catch( ... )
	{
	}
}

void CMyHomeWeatherDlg::OnDestroy()
{
	KillTimer( UPDATE_TIME );
	KillTimer( UPDATE_WEATHER );

	CDialog::OnDestroy();
}

void CMyHomeWeatherDlg::OnLButtonDown( UINT nFlags, CPoint point )
{
	SetCapture();
	_dragPoint = point;
	GetWindowRect( &_rDrag );

	CDialog::OnLButtonDown( nFlags, point );
}

void CMyHomeWeatherDlg::OnMouseMove( UINT nFlags, CPoint point )
{
	if( (nFlags & MK_LBUTTON) && GetCapture() == this )
	{
		const CPoint delta = point - _dragPoint;

		OffsetRect( &_rDrag, delta.x, delta.y );
		SetWindowPos( NULL, _rDrag.left, _rDrag.top, _rDrag.Width(), _rDrag.Height(), SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );// | SWP_MOVE);
	}
	CDialog::OnMouseMove( nFlags, point );
}

void CMyHomeWeatherDlg::OnLButtonUp( UINT nFlags, CPoint point )
{
	if( GetCapture() == this )
	{
		ReleaseCapture();
	}
	CDialog::OnLButtonUp( nFlags, point );
}

void CMyHomeWeatherDlg::OnSize( UINT nType, int cx, int cy )
{
	CDialog::OnSize( nType, cx, cy );

	if( _weatherWidget.GetSafeHwnd() )
		_weatherWidget.SetWindowPos( 0, 0, 0, cx, cy, 0 );
}
