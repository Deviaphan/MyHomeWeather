#pragma once

#include "curl/curl.h"
#include <vector>

enum class ProxyType: int
{
	NoProxy = 0,
	UseTor,
	UseProxy,
};

class CurlList
{
public:
	CurlList()
		: header( nullptr )
	{
	}

	~CurlList()
	{
		if( header )
		{
			curl_slist_free_all( header );
			header = nullptr;
		}
	}

public:
	curl_slist * header;
};

class Curl
{
public:
	explicit Curl( unsigned timeout, bool allowProxy = true, const char * userAgent = nullptr );

	~Curl();

	operator CURL *()
	{
		return _curlHandle;
	}

	void Destroy();

	void InitHeaders();
	void InitProxy( ProxyType proxyType = ProxyType::NoProxy, const char * proxy = nullptr );

	CurlList & GetCurlList()
	{
		return _curlList;
	}

	static void GlobalInit();
	static void GlobalCleanup();

	class Global
	{
	public:
		Global()
		{
			Curl::GlobalInit();
		}

		~Global()
		{
			Curl::GlobalCleanup();
		}
	};


private:
	CURL * _curlHandle;
	CurlList _curlList;
	bool _allowProxy;
};


class CurlBuffer
{
public:
	CurlBuffer()
	{
		Reset();
	}

public:
	static size_t WriteMemoryCallback( void *contents, size_t size, size_t count, void *userp );

public:
	const void * GetData() const
	{
		if( !_buffer.empty() )
			return std::data(_buffer);

		return nullptr;
	}

	size_t GetSize() const
	{
		return _buffer.size();
	}

	void TerminateBuffer()
	{
		_buffer.push_back( '\0' );
	}

	void Reset()
	{
		_buffer.clear();
		_buffer.reserve( 512 * 1024 );
	}

private:
	void WriteBytes( const void * data, size_t numBytes )
	{
		const size_t startPos = _buffer.size();
		_buffer.resize( startPos + numBytes );
		memcpy( &_buffer[startPos], data, numBytes );
	}

private:
	std::vector<unsigned char> _buffer;	///< Указатель на буфер
};

inline void VerifyCurl( CURLcode curlResult )
{
	if( curlResult != CURLE_OK )
		throw std::exception( "CURL error. Sad." );
}
