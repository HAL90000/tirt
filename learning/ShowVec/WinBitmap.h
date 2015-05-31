#pragma once
#include <Windows.h>
#include "Math.h"
#include "WindowDef.h"
#include <gdiplus.h>
#include <vector>
#include <iostream>

struct TColorBGR
{
	TColorBGR():r(0),g(0),b(0),a(0){}
	TColorBGR(int R,int G,int B):r(R),g(G),b(B),a(255){}
	TColorBGR(int R,int G,int B,int A):r(R),g(G),b(B),a(A){}
	BYTE b;
	BYTE g;
	BYTE r;
	BYTE a;
	
	void Set(BYTE r, BYTE g, BYTE b)
	{
		this->r = r;
		this->g = g;
		this->b = b;
	}
	void Set(BYTE r, BYTE g, BYTE b,BYTE a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	operator COLORREF()
	{
		return RGB(r,g,b);
		//return *((COLORREF*)this);
	}

	TColorBGR operator+(const TColorBGR &c) const
	{
		return TColorBGR(r + c.r, g + c.g, b + c.b, a + c.a);
	}

	TColorBGR operator*(BYTE c) const
	{
		return TColorBGR((int)r*c/255, (int)g*c/255, (int)b*c/255, (int)a*c/255);
	}

	double Val() const
	{
		return (((double)b*b)+((double)g*g)+((double)r*r))/195075.0;
	}

	static double inline sqr(double x)
	{
		return x*x;
	}

	double Dist(const TColorBGR &c) const
	{
		return sqr(r / 255. - c.r / 255.) + sqr(g / 255. - c.g / 255.) + sqr(b / 255. - c.b / 255.);
	}
};

class WinBitmap
{
	static const int BytePerPixel = 4;

	size_t m_rowSize;
	int m_width;
	int m_height;
	HDC m_hDC;
	void *m_data;
	HBITMAP m_bitmap;


public:
	
	WinBitmap() : m_width(0), m_height(0), m_hDC(NULL), m_bitmap(NULL), m_rowSize(0)
	{
	}

	WinBitmap(int w,int h,HDC hDC)
	{
		Alloc(w, h, hDC);
	}

	~WinBitmap()
	{
		Free();
	}

	HDC GetDC() const
	{
		return m_hDC;
	}

	HBITMAP GetBitmap() const
	{
		return m_bitmap;
	}

	size_t Width() const
	{
		return m_width;
	}
	size_t Height() const
	{
		return m_height;
	}

	void Free()
	{
		m_width = m_height = 0;
		m_rowSize = 0;
		m_data = 0;
		if (m_bitmap != NULL) {
			DeleteObject( m_bitmap );
			m_bitmap = NULL;
		}
		if (m_hDC != NULL) {
			DeleteDC( m_hDC );
			m_hDC = NULL;
		}
	}

	void Alloc(size_t w, size_t h, HDC hDC)
	{
		Free();
		if (w == 0 || h == 0 ) {
			return;
		}
		m_hDC = CreateCompatibleDC(hDC);

		if (m_hDC == NULL) throw std::runtime_error("could not create context");
		if (m_hDC == NULL) return;
		/*SetGraphicsMode( m_hDC, GM_ADVANCED );
		
		XFORM ctm = { 1.0, 0, 0, -1.0, 0.0, h };

		SetWorldTransform( m_hDC, &ctm );*/

		m_width = w;
		m_height = h;
		m_rowSize = (w * BytePerPixel + 3) & (~3u);

		BITMAPINFO bmi;
		bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
		bmi.bmiHeader.biBitCount = BytePerPixel*8;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biWidth = m_width;
		bmi.bmiHeader.biHeight = -((LONG)(m_height));
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = m_rowSize * m_height;
		bmi.bmiHeader.biClrImportant = 0;
		bmi.bmiHeader.biXPelsPerMeter = 0;
		bmi.bmiHeader.biYPelsPerMeter = 0;

		m_bitmap = CreateDIBSection(m_hDC, &bmi, DIB_RGB_COLORS, (void**)&m_data, NULL, 0);
		if (m_bitmap == NULL) throw std::runtime_error("could not create bitmap");
		if (m_data == NULL) throw std::runtime_error("could not create bitmap data");

		DeleteObject( SelectObject( m_hDC, m_bitmap) );
	}

	bool safePixel(int x,int y)
	{
		return x >= 0 && x < m_width && y >= 0 && y < m_height;
	}
	
	inline TColorBGR& Pixel(int x,int y)
	{
		return ((TColorBGR*)((BYTE*)m_data+y*m_rowSize))[x];
	}

	inline const TColorBGR& Pixel(int x,int y) const
	{
		return ((TColorBGR*)((BYTE*)m_data+y*m_rowSize))[x];
	}

	void Clear(TColorBGR color)
	{
		for (int y=0;y<m_height;++y)
			for (int x=0;x<m_width;++x)
			{
				Pixel(x,y) = color;
			}
	}

	void Invert()
	{
		for (int y=0;y<m_height;++y)
			for (int x=0;x<m_width;++x)
			{
				TColorBGR color = Pixel(x,y);
				color.b ^= 0xff;
				color.g ^= 0xff;
				color.r ^= 0xff;
				Pixel(x,y) = color;
			}
	}

	void DrawCircle(int x,int y, int r, int width, TColorBGR color)
	{
		
		HPEN pen = CreatePen( PS_SOLID, width, color );
		HPEN oldPen = (HPEN)SelectObject( m_hDC, pen );
		
		LOGBRUSH brushl;
		brushl.lbStyle = BS_NULL;
		HBRUSH brush = CreateBrushIndirect(&brushl);//CreateSolidBrush( color );
		HBRUSH oldBrush = (HBRUSH)SelectObject( m_hDC, brush );
		SetDCBrushColor( m_hDC, color );
		Ellipse( m_hDC, x+r, y+r, x-r, y-r );
		SelectObject( m_hDC, oldBrush );
		DeleteObject( brush );
		SelectObject( m_hDC, oldPen );
		DeleteObject( pen );
		
		/*
		for (int ix = x-r;ix<=x+r;++ix)
			for (int iy = y-r;iy<=y+r;++iy)
				if ((ix-x)*(ix-x)+(iy-y)*(iy-y) < r*r)
					if (safePixel(ix,iy))
						Pixel(ix,iy) = color;
		*/
	}

	void DrawCircle(int x, int y, int r, TColorBGR color)
	{

		HPEN pen = CreatePen(PS_NULL, 0, color);
		HPEN oldPen = (HPEN)SelectObject(m_hDC, pen);
		HBRUSH brush = CreateSolidBrush(color);
		HBRUSH oldBrush = (HBRUSH)SelectObject(m_hDC, brush);
		SetDCBrushColor(m_hDC, color);
		Ellipse(m_hDC, x + r, y + r, x - r, y - r);
		SelectObject(m_hDC, oldBrush);
		DeleteObject(brush);
		SelectObject(m_hDC, oldPen);
		DeleteObject(pen);

	}

	void DrawRect(int x, int y, int cx, int cy, TColorBGR color)
	{

		HPEN pen = CreatePen(PS_NULL, 0, color);
		HPEN oldPen = (HPEN)SelectObject(m_hDC, pen);
		HBRUSH brush = CreateSolidBrush(color);
		HBRUSH oldBrush = (HBRUSH)SelectObject(m_hDC, brush);
		SetDCBrushColor(m_hDC, color);
		Rectangle(m_hDC, x, y, x + cx, y + cy);
		SelectObject(m_hDC, oldBrush);
		DeleteObject(brush);
		SelectObject(m_hDC, oldPen);
		DeleteObject(pen);

	}
	
	void DrawLine( int x1, int y1, int x2, int y2, int width, TColorBGR color)
	{
		
		POINT p;
		MoveToEx( m_hDC, x1, y1, &p );

		HPEN pen = CreatePen(PS_SOLID, width, color);
		HPEN oldPen = (HPEN)SelectObject( m_hDC, pen );

		LineTo( m_hDC, x2, y2 );

		SelectObject( m_hDC, oldPen );
		DeleteObject( pen );

	}

	void DrawLines( POINT *pt, int n, TColorBGR color)
	{

		HPEN pen = CreatePen(PS_SOLID, 1, color);
		HPEN oldPen = (HPEN)SelectObject( m_hDC, pen );

		Polyline( m_hDC, pt, n );

		SelectObject( m_hDC, oldPen );
		DeleteObject( pen );

	}

	TColorBGR& operator()(int x,int y)
	{
		return Pixel(x,y);
	}

	void* Data()
	{
		return m_data;
	}

	size_t bSize()
	{
		return m_height * m_rowSize;
	}

	void Load(const tstring &filename, HDC hDC)
	{
		// Start Gdiplus 
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

		// Load the image 
		Gdiplus::Bitmap* image = Gdiplus::Bitmap::FromFile(filename.c_str());
		if (image == nullptr)
			throw std::runtime_error("file not found");

		Alloc(image->GetWidth(), image->GetHeight(), hDC);
		
		Gdiplus::Rect rect(0, 0, image->GetWidth(), image->GetHeight());

		Gdiplus::BitmapData* bitmapData = new Gdiplus::BitmapData;

		image->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, bitmapData);

		if (bitmapData->Scan0 == nullptr)
			std::cerr << "ERROR Scan0 NULL" << std::endl;
		if (bitmapData->Stride != m_rowSize)
			std::cerr << "stride = " << bitmapData->Stride << "; m_rowSize = " << m_rowSize << std::endl;

		memcpy(m_data, bitmapData->Scan0, bSize());
		
		image->UnlockBits(bitmapData);

		delete bitmapData;
		
		// delete the image when done 
		delete image; image = 0;

		// Shutdown Gdiplus 
		Gdiplus::GdiplusShutdown(gdiplusToken);
	}

	int GetEncoderClsid(const TCHAR* format, CLSID* pClsid)
	{
		using namespace Gdiplus;
		UINT num = 0; // number of image encoders
		UINT size = 0; // size of the image encoder array in bytes
		ImageCodecInfo* pImageCodecInfo = NULL;
		GetImageEncodersSize(&num, &size);
		if (size == 0)
			return -1; // Failure
		pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
		if (pImageCodecInfo == NULL)
			return -1; // Failure
		GetImageEncoders(num, size, pImageCodecInfo);
		for (UINT j = 0; j < num; ++j)
		{
			if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
			{
				*pClsid = pImageCodecInfo[j].Clsid;
				free(pImageCodecInfo);
				return j; // Success
			}
		}
		free(pImageCodecInfo);
		return 0;
	}

	void Save(const tstring &filename, const tstring &imgType)
	{
		// Start Gdiplus 
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

		// Create the image 

		BITMAPINFO bmi;
		bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
		bmi.bmiHeader.biBitCount = BytePerPixel * 8;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biWidth = m_width;
		bmi.bmiHeader.biHeight = -((LONG)(m_height));
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = m_rowSize * m_height;
		bmi.bmiHeader.biClrImportant = 0;
		bmi.bmiHeader.biXPelsPerMeter = 0;
		bmi.bmiHeader.biYPelsPerMeter = 0;

		Gdiplus::Bitmap* image = Gdiplus::Bitmap::FromBITMAPINFO(&bmi, Data()); //Gdiplus::Bitmap::FromFile(filename.c_str());

		CLSID pngClsid;
		tstringstream type;
		type << TEXT("image/") << imgType;
		GetEncoderClsid(type.str().c_str(), &pngClsid);

		image->Save(filename.c_str(), &pngClsid, NULL);

		// delete the image when done 
		delete image; image = 0;

		// Shutdown Gdiplus 
		Gdiplus::GdiplusShutdown(gdiplusToken);
	}

};

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif