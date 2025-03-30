#pragma once
#include <afxwin.h>
#include <string>
#include <map>

class WeatherWidget: public CStatic
{
	DECLARE_DYNAMIC( WeatherWidget );
public:
	WeatherWidget();

protected:
	void LoadSettings();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd( CDC * pDC );

	void DrawBackground( HDC hdc );

	void DrawClock( HDC hdc );
	void DrawWeather( HDC hdc );

public:
	void Initialize();
	void UpdateClock();
	void UpdateWeather();

private:
	CImage _buffer;
	int _width;
	int _height;

	// clock
	int _lastTime;
	CString _timeFirstLine;
	CString _timeSecondLine;
	CFont _timeLine1Font;
	CFont _timeLine2Font;
	// clock

	// weather
	CString _weatherTitle;
	CString _weatherC;
	CString _iconName;
	float _temperature;
	int _iconId;
	CImage _weatherIcon;
	int _lastWeatherIcon;
	CString _lastFolder;
	CFont _weatherFont;

	std::map<int, std::string> _icons;

	::std::string _latitude;
	::std::string _longitude;
	::std::string _appid;
};

