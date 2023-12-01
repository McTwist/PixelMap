#include "render/utility.hpp"

#include "minecraft.hpp"

#include <math.h>

namespace utility
{
namespace color
{

RGBA blend(const RGBA &a, const RGBA &b, int h)
{
	if (b.a == 0)
		return a;
	if (a.a == 0)
		return b;
	constexpr float ac = 1 / 255.f;
	RGBA c(0);
	float aa = a.a * ac;
	float ab = b.a * ac;
	float pa = ab * (1 - aa);
	float alpha = aa + pa;
	if (alpha > 0)
	{
		float ph = h / 128.f;
		c.r = glm::u8((a.r * aa + (b.r * ph) * pa) / alpha);
		c.g = glm::u8((a.g * aa + (b.g * ph) * pa) / alpha);
		c.b = glm::u8((a.b * aa + (b.b * ph) * pa) / alpha);
		c.a = glm::u8(alpha * 255);
	}
	if (b.a == 255)
		c.a = 255;
	return c;
};

RGBA interpolate(const RGBA &a, const RGBA &b, float n)
{
	n = glm::clamp(n, 0.f, 1.f);
	return RGBA(
		glm::u8((1.f-n)*a.r + n*b.r),
		glm::u8((1.f-n)*a.g + n*b.g),
		glm::u8((1.f-n)*a.b + n*b.b),
		a.a);
}

} // namespace color

namespace space
{

/*static inline Vector::value_type mod(Vector::value_type value, Vector::value_type modulus)
{
	return ::fmodf(::fmodf(value, modulus) + modulus, modulus);
}*/
/*static inline Vector::value_type intbound(Vector::value_type s, Direction::value_type d)
{
	s *= glm::sign(d);
	d *= glm::sign(d);
	s = mod(s, 1);
	return (1-s)/d;
}*/

inline float intbound(float s, float ds)
{
	if (ds < 0)
	{
		return intbound(-s, -ds);
	}
	else
	{
		s = std::fmod(s, 1.f);
		return (1 - s) / ds;
	}
}
inline Vector intbound(const Vector & pos, const Direction & dir)
{
	return {
		intbound(pos.x, dir.x),
		intbound(pos.y, dir.y),
		intbound(pos.z, dir.z)
	};
}

/**
 * @brief Initialize fast voxel ray tracing
 * Reference: AMANATIDES, John, et al. A fast voxel traversal algorithm for ray tracing. In: Eurographics. 1987. p. 3-10.
 * https://citeseerx.ist.psu.edu/document?repid=rep1&type=pdf&doi=7620a26cf2ffc6a4d634c7cde816d2f716904d26
 * http://www.cse.yorku.ca/~amana/research/grid.pdf
 * Useful: https://gamedev.stackexchange.com/a/49423
 * @param _pos 
 * @param dir 
 */
RayTracing::RayTracing(Vector _pos, Direction dir)
{
	if (glm::distance(Direction(), dir) < glm::epsilon<Direction::value_type>())
		return;
	pos = glm::floor(_pos);
	step = glm::sign(dir);
	tMax = intbound(_pos, dir);
	/*
	 * Note: 0/0 = NaN, and it so happens that if the step is
	 * 0, tMax will never move in that direction.
	 */
	tDelta = step / dir;
}
Vector RayTracing::next()
{
	if (tMax.x < tMax.y && tMax.x < tMax.z)
	{
		pos.x += step.x;
		tMax.x += tDelta.x;
	}
	else if (tMax.y < tMax.z)
	{
		pos.y += step.y;
		tMax.y += tDelta.y;
	}
	else
	{
		pos.z += step.z;
		tMax.z += tDelta.z;
	}
	return pos;
	// TODO: Investigate why this was written
	/*static const auto inf = std::numeric_limits<Direction::value_type>::infinity();
	static const Direction::value_type epsilon = 0.00000001f;
	auto ipos = convert(pos);
	auto step = glm::i8vec3(
		glm::sign(dir.x),
		glm::sign(dir.y),
		glm::sign(dir.z));
	//auto delta = 1 / dir;
	auto dist = glm::vec3(
		step.x > 0 ? ipos.x + 1 - pos.x : pos.x - ipos.x,
		step.y > 0 ? ipos.y + 1 - pos.y : pos.y - ipos.y,
		step.z > 0 ? ipos.z + 1 - pos.z : pos.z - ipos.z);
	auto tMax = glm::vec3(
		glm::abs(dir.x) < epsilon ? inf : 1 / dir.x,
		glm::abs(dir.y) < epsilon ? inf : 1 / dir.y,
		glm::abs(dir.z) < epsilon ? inf : 1 / dir.z);*/
}
TilePosition to(Vector pos)
{
	return TilePosition(pos);
}

PlanePosition toPlane(Vector pos)
{
	return PlanePosition(pos.x, pos.z);
}

PlanePosition::value_type proj(Vector::value_type p,
	Vector::value_type a1, Vector::value_type b1,
	PlanePosition::value_type a2, PlanePosition::value_type b2)
{
	return ((p - a1) / (b1 - a1)) * (b2 - a2) + a2;
}

} // namespace color

namespace coord
{

template<typename T>
constexpr bool isPowerOfTwo(T v) noexcept
{
	return v != 0 && (v & (v - 1)) == 0;
}

constexpr unsigned int toShiftValue(unsigned int v) noexcept
{
	assert(isPowerOfTwo(v));
	unsigned int shift = 0;
	for (unsigned int mask = 1; shift < 32 && ~mask & v; mask <<= 1, ++shift);
	return shift;
}

// Premature optimization
template<typename T, int v>
T linearShift(T a) noexcept
{
	static_assert(std::is_integral<T>::value);
	static_assert(std::is_convertible<int, T>::value);
	static_assert(isPowerOfTwo(v));
	static_assert(toShiftValue(v) > 0);
	return a >> toShiftValue(v);
}

template<typename T, typename U>
T linearDiv(T a, U d)
{
	static_assert(std::is_integral<T>::value);
	static_assert(std::is_integral<U>::value);
	static_assert(std::is_convertible<U, T>::value);
	// Note: There is a better way (platform independent; require minimum value):
	// Note2: minimum value could be calculated
	// (a - aMin) / d + aMin / d
	// Note: An even better way (platform specific; premature optimization):
	// a >> toShift(d)
	return (a < 0 ? a - (T(d) - 1) : a) / T(d);
}

template<typename T, typename U>
T positiveMod(T a, U d)
{
	static_assert(std::is_integral<T>::value);
	static_assert(std::is_integral<U>::value);
	static_assert(std::is_convertible<U, T>::value);
	return ((a % T(d)) + T(d)) % T(d);
}

RegionPosition toRegion(const ChunkPosition & cpos)
{
	return {
		linearDiv(cpos.x, Minecraft::regionWidth()),
		linearDiv(cpos.y, Minecraft::regionWidth())
	};
}

ChunkPosition toChunk(const SectionPosition & spos)
{
	return {spos.x, spos.z};
}

SectionPosition toSection(const BlockPosition & bpos)
{
	return {
		toSection(bpos.x),
		toSection(bpos.y),
		toSection(bpos.z)
	};
}

BlockPosition modSection(const BlockPosition & bpos)
{
	return {
		positiveMod(bpos.x, Minecraft::chunkWidth()),
		positiveMod(bpos.y, Minecraft::chunkWidth()),
		positiveMod(bpos.z, Minecraft::chunkWidth())
	};
}

int32_t toSection(int32_t b)
{
	return linearDiv(b, Minecraft::chunkWidth());
}

} // namespace coord

namespace math
{

std::size_t mod(int32_t v, std::size_t m)
{
	assert(m != 0 && !(m & (m - 1)));
	return std::size_t(((std::size_t(v) % m) + m) % m);
}

std::size_t indexMod2d(std::size_t width, int32_t x, int32_t z)
{
	return width * mod(z, width) + mod(x, width);
}

std::size_t index2d(std::size_t width, int32_t x, int32_t z)
{
	return width * std::size_t(z) + std::size_t(x);
}

} // namespace math
} // namespace utility
