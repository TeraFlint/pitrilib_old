#pragma once
#include <iostream>

namespace Pitri
{
	/*-- Internal access proxy --*
	template<class T, unsigned N>
	class AccessProxy
	{
	private:
		T &value()
		{
			return reinterpret_cast<T *>(this)[N];
		}

	public:
		T &operator=(const T &other)
		{
			return value() = other;
		}

		operator T()
		{
			return value();
		}
	};*/

	/*-- Vector --*/
	template<class T, unsigned N>
	class Vec
	{
	protected:
		T data[N];

	public:
		Vec()
		{
			T *ptr = &data[0];
			for (unsigned i = 0; i < N; ++i)
			{
				*ptr++ = 0;
			}
		}
		Vec(std::initializer_list<T> par) : Vec()
		{
			T *ptr = &data[0], *end = &data[N];
			for (auto &entry : par)
			{
				if (ptr == end)
					break;

				*ptr++ = entry;
			}
		}
		template <class O>
		Vec(const Vec<O, N> &other) : Vec()
		{
			T *dst = &data[0];
			O *src = &other.data[0];
			for (unsigned i = 0; i < N; ++i)
			{
				*dst++ = static_cast<T>(*src++);
			}
		}

		operator bool() const
		{
			T *ptr = &data[0];
			for (unsigned i = 0; i < N; ++i)
			{
				if (*ptr++)
					return true;
			}
			return false;
		}

		/*Returns a reference of the nth element.
		- index: Index of the element.*/
		T &operator[](unsigned index)
		{
			return data[index];
		}
		/*Returns the value of the nth element.
		- index: Index of the element.*/
		T operator[](unsigned index) const
		{
			return data[index];
		}

		Vec<T, N> operator-() const
		{
			Vec<T, N> result;
			const T *src = &data[0];
			T *dst = &result.data[0];

			for (unsigned i = 0; i < N; ++i)
			{
				*dst++ = -(*src++);
			}
			return result;
		}

		friend Vec<T, N> operator+(const Vec<T, N> &left, const Vec<T, N> &right)
		{
			Pitri::Vec<T, N> result;
			const T *lft = &left.data[0], *rgt = &right.data[0];
			T *dst = &result.data[0];
			for (unsigned i = 0; i < N; ++i)
			{
				*dst++ = (*lft++) + (*rgt++);
			}
			return result;
		}
		friend Vec<T, N> operator-(const Vec<T, N> &left, const Vec<T, N> &right)
		{
			return left + (-right);
		}
		friend Vec<T, N> operator+=(const Vec<T, N> &left, const Vec<T, N> &right)
		{
			return left = left + right;
		}
		friend Vec<T, N> operator-=(const Vec<T, N> &left, const Vec<T, N> &right)
		{
			return left = left - right;
		}

		friend T operator *(const Vec<T, N> &left, const Vec<T, N> &right)
		{
			T result = 0;
			const T *lft = &left.data[0], *rgt = &right.data[0];
			for (unsigned i = 0; i < N; ++i)
			{
				result += (*lft++) * (*rgt++);
			}
			return result;
		}

		friend std::ostream &operator<<(std::ostream &out, const Pitri::Vec<T, N> &right)
		{
			out << "(";
			const T *ptr = &right.data[0];
			for (unsigned i = 0; i < N; ++i)
			{
				if (i)
					out << ", ";
				out << *ptr++;
			}
			out << ")";
			return out;
		}
		friend std::ostream &operator<<(std::ostream &out, Pitri::Vec<T, N> *right)
		{
			return out << *right;
		}

		unsigned Size() const
		{
			return N;
		}

		double Length() const
		{
			return sqrt(Square());
		}
		double Square() const
		{
			double result = 0;
			const T *ptr = &data[0];
			for (unsigned i = 0; i < N; ++i)
			{
				result += (*ptr) * (*ptr);
				ptr++;
			}
			return result;
		}
		bool SetLength(double to_length = 1)
		{
			double from_length = Length();
			if (!from_length)
				return false;

			T *ptr = &data[0];
			for (unsigned i = 0; i < N; ++i)
			{
				*ptr++ = (*ptr) * to_length / from_length;
			}
			return true;
		}

		T TypeLength() const
		{
			return sqrt(Square());
		}
		T TypeSquare() const
		{
			T result = 0;
			const T *ptr = &data[0];
			for (unsigned i = 0; i < N; ++i)
			{
				result += (*ptr) * (*ptr);
				ptr++;
			}
			return result;
		}
	};

	template<class T>
	class Vec2 : public Vec<T, 2>
	{
	public:
		using Vec<T, 2>::Vec;
		Vec2() = default;
		Vec2(T x, T y)
		{
			data[0] = x;
			data[1] = y;
		}

		void operator=(const Vec2<T> &other)
		{
			for (unsigned i = 0; i < 2; ++i)
			{
				data[i] = other.data[i];
			}
		}

		T &x = data[0];
		T &y = data[1];
		//AccessProxy<T, 0> x;
		//AccessProxy<T, 1> y;
	};

	template<class T>
	class Vec3 : public Vec<T, 3>
	{
	public:
		using Vec<T, 3>::Vec;
		Vec3(T x, T y, T z)
		{
			data[0] = x;
			data[1] = y;
			data[2] = z;
		}

		void operator=(const Vec3<T> &other)
		{
			for (unsigned i = 0; i < 3; ++i)
			{
				data[i] = other.data[i];
			}
		}

		T &x = data[0];
		T &y = data[1];
		T &z = data[2];

		//AccessProxy<T, 0> x;
		//AccessProxy<T, 1> y;
		//AccessProxy<T, 2> z;
	};

	/*-- Rect --*/
	template<class T>
	class Rect
	{
	public:
		Vec2<T> pos, size;

		Rect() = default;
		Rect(T x, T y, T wdt, T hgt) : pos(x, y), size(wdt, hgt)
		{
		}
		Rect(Vec2<T> pos, Vec2<T> size) : pos(pos), size(size)
		{
		}

		operator bool() const
		{
			return size.x && size.y;
		}

		/*The rectangle or operator.
		Combines two rectangles into the smallest rectangle that contains both smaller rectangles.*/
		friend Pitri::Rect<T> operator|(const Pitri::Rect<T> &left, const Pitri::Rect<T> &right)
		{
			T top = std::min(left.Top(), right.Top());
			T btm = std::max(left.Bottom(), right.Bottom());
			T lft = std::min(left.Left(), right.Left());
			T rgt = std::max(left.Right(), right.Right());
			return Pitri::Rect<T>(lft, top, rgt - lft, btm - top);
		}
		/*The rectangle and operator.
		Returns the common area of two rectangles. If there's no common area, a null rectangle gets returned.*/
		friend Pitri::Rect<T> operator&(const Pitri::Rect<T> &left, const Pitri::Rect<T> &right)
		{
			T top = std::max(left.Top(), right.Top());
			T btm = std::min(left.Bottom(), right.Bottom());
			T lft = std::max(left.Left(), right.Left());
			T rgt = std::min(left.Right(), right.Right());

			if (top >= btm || lft >= rgt)
				return Rect<T>(0, 0, 0, 0);
			return Rect<T>(lft, top, rgt - lft, btm - top);
		}
		/*The rectangle or operator.
		Combines two rectangles into the smallest rectangle that contains both smaller rectangles.
		Result gets saved in left.*/
		friend Pitri::Rect<T> operator|=(Pitri::Rect<T> &left, const Pitri::Rect<T> &right)
		{
			return left = left | right;
		}
		/*The rectangle and operator.
		Returns the common area of two rectangles. If there's no common area, a null rectangle gets returned.
		Results gets saved in left.*/
		friend Pitri::Rect<T> operator&=(Pitri::Rect<T> &left, const Pitri::Rect<T> &right)
		{
			return left = left | right;
		}

		T Top() const
		{
			if (size.y < 0)
				return pos.y + size.y;
			return pos.y;
		}
		T Bottom() const
		{
			if (size.y < 0)
				return pos.y;
			return pos.y + size.y;
		}
		T Left() const
		{
			if (size.x < 0)
				return pos.x + size.x;
			return pos.x;
		}
		T Right() const
		{
			if (size.x < 0)
				return pos.x;
			return pos.x + size.x;
		}
	};
}



