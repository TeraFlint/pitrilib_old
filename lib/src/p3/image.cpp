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

	unsigned Image::GetError()
	{
		return error;
	}

	bool Image::Inside(const unsigned x, const unsigned y) const
	{
		//No need to check, if x or y are positive. They're unsigned.
		//So, if someone plugs negative values in, they will underflow and be way bigger than the image dimensions.
		return x < width && y < height;
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