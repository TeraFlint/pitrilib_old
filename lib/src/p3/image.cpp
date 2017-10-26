#include "image.h"

#include <wincodec.h>
#include <wincodecsdk.h>
#include <Shlwapi.h>
#include <locale>
#include <codecvt>

#include "compointer.h"

#pragma comment(lib,"Shlwapi.lib")

namespace Pitri
{
	Color::Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a) : r(r), g(g), b(b), a(a)
	{
	}

	bool Color::operator == (const Color &other) const
	{
		return r == other.r && g == other.g && b == other.b && a == other.a;
	}

	bool Color::operator != (const Color &other) const
	{
		return !(*this == other);
	}
	unsigned char &Color::operator[](const unsigned index)
	{
		if (index == 0) return r;
		if (index == 1) return g;
		if (index == 2) return b;
		return a;
	}
}

/* Overlay operator. Creates a new color by laying right on left. */
Pitri::Color operator<<(Pitri::Color left, Pitri::Color right)
{
	if (right.a == 255) return right;

	Pitri::Color result = left;
	if (left.a || right.a)
	{
		float sa = static_cast<float>(right.a) / 255, da = static_cast<float>(left.a) / 255;
		for (unsigned char *dst = &result.r, *src = &right.r ; dst < &result.a; ++dst, ++src)
		{
			*dst = (*src * sa + *dst * da * (1-sa)) / (sa + da * (1-sa));
		}
		result.a = 255 * (1- (1 - da)*(1 - sa));
	}
	return result;
}
/* Overlay operator. Creates a new color by laying left on right. */
Pitri::Color operator>>(Pitri::Color left, Pitri::Color right)
{
	return right << left;
}
/* Overlay operator. Lays right on left and saves result in left. */
Pitri::Color operator<<=(Pitri::Color &left, Pitri::Color right)
{
	return left = left << right;
}
/* Overlay operator. Lays left on right and saves result in left. */
Pitri::Color operator>>=(Pitri::Color &left, Pitri::Color right)
{
	return left = left >> right;
}
std::ostream &operator<<(std::ostream &out, Pitri::Color clr)
{
	int r = clr.r, g = clr.g, b = clr.b, a = clr.a;
	if (!a) return out << "RGB(" << r << ", " << g << ", " << b << ")";
	return out << "RGBa(" << r << ", " << g << ", " << b << ", " << a << ")";
}

namespace Pitri
{
	Image::Image(unsigned width, unsigned height) : width(width), height(height)
	{
		error = 0;
		bitmap.resize(width*height);
	}

	std::vector<Color>::iterator Image::begin()
	{
		return bitmap.begin();
	}
	std::vector<Color>::iterator Image::end()
	{
		return bitmap.end();
	}

	bool Image::Save(const std::string &filename)
	{
		ComPtr<IWICImagingFactory> factory;
		ComPtr<IWICBitmapEncoder> encoder;
		ComPtr<IStream> stream;
		ComPtr<IWICBitmapFrameEncode> frame;
		ComPtr<IWICBitmap> tempbitmap;

		std::wstring dest;
		for (auto c : filename)
			dest += wchar_t(c);

#if defined(CLSID_WICImagingFactory)
		if (S_OK != CoCreateInstance(CLSID_WICImagingFactory, 0, CLSCTX_INPROC_SERVER, __uuidof(factory), reinterpret_cast<void **>(&factory)))
		{
			if (S_OK != CoCreateInstance(CLSID_WICImagingFactory1, 0, CLSCTX_INPROC_SERVER, __uuidof(factory), reinterpret_cast<void **>(&factory)))
			{
				error = 200;
				return false;
			}
		}
#else
		if (S_OK != CoCreateInstance(CLSID_WICImagingFactory, 0, CLSCTX_INPROC_SERVER, __uuidof(factory), reinterpret_cast<void **>(&factory)))
		{
			error = 200;
			return false;
		}
#endif
		if (S_OK != factory->CreateEncoder(GUID_ContainerFormatPng, 0, &encoder))
		{
			error = 201;
			return false;
		}
		if (S_OK != SHCreateStreamOnFileEx(dest.c_str(), STGM_CREATE | STGM_WRITE, FILE_ATTRIBUTE_NORMAL, 0, 0, &stream))
		{
			error = 202;
			return false;
		}
		if (S_OK != encoder->Initialize(stream, WICBitmapEncoderNoCache))
		{
			error = 203;
			return false;
		}
		if (S_OK != encoder->CreateNewFrame(&frame, 0))
		{
			error = 204;
			return false;
		}
		if (S_OK != frame->Initialize(0))
		{
			error = 205;
			return false;
		}
		if (S_OK != frame->SetSize(width, height))
		{
			error = 206;
			return false;
		}
		WICPixelFormatGUID format = GUID_WICPixelFormat32bppRGBA;
		if (S_OK != frame->SetPixelFormat(&format))
		{
			error = 207;
			return false;
		}
		unsigned int depth = sizeof(bitmap[0]);
		if (S_OK != factory->CreateBitmapFromMemory(width, height, GUID_WICPixelFormat32bppRGBA, width * depth, width * height * depth, const_cast<BYTE *>(reinterpret_cast<const BYTE *>(&bitmap[0])), &tempbitmap))
		{
			error = 208;
			return false;
		}
		if (S_OK != frame->WriteSource(tempbitmap, 0))
		{
			error = 209;
			return false;
		}
		if (S_OK != frame->Commit())
		{
			error = 210;
			return false;
		}
		if (S_OK != encoder->Commit())
		{
			error = 211;
			return false;
		}
		if (S_OK != stream->Commit(STGC_DEFAULT))
		{
			error = 212;
			return false;
		}
		return true;
	}

	bool Image::Load(const std::string &filename)
	{
		ComPtr<IWICImagingFactory> factory;
		ComPtr<IWICBitmapDecoder> decoder;
		ComPtr<IWICBitmapFrameDecode> frame;
		ComPtr<IWICFormatConverter> converter;

		std::wstring dest;
		for (auto c : filename)
			dest += wchar_t(c);

		if (S_OK != CoCreateInstance(CLSID_WICImagingFactory, 0, CLSCTX_INPROC_SERVER, __uuidof(factory), reinterpret_cast<void **>(&factory)))
		{
			error = 100;
			return false;
		}
		if (S_OK != factory->CreateDecoderFromFilename(dest.c_str(), 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder))
		{
			error = 101;
			return false;
		}
		if (S_OK != decoder->GetFrame(0, &frame))
		{
			error = 102;
			return false;
		}
		if (S_OK != factory->CreateFormatConverter(&converter))
		{
			error = 103;
			return false;
		}
		if (S_OK != converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, 0, 0.0, WICBitmapPaletteTypeCustom))
		{
			error = 104;
			return false;
		}
		if (S_OK != converter->GetSize(&width, &height))
		{
			error = 105;
			return false;
		}
		bitmap.resize(width*height);
		unsigned depth = sizeof(bitmap[0]);
		if (S_OK != converter->CopyPixels(0, width*depth, bitmap.size()*depth, reinterpret_cast<BYTE *>(&bitmap[0])))
		{
			error = 106;
			return false;
		}
		return true;
	}

	unsigned Image::Width() const
	{
		return width;
	}
	unsigned Image::Height() const
	{
		return height;
	}
	Color &Image::Pixel(unsigned x, unsigned y)
	{
		return bitmap[y*width + x];
	}
	const Color &Image::Pixel(unsigned x, unsigned y) const
	{
		return bitmap[y*width + x];
	}

	unsigned Image::GetError()
	{
		return error;
	}

	bool Image::Inside(const unsigned x, const unsigned y) const
	{
		return x < width && y < height;
	}

	std::vector<unsigned> Image::GetContentArea() const
	{
		unsigned x = 0, y = 0, w = 0, h = 0;
		if (!GetContentArea(x, y, w, h))
			return {};
		return { x, y, w, h };
	}

	bool Image::GetContentArea(unsigned &xpos, unsigned &ypos, unsigned &wdt, unsigned &hgt) const
	{
		const Color *px = &bitmap[0];
		unsigned x1 = -1, x2 = -1, y1 = -1, y2 = -1;
		for (unsigned y = 0; y < height; ++y)
		{
			for (unsigned x = 0; x < width; ++x)
			{
				if (!(px++->a))
					continue;

				if (y1 == -1) y1 = y;
				y2 = y;
				if (x1 == -1 || x < x1) x1 = x;
				if (x2 == -1 || x > x2) x2 = x;
			}
		}

		if (x1 == -1 || x2 == -1 || y1 == -1 || y2 == -1)
			return false;

		xpos = x1;
		ypos = y1;
		wdt = x2 - x1 + 1;
		hgt = y2 - y1 + 1;
		return true;
	}

	Color Image::InterpolatePixelColor(float x, float y, const bool relative) const
	{
		Color empty(0, 0, 0, 0);
		if (!width || !height)
			return empty;
		if (relative)
		{
			x *= (width-1);
			y *= (height-1);
		}

		int xgrid = static_cast<int>(x + 2) - 2, ygrid = static_cast<int>(y + 2) - 2;
		float xoff = x - xgrid, yoff = y - ygrid;

		int x1 = xgrid, x2 = xgrid + 1;
		int y1 = ygrid, y2 = ygrid + 1;
		if (xgrid == x) x2 = xgrid;
		if (ygrid == y) y2 = ygrid;

		Color clr1 = empty, clr2 = empty, clr3 = empty, clr4 = empty;
		if (Inside(x1, y1)) clr1 = Pixel(x1, y1);
		if (Inside(x2, y1)) clr2 = Pixel(x2, y1);
		if (Inside(x1, y2)) clr3 = Pixel(x1, y2);
		if (Inside(x2, y2)) clr4 = Pixel(x2, y2);


		Color top = ColorTransition(clr1, clr2, xoff);
		Color btm = ColorTransition(clr3, clr4, xoff);;
		return ColorTransition(top, btm, yoff);

		//Without alpha correction:
		//*dst = (1 - yoff) * ((1 - xoff) * (*c1) + (xoff)* (*c2)) + (yoff)* ((1 - xoff) * (*c3) + (xoff)* (*c4));
	}


	/* Color functions */

	bool ChangeColorLighting(Color &clr, const unsigned char light, const bool brighten)
	{
		if (brighten && light == 0 || !brighten && light == 255)
			return false;

		unsigned char *c = &clr.r;
		for (unsigned i = 0; i < 3; ++i)
		{
			if (!brighten)
				*c++ = *c * light / 255;
			else
			{
				*c++ = 255 - (255 - light) * (255 - *c) / 255;
			}
		}
		return true;
	}
	bool ChangeColorLighting(Color &clr, float light)
	{
		if (!light) return false;
		if (light < -1) light = -1;
		else if (light > 1) light = 1;

		unsigned char *c = &clr.r;
		for (unsigned i = 0; i < 3; ++i)
		{
			if (light < 0) *c++ *= light + 1;
			else *c++ = 255 - (255 * (1 - light)) * (255 - *c) / 255;
		}
		return true;
	}

	Color ColorTransition(Color a, Color b, const unsigned char progress)
	{
		Color c(0, 0, 0, 0);
		if (a.a || a.b)
		{
			if (!a.a) a = Color(b.r, b.g, b.b, 0);
			if (!b.a) b = Color(a.r, a.g, a.b, 0);

			unsigned char *pa = &a.r, *pb = &b.r, *pc = &c.r;
			for (unsigned i = 0; i < 4; ++i)
			{
				*pc++ = static_cast<int>(255 - progress) * (*pa++) / 255 + static_cast<int>(progress)* (*pb++) / 255;
			}
		}
		return c;
	}
	Color ColorTransition(Color a, Color b, float progress)
	{
		Color c(0, 0, 0, 0);
		if (a.a || a.b)
		{
			if (!a.a) a = Color(b.r, b.g, b.b, 0);
			if (!b.a) b = Color(a.r, a.g, a.b, 0);

			if (progress < 0) progress = 0;
			else if (progress > 1) progress = 1;

			unsigned char *pa = &a.r, *pb = &b.r, *pc = &c.r;
			for (unsigned i = 0; i < 4; ++i)
			{
				*pc++ = (1 - progress) * (*pa++) + (progress)* (*pb++);
			}
		}
		return c;
	}


	/*Initialize helper for the WIC library.
	Don't mind it, it does its own thing automatically.*/
	struct CoInitializeHelper
	{
		CoInitializeHelper()
		{
			CoInitialize(0);
		}

		~CoInitializeHelper()
		{
			CoUninitialize();
		}

	} coinitializehelper;
}