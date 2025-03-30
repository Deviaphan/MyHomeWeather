#pragma once

class CImageSmartDc
{
public:
	CImageSmartDc()
		: _image( nullptr )
		, _hdc( nullptr )
	{}

	explicit CImageSmartDc( CImage & image )
		: _image( &image )
		, _hdc( _image->GetDC() )
	{
	}

	~CImageSmartDc()
	{
		if( _hdc )
			_image->ReleaseDC();
	}

	void Attach( CImage & image )
	{
		_image = &image;
		_hdc = _image->GetDC();
	}

private:
	CImageSmartDc( const CImageSmartDc & ) = delete;
	CImageSmartDc & operator = ( const CImageSmartDc & ) = delete;

public:

	operator HDC()
	{
		return _hdc;
	}


private:
	CImage * _image;
	HDC _hdc;
};
