#ifndef _VEC2_
#define _VEC2_

// Defines a generic 2d vector
template< class T >
struct Vector2d
{
	Vector2d() : x(0), y(0)
	{
	}

	Vector2d(T xValue, T yValue) : x(xValue), y(yValue)
	{
	}

	T x;
	T y;
};

template< class T >
bool operator==(const Vector2d<T>& a, const Vector2d<T>& b)
{
	return ((a.x == b.x) && (a.y == b.y));
}

template< class T >
bool operator!=(const Vector2d<T>& a, const Vector2d<T>& b)
{
	return !::operator==(a, b);
}

typedef Vector2d<int> ivec2;
typedef Vector2d<float> vec2;

#endif //_VEC2_
