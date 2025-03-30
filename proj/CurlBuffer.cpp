// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CurlBuffer.h"
#include <string>

const char * GetRandomAgent();

void Curl::GlobalInit()
{
	curl_global_init( CURL_GLOBAL_ALL );
}

void Curl::GlobalCleanup()
{
	curl_global_cleanup();
}

Curl::Curl( unsigned timeout, bool allowProxy, const char * userAgent )
	: _allowProxy(allowProxy)
	, _curlHandle( nullptr )
{
	_curlHandle = curl_easy_init();
	curl_easy_setopt( _curlHandle, CURLOPT_USERAGENT, userAgent ? userAgent : GetRandomAgent() );
	curl_easy_setopt( _curlHandle, CURLOPT_TIMEOUT, timeout );

	curl_easy_setopt( _curlHandle, CURLOPT_SSL_VERIFYPEER, 0 );
	curl_easy_setopt( _curlHandle, CURLOPT_SSL_VERIFYHOST, 0 );

	curl_easy_setopt( _curlHandle, CURLOPT_FOLLOWLOCATION, 1 );
}

Curl::~Curl()
{
	Destroy();
}

void Curl::Destroy()
{
	if( _curlHandle )
	{
		curl_easy_cleanup( _curlHandle );
		_curlHandle = nullptr;
	}
}

void Curl::InitHeaders()
{
	VerifyCurl( curl_easy_setopt( _curlHandle, CURLOPT_HEADER, 0 ) );

	_curlList.header = curl_slist_append( _curlList.header, "Content-Type: text/html; charset=UTF-8" );
	_curlList.header = curl_slist_append( _curlList.header, "Accept-Language: ru-RU,ru;q=0.8,en-US;q=0.5,en;q=0.3" );
	_curlList.header = curl_slist_append( _curlList.header, "DNT: 1" );
	_curlList.header = curl_slist_append( _curlList.header, "Cache-Control: max-age=0'" );
	_curlList.header = curl_slist_append( _curlList.header, "Connection: keep-alive" );

	VerifyCurl( curl_easy_setopt( _curlHandle, CURLOPT_HTTPHEADER, _curlList.header ) );
}

void Curl::InitProxy( ProxyType proxyType, const char * proxy )
{
	if( !_allowProxy )
		return;

	if( !proxy )
		return;

	switch( proxyType )
	{
		case ProxyType::NoProxy:
			break;

		case ProxyType::UseTor:
		{
			VerifyCurl( curl_easy_setopt( _curlHandle, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5 ) );
			VerifyCurl( curl_easy_setopt( _curlHandle, CURLOPT_PROXY, proxy ) );

			break;
		}
		case ProxyType::UseProxy:
		{
			VerifyCurl( curl_easy_setopt( _curlHandle, CURLOPT_PROXY, proxy ) );
			break;
		}
		default:
			break;
	}

	return;
}

size_t CurlBuffer::WriteMemoryCallback( void *contents, size_t size, size_t count, void *userp )
{
	const size_t writenBytes = size * count;

	CurlBuffer * curlBuffer = (CurlBuffer*)userp;
	if( curlBuffer )
		curlBuffer->WriteBytes( contents, writenBytes );

	return writenBytes;
}

const char * GetRandomAgent()
{
	static const std::string userAgents[] =
	{
		"Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)",
		"Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.0)",
		"Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; Trident/6.0)",
		"Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.3; Trident/7.0; .NET4.0E; .NET4.0C)",
		"Mozilla/4.0 (compatible; MSIE 7.0; Windows Phone OS 7.0; Trident/3.1; IEMobile/7.0)",
		"Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; Trident/4.0)",
		"Mozilla/5.0 (Android 4.2; rv:19.0) Gecko/20121129 Firefox/19.0",
		"Mozilla/5.0 (Android 6.0.1; Mobile; rv:48.0) Gecko/48.0 Firefox/48.0",
		"Mozilla/5.0 (Android; Linux armv7l; rv:10.0.1) Gecko/20100101 Firefox/10.0.1 Fennec/10.0.1",
		"Mozilla/5.0 (Android; Mobile; rv:35.0) Gecko/35.0 Firefox/35.0",
		"Mozilla/5.0 (BB10; Touch) AppleWebKit/537.10+ (KHTML, like Gecko) Version/10.1.0.2342 Mobile Safari/537.10+",
		"Mozilla/5.0 (Linux U; en-US)  AppleWebKit/528.5  (KHTML, like Gecko, Safari/528.5 ) Version/4.0 Kindle/3.0 (screen 600x800; rotate)",
		"Mozilla/5.0 (Linux; Android 4.3; SPH-L710 Build/JSS15J) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/32.0.1700.99 Mobile Safari/537.36",
		"Mozilla/5.0 (Linux; Android 4.4.2; SAMSUNG-SM-T537A Build/KOT49H) AppleWebKit/537.36 (KHTML like Gecko) Chrome/35.0.1916.141 Safari/537.36",
		"Mozilla/5.0 (Linux; Android 4.4.2; SM-T230NU Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.81 Safari/537.36",
		"Mozilla/5.0 (Linux; Android 4.4.4; Nexus 7 Build/KTU84P) AppleWebKit/537.36 (KHTML like Gecko) Chrome/36.0.1985.135 Safari/537.36",
		"Mozilla/5.0 (Linux; Android 4.4; Nexus 5 Build/BuildID) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/30.0.0.0 Mobile Safari/537.36",
		"Mozilla/5.0 (Linux; Android 5.0.1; SCH-R970 Build/LRX22C) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/45.0.2454.84 Mobile Safari/537.36",
		"Mozilla/5.0 (Linux; Android 5.0.2; SAMSUNG SM-T530NU Build/LRX22G) AppleWebKit/537.36 (KHTML, like Gecko) SamsungBrowser/3.2 Chrome/38.0.2125.102 Safari/537.36",
		"Mozilla/5.0 (Linux; Android 5.1.1; Nexus 7 Build/LMY47V) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/43.0.2357.78 Safari/537.36 OPR/30.0.1856.93524",
		"Mozilla/5.0 (Linux; Android 5.1; C6740N Build/LMY47O) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/42.0.2311.111 Mobile Safari/537.36",
		"Mozilla/5.0 (Linux; Android 6.0.1; SM-G900H Build/MMB29K) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/52.0.2743.98 Mobile Safari/537.36",
		"Mozilla/5.0 (Linux; Android 6.0; HTC One M9 Build/MRA58K) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/52.0.2743.98 Mobile Safari/537.36",
		"Mozilla/5.0 (Linux; Android 6.0; LG-D850 Build/MRA58K) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/53.0.2785.97 Mobile Safari/537.36",
		"Mozilla/5.0 (Linux; Android 6.0; Nexus 5X Build/MDB08L) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/53.0.2785.124 Mobile Safari/537.36",
		"Mozilla/5.0 (Linux; Android 7.0; Nexus 9 Build/NRD90R) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/53.0.2785.124 Safari/537.36",
		"Mozilla/5.0 (MSIE 9.0; Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.79 Safari/537.36 Edge/14.14931",
		"Mozilla/5.0 (Macintosh; Intel Mac OS X 10.12; rv:49.0) Gecko/20100101 Firefox/49.0",
		"Mozilla/5.0 (Macintosh; Intel Mac OS X 10.6; rv:25.0) Gecko/20100101 Firefox/25.0",
		"Mozilla/5.0 (Macintosh; Intel Mac OS X 10.6; rv:9.0) Gecko/20100101 Firefox/9.0",
		"Mozilla/5.0 (Macintosh; Intel Mac OS X 10.7; rv:20.0) Gecko/20100101 Firefox/20.0",
		"Mozilla/5.0 (Macintosh; Intel Mac OS X 10.9; rv:35.0) Gecko/20100101 Firefox/35.0",
		"Mozilla/5.0 (Macintosh; Intel Mac OS X 10.9; rv:47.0) Gecko/20100101 Firefox/47.0",
		"Mozilla/5.0 (Macintosh; Intel Mac OS X 1083) AppleWebKit/537.36 (KHTML like Gecko) Chrome/28.0.1469.0 Safari/537.36",
		"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2859.0 Safari/537.36",
		"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_5) AppleWebKit/602.1.50 (KHTML, like Gecko) Version/10.0 Safari/602.1.50",
		"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_2) AppleWebKit/535.1 (KHTML, like Gecko) Chrome/14.0.835.186 Safari/535.1",
		"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_2; rv:10.0.1) Gecko/20100101 Firefox/10.0.1",
		"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_5) AppleWebKit/537.78.1 (KHTML like Gecko) Version/7.0.6 Safari/537.78.1",
		"Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/42.0.2311.135 Safari/537.36 Edge/12.10240",
		"Mozilla/5.0 (Windows NT 10.0; ARM; Lumia 950 Dual SIM) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.79 Safari/537.36 Edge/14.14393",
		"Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.103 Safari/537.36",
		"Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/53.0.2785.90 Safari/537.36 Vivaldi/1.4.589.11",
		"Mozilla/5.0 (Windows NT 10.0; WOW64; rv:47.0) Gecko/20100101 Firefox/47.0",
		"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.71 Safari/537.36 Edge/12.0",
		"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2869.0 Safari/537.36",
		"Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:52.0) Gecko/20100101 Firefox/52.0",
		"Mozilla/5.0 (Windows NT 6.0; WOW64; rv:40.0) Gecko/20100101 Firefox/40.0",
		"Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.155 Safari/537.36 OPR/31.0.1889.174",
		"Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:35.0) Gecko/20100101 Firefox/35.0",
		"Mozilla/5.0 (Windows NT 6.2; WOW64; rv:39.0) Gecko/20100101 Firefox/39.0",
		"Mozilla/5.0 (Windows NT 6.2; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/32.0.1667.0 Safari/537.36",
		"Mozilla/5.0 (Windows NT 6.3; rv:36.0) Gecko/20100101 Firefox/36.0",
		"Mozilla/5.0 (Windows Phone 10.0; Android 4.2.1; DEVICE INFO) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.71 Mobile Safari/537.36 Edge/12.0",
		"Mozilla/5.0 (X11; Linux i686; rv:43.0) Gecko/20100101 Firefox/43.0",
		"Mozilla/5.0 (X11; Linux i686; rv:46.0) Gecko/20100101 Firefox/46.0",
		"Mozilla/5.0 (X11; Linux i686; rv:49.0) Gecko/20100101 Firefox/49.0",
		"Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; WOW64; Trident/6.0)",
		"Mozilla/5.0 (compatible; MSIE 10.0; Windows Phone 8.0; Trident/6.0; IEMobile/10.0; ARM; Touch; NOKIA; Lumia 920)",
		"Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.2; WOW64; Trident/5.0)",
		"Mozilla/5.0 (webOS/1.3; U; en-US) AppleWebKit/525.27.1 (KHTML, like Gecko) Version/1.0 Safari/525.27.1 Desktop/1.0",
		"Opera/9.80 (Android 4.0.4; Linux; Opera Mobi/ADR-1205181138; U; pl) Presto/2.10.254 Version/12.00",
		"Opera/9.80 (S60; SymbOS; Opera Mobi/499; U; ru) Presto/2.4.18 Version/10.00",
		"Opera/9.80 (Windows NT 6.0) Presto/2.12.388 Version/12.14",
		"Opera/9.80 (Windows NT 6.1; WOW64) Presto/2.12.388 Version/12.16",
	};

	const unsigned index = (rand() * (_countof( userAgents ) - 1)) / RAND_MAX;
	return userAgents[index].c_str();
}