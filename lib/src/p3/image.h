#pragma once
#include <string>
#include <vector>
#include <iostream> //for the operator

namespace Pitri
{
	struct Color
	{
		Color() = default;
		Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);

		bool operator == (const Color &other) const;
		bool operator != (const Color &other) const;
		unsigned char &operator[](const unsigned index);

		unsigned char r, g, b, a;
	};
}

/*Color overlay operator. Right over left.*/
Pitri::Color operator<<(Pitri::Color left, Pitri::Color right);
/*Color overlay operator. Left over right.*/
Pitri::Color operator>>(Pitri::Color left, Pitri::Color right);
/*Color overlay operator. Right over left, result gets saved in left.*/
Pitri::Color operator<<=(Pitri::Color &left, Pitri::Color right);
/*Color overlay operator. Left over right, result gets saved in left.*/
Pitri::Color operator>>=(Pitri::Color &left, Pitri::Color right);

std::ostream &operator<<(std::ostream &out, Pitri::Color clr);

namespace Pitri
{
	class Image
	{
	private:
		unsigned width, height;
		unsigned error;
		std::vector<Color> bitmap;

	public:
		Image() = default;
		Image(unsigned width, unsigned height);
		~Image() = default;

		std::vector<Color>::iterator begin();
		std::vector<Color>::iterator end();

		bool Save(const std::string &filename);
		bool Load(const std::string &filename);

		/*Width() returns the width of the image.*/
		unsigned Width() const;
		/*Height() returns the height of the image.*/
		unsigned Height() const;
		/*Pixel() returns a reference to a pixel in the bitmap.
		- x: X coordinate.
		- y: Y coordinate.*/
		Color &Pixel(unsigned x, unsigned y);
		const Color &Pixel(unsigned x, unsigned y) const;

		/*GetError() returns a non-zero value, if Save() or Load() failed.*/
		unsigned GetError();

		/*Inside() checks, if the coordinates are inside the image. If true, this pixel can be written.
		- x: X coordinate.
		- y: Y coordinate.*/
		bool Inside(const unsigned x, const unsigned y) const;

		/*InterpolatePixelColor() gives bilinear interpolation for floaty coordinates. Returns an empty color, if out of borders.
		- x: Precise x coordinate.
		- y: Precose y coordinate.
		- relative: If true, coordinates go from 0 to 1.*/
		Color InterpolatePixelColor(float x, float y, const bool relative = true) const;
	};
}
