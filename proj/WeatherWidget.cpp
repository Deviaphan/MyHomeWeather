#include "pch.h"
#include "WeatherWidget.h"
#include "SmartHdc.h"
#include "CurlBuffer.h"
#include "json/json.h"
#include <fstream>
#include <filesystem>

const CString dayName[7] = { L"Воскресение", L"Понедельник", L"Вторник", L"Среда", L"Четверг", L"Пятница", L"Суббота" };

const CString monthName[12] = { L"Январь", L"Февраль", L"Март", L"Апрель", L"Май", L"Июнь", L"Июль", L"Август", L"Сентябрь", L"Октябрь", L"Ноябрь", L"Декабрь", };


void PremulAlpha( CImage & img )
{
	if( img.IsNull() )
		return;

	if( img.GetBPP() != 32 )
		return;

	unsigned char * bits = static_cast<unsigned char*>(img.GetBits());
	const int pitch = img.GetPitch();

	const int width = img.GetWidth();
	const int height = img.GetHeight();

	for( int y = 0; y < height; ++y )
	{
		unsigned int * pixel = reinterpret_cast<unsigned*>(bits);
		unsigned int * end = pixel + width;

		for( ; pixel < end; ++pixel )
		{
			if( (*pixel & 0xFF000000) == 0 )
			{
				*pixel = 0;
			}
			else
			{
				const unsigned int a = (*pixel >> 24) & 0x000000FF;

				const unsigned int r = ((*pixel & 0xFF) * a) >> 8;
				const unsigned int g = (((*pixel >> 8) & 0xFF) * a) >> 8;
				const unsigned int b = (((*pixel >> 16) & 0xFF) * a) >> 8;

				*pixel = (a << 24) | (b << 16) | (g << 8) | r;
			}
		}

		bits += pitch;
	}
}


int ReadInt( const Json::Value& root, const char* name, int defaultValue = 0 )
{
	const Json::Value value = root[name];
	if( value.isInt() )
	{

		return value.asInt();
	}
	else
	{
		return defaultValue;
	}
}

double ReadDouble( const Json::Value& root, const char* name, double defaultValue = 0.0 )
{
	const Json::Value value = root[name];
	if( value.isDouble() )
	{
		return value.asDouble();
	}
	else
	{
		return defaultValue;
	}
}

std::string ReadString( const Json::Value& root, const char* name, const char* defaultValue = "" )
{
	Json::Value value = root[name];
	if( value.isString() )
	{
		return value.asString();
	}
	else
	{
		return defaultValue;
	}
}

CString Utf8Unicode( const std::string& str )
{
	wchar_t unicode[64] = {};
	MultiByteToWideChar( CP_UTF8, MB_PRECOMPOSED, str.c_str(), (int)str.length(), unicode, 64 );

	return { unicode };
}


IMPLEMENT_DYNAMIC( WeatherWidget, CStatic )

BEGIN_MESSAGE_MAP( WeatherWidget, CStatic )
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

WeatherWidget::WeatherWidget()
	: _width( 0 )
	, _height( 0 )
	, _lastTime( 0 )
	, _temperature( 0 )
	, _iconId( 0 )
	, _lastWeatherIcon( -1 )
{
}

void WeatherWidget::LoadSettings()
{
	std::ifstream fin( "./settings.json" );
	if( fin.is_open() )
	{
		Json::Value root;
		Json::CharReaderBuilder builder;
		JSONCPP_STRING errs;
		if( parseFromStream( builder, fin, &root, &errs ) )
		{
			_latitude = ReadString( root, "latitude", "" );
			_longitude = ReadString( root, "longitude", "" );
			_appid = ReadString( root, "appid", "" );
		}
	}

	if( _appid.empty() || _latitude.empty() || _longitude.empty() )
	{
		AfxMessageBox( L"Необходимо задать appid, широту и долготу", MB_OK | MB_ICONERROR );
		throw std::exception();
	}
}

void WeatherWidget::Initialize()
{
	LoadSettings();

	LOGFONT logFont;
	::ZeroMemory( &logFont, sizeof( logFont ) );
	AfxGetMainWnd()->GetFont()->GetLogFont( &logFont );

	logFont.lfQuality = ANTIALIASED_QUALITY;

	logFont.lfHeight = -150;
	_timeLine1Font.CreateFontIndirect( &logFont );

	logFont.lfHeight = -30;
	_timeLine2Font.CreateFontIndirect( &logFont );

	logFont.lfHeight = -80;
	_weatherFont.CreateFontIndirectW( &logFont );

	std::ifstream iFile( "icons/icons.json" );
	Json::Value root;
	Json::Reader reader;
	reader.parse(iFile, root, false);
	const auto members = root.getMemberNames();

	for( const auto & ii : members )
	{
		const Json::Value & object = root[ii];
		int iconId = 0;
		try
		{
			iconId = std::stoi( ii );
		}
		catch( ... )
		{
		}

		_icons.emplace( iconId, object.asString() );
	}
}

void WeatherWidget::OnPaint()
{
	try
	{
		CPaintDC pdc( this );
		CRect r;
		GetClientRect( r );

		if( _buffer.IsNull() || (_buffer.GetWidth() != r.Width() || _buffer.GetHeight() != r.Height()) )
		{
			if( !_buffer.IsNull() )
				_buffer.Destroy();

			_width = r.Width();
			_height = r.Height();

			_buffer.Create( _width, _height, 24, BI_RGB );
		}

		{
			CImageSmartDc hdc( _buffer );

			DrawBackground( hdc );

			DrawClock( hdc );

			DrawWeather( hdc );
		}

		_buffer.BitBlt( pdc.GetSafeHdc(), r.left, r.top, SRCCOPY );
	}
	catch( ... )
	{
	}
}

BOOL WeatherWidget::OnEraseBkgnd( CDC * /*pDC*/ )
{
	return FALSE;
}

void WeatherWidget::DrawBackground( HDC hdc )
{
	CRect r( 0, 0, _width, _height );

	const COLORREF bgColor = RGB( 57, 57, 57 );
	CBrush bgBrush( bgColor );
	FillRect( hdc, r, bgBrush );
}

void WeatherWidget::DrawClock( HDC hdc )
{
	::SetBkMode( hdc, TRANSPARENT );

	static const CSize offsets[5] = { { -3, -3 }, { -3, 3 }, { 3, -3 }, { 3, 3 }, { 0, 0 } };
	constexpr COLORREF fontColor[5] = { RGB( 0, 0, 0 ), RGB( 0, 0, 0 ), RGB( 0, 0, 0 ), RGB( 0, 0, 0 ), RGB( 255, 255, 255 ) };

	for( int i = 0; i < 5; ++i )
	{
		CRect text1( 30, 0, _width, _height );
		text1.OffsetRect( offsets[i].cx, offsets[i].cy );

		CRect text2( 30, 170, _width, _height );
		text2.OffsetRect( offsets[i].cx, offsets[i].cy );

		::SetTextColor( hdc, fontColor[i] );

		HGDIOBJ oldFont = ::SelectObject( hdc, _timeLine1Font );

		::DrawText( hdc, _timeFirstLine, _timeFirstLine.GetLength(), text1, DT_SINGLELINE );

		::SelectObject( hdc, oldFont );

		oldFont = ::SelectObject( hdc, _timeLine2Font );
		::DrawText( hdc, _timeSecondLine, _timeSecondLine.GetLength(), text2, DT_SINGLELINE );
		::SelectObject( hdc, oldFont );
	}
}

void WeatherWidget::DrawWeather( HDC hdc )
{
	if( !_weatherIcon.IsNull() )
	{
		_weatherIcon.AlphaBlend( hdc, 418, 0 );

		//CPen pen( PS_SOLID, 1, RGB(255,0,0) );
		//::SelectObject( hdc, pen );
		//::MoveToEx( hdc, 418, 0, 0 );
		//::LineTo( hdc, 418 + 256, 0 );
		//::LineTo( hdc, 418 + 256, 256 );
		//::LineTo( hdc, 418, 256 );
		//::LineTo( hdc, 418, 0 );

		LOGFONT logFont;
		::ZeroMemory( &logFont, sizeof( logFont ) );
		AfxGetMainWnd()->GetFont()->GetLogFont( &logFont );
		logFont.lfHeight = -80;
		CFont wFont;
		wFont.CreateFontIndirectW( &logFont );

		static const CSize offsets[5] = {{-3, -3}, {-3, 3}, {3, -3}, {3, 3}, {0, 0}};
		constexpr COLORREF fontColor[5] = {RGB( 0, 0, 0 ), RGB( 0, 0, 0 ), RGB( 0, 0, 0 ), RGB( 0, 0, 0 ), RGB( 230, 235, 235 )};

		HGDIOBJ oldFont = ::SelectObject( hdc, wFont );

		CRect titleRect(0,0,0,0);// (270, 230, 600, 400);
		constexpr int posX = 270;
		constexpr int width = 380;

		::DrawText( hdc, _weatherTitle, _weatherTitle.GetLength(), &titleRect, DT_SINGLELINE | DT_CALCRECT );
		if( titleRect.Width() > width )
		{
			::SelectObject( hdc, oldFont );

			const double scale = (double)width / titleRect.Width();
			logFont.lfHeight = static_cast<LONG>(-80.0 * scale);
			wFont.DeleteObject();
			wFont.CreateFontIndirectW( &logFont );
			oldFont = ::SelectObject( hdc, wFont );
		}

		::SetBkMode( hdc, TRANSPARENT );

		for( int i = 0; i < 5; ++i )
		{
			::SetTextColor( hdc, fontColor[i] );

			::DrawText( hdc, _weatherTitle, _weatherTitle.GetLength(), CRect( posX+offsets[i].cx, 230 + offsets[i].cy, posX + width + offsets[i].cx, 295 + offsets[i].cy ), DT_SINGLELINE | DT_RIGHT | DT_VCENTER | DT_NOCLIP );
		}
		::SelectObject( hdc, oldFont );

		oldFont = ::SelectObject( hdc, _weatherFont );
		for( int i = 0; i < 5; ++i )
		{
			::SetTextColor( hdc, fontColor[i] );

			::DrawText( hdc, _weatherC, _weatherC.GetLength(), CRect( offsets[i].cx, 230 + offsets[i].cy, 250 + offsets[i].cx, 310 + offsets[i].cy ), DT_SINGLELINE | DT_RIGHT | DT_VCENTER | DT_NOCLIP );
		}
		::SelectObject( hdc, oldFont );
	}
}

void WeatherWidget::UpdateClock()
{
	try
	{
		SYSTEMTIME sysTime = {};
		::GetLocalTime( &sysTime );

		const int t = sysTime.wHour + sysTime.wMinute;
		if( _lastTime != t )
		{
			_lastTime = t;

			_timeFirstLine.Format( L"%02i:%02i", (int)sysTime.wHour, (int)sysTime.wMinute );

			_timeSecondLine.Format( L"%s,  %i %s  %i", dayName[sysTime.wDayOfWeek].GetString(), (int)sysTime.wDay, monthName[sysTime.wMonth - 1].GetString(), (int)sysTime.wYear );
		}
	}
	catch( ... )
	{

	}
}

void WeatherWidget::UpdateWeather()
{
	try
	{
		CStringA curlQuery;
		curlQuery.Format( "http://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&lang=ru&units=metric",
			_latitude.c_str(),
			_longitude.c_str(),
			_appid.c_str() );

		Curl curl( 30, false );

		CurlBuffer chunk;

		VerifyCurl( curl_easy_setopt( curl, CURLOPT_URL, curlQuery.GetString() ) );
		VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, CurlBuffer::WriteMemoryCallback ) );
		VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEDATA, &chunk ) );

		const CURLcode result = curl_easy_perform( curl );
		if( CURLE_OK != result )
			return;

		chunk.TerminateBuffer();

		const char * json = static_cast<const char*>(chunk.GetData());

		Json::Value root;
		Json::Reader reader;
		reader.parse( json, json + chunk.GetSize(), root );

		{
			const Json::Value& weather = root["weather"];
			if( !weather.isArray() )
				return;
			if( weather.empty() )
				return;

			const Json::Value& object = weather[0];
			if( !object.isObject() )
				return;

			_iconId = ReadInt( object, "id", 0 );
			_iconName = Utf8Unicode( ReadString( object, "icon", "unknown" ) );
			_weatherTitle = Utf8Unicode( ReadString( object, "description" ) );
		}

		{
			const Json::Value& main = root["main"];
			if( !main.isObject() )
				return;

			_temperature = (float)ReadDouble( main, "temp", 0.0 );

			//_feelsLike = ReadInt( root, "temperatureFeelsLike", 0 );
			//_humidity = ReadInt( root, "relativeHumidity", 0 );
		}

		CString folder = L"day";
		if( _iconName.Find( L"n" ) > 0 )
			folder = L"night";

		if( _lastWeatherIcon != _iconId || _lastFolder.Compare( folder ) != 0 )
		{
			_lastWeatherIcon = _iconId;
			_lastFolder = folder;

			if( !_weatherIcon.IsNull() )
				_weatherIcon.Destroy();

			static wchar_t workPath[MAX_PATH] = L"";
			if( workPath[0] == L'\0' )
			{
				HMODULE thisModule = nullptr;
				VERIFY( GetModuleHandleEx( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR) __FUNCTION__, &thisModule ) );
				VERIFY( GetModuleFileName( thisModule, workPath, MAX_PATH ) );
				VERIFY( PathRemoveFileSpec( workPath ) );
			}
			
			CString fileName = L"clear_sky";
			auto ii = _icons.find( _iconId );
			if( ii != _icons.end() )
				fileName = ii->second.c_str();

			CString path;
			path.Format( L"%s\\icons\\%s\\%s.png", workPath, folder.GetString(), fileName.GetString() );

			std::error_code errCode{};
			if( !std::filesystem::exists( path.GetString(), errCode ) )
			{
				path.Format( L"%s\\icons\\%s\\%s.png", workPath, folder.GetString(), L"clear_sky" );
			}

			if( _weatherIcon.Load( path ) == 0 )
			{
				PremulAlpha( _weatherIcon );
			}
		}

		_weatherC.Format( L"%.1f°", _temperature );
	}
	catch( ... )
	{

	}
}
