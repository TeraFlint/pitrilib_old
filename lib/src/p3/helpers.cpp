#include "helpers.h"

/*-- Vec 2 --* /
namespace Pitri
{
	template<class T>
	Vec2<T>::Vec2(T x, T y) : x(x), y(y)
	{
	}
}

template<class T>
Pitri::Vec2<T> operator+(const Pitri::Vec2<T> &left, const Pitri::Vec2<T> &right)
{
	return Vec2(left.x + right.x, left.y + right.y);
}


/*-- Vec3 --* /
namespace Pitri
{
	template<class T>
	Vec3<T>::Vec3(T x, T y, T z) : x(x), y(y), z(z)
	{
	}
}

/*-- Rect --* /
namespace Pitri
{
	template<class T>
	Rect<T>::Rect(T x, T y, T wdt, T hgt) : Rect<T>(Vec2<T>(x, y), Vec2<T>(wdt, hgt))
	{
	}

	template<class T>
	Rect<T>::Rect(Vec2<T> pos, Vec2<T> size) : pos(pos), size(size)
	{
	}
}

*/