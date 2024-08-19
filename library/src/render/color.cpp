#include "render/color.hpp"

#include <algorithm>

template<typename T>
constexpr T vmax(T a, T b)
{
	return (std::max)(a, b);
}
template<typename T, typename... V>
constexpr T vmax(T a, T b, V... v)
{
	return vmax(vmax(a, b), v...);
}
template<typename T>
constexpr T vmin(T a, T b)
{
	return (std::min)(a, b);
}
template<typename T, typename... V>
constexpr T vmin(T a, T b, V... v)
{
	return vmin(vmin(a, b), v...);
}

namespace utility
{
namespace color
{

template<typename T>
std::enable_if_t<std::is_floating_point_v<T>, T> clampColor(std::enable_if_t<std::is_floating_point_v<T>, T> a)
{
	return glm::clamp<T>(a, T(0), T(1));
}
template<typename T>
std::enable_if_t<std::is_integral_v<T>, T> clampColor(std::enable_if_t<std::is_integral_v<T>, T> a)
{
	return glm::clamp<T>(a, T(0), T(255));
}
glm::vec4 clampColor(glm::vec4 a)
{
	return glm::clamp(a, glm::vec4::value_type(0), glm::vec4::value_type(1));
}
RGBA clampColor(RGBA a)
{
	return glm::clamp(a, RGBA::value_type(0), RGBA::value_type(255));
}

template<typename I>
float toFloat(std::enable_if_t<std::is_integral_v<I>, I> a)
{
	return a / 255.f;
}
glm::vec4 toFloat(RGBA a)
{
	return glm::vec4(
		toFloat<RGBA::value_type>(a.r),
		toFloat<RGBA::value_type>(a.g),
		toFloat<RGBA::value_type>(a.b),
		toFloat<RGBA::value_type>(a.a)
	);
}
template<typename I>
std::enable_if_t<std::is_integral_v<I>, I> toInt(float a)
{
	return glm::roundEven(a * 255.f);
}
RGBA toInt(glm::vec4 a)
{
	return RGBA(
		toInt<RGBA::value_type>(a.r),
		toInt<RGBA::value_type>(a.g),
		toInt<RGBA::value_type>(a.b),
		toInt<RGBA::value_type>(a.a)
	);
}

namespace blend
{

// https://www.w3.org/TR/compositing-1/#blendingseparable

template<typename C> C normal(C b, C s);
template<typename C> C multiply(C b, C s);
template<typename C> C screen(C b, C s);
template<typename C> C overlay(C b, C s);
template<typename C> C darken(C b, C s);
template<typename C> C lighten(C b, C s);
template<typename C> C color_dodge(C b, C s);
template<typename C> C color_burn(C b, C s);
template<typename C> C hard_light(C b, C s);
template<typename C> C soft_light(C b, C s);
template<typename C> C difference(C b, C s);
template<typename C> C exclusion(C b, C s);

template<typename C>
C normal(C b, C s)
{
	return s;
}
template<typename C>
C multiply(C b, C s)
{
	return b * s;
}
template<typename C>
C screen(C b, C s)
{
	return b + s - (b * s);
}
template<typename C>
C overlay(C b, C s)
{
	return hard_light<C>(s, b);
}
template<typename C>
C darken(C b, C s)
{
	return vmin(b, s);
}
template<typename C>
C lighten(C b, C s)
{
	return vmax(b, s);
}
template<typename C>
C color_dodge(C b, C s)
{
	// not compliant to 0-255
	if (b == C(0))
		return C(0);
	else if (s == C(1))
		return C(1);
	else
		return vmin(C(1), b / (C(1) - s));
}
template<typename C>
C color_burn(C b, C s)
{
	if (b == C(1))
		return C(1);
	else if (s == C(0))
		return C(0);
	else
		return C(1) - vmin(C(1), (C(1) - b) / s); // not compliant to 0-255
}
template<typename C>
C hard_light(C b, C s)
{
	if (s <= C(.5))
		return multiply<C>(b, C(2) * s);
	else
		return screen<C>(b, (C(2) * s) - C(1)); // not compliant to 0-255
}
template<typename C>
C soft_light_D(C b)
{
	if (b <= C(.25))
		return ((((C(16) * b) - C(12)) * b) + C(4)) * b;
	else
		return glm::sqrt(b);
}
template<typename C>
C soft_light(C b, C s)
{
	if (s <= C(.5))
		return b - ((C(1) - (C(2) * s)) * b * (C(1) - b));
	else
		return b + (((C(2) * s) - C(1)) * (soft_light_D<C>(b) - b));
}
template<typename C>
C difference(C b, C s)
{
	if (b > s)
		return b - s;
	else
		return s - b;
}
template<typename C>
C exclusion(C b, C s)
{
	return b + s - (C(2) * b * s);
}

// https://www.w3.org/TR/compositing-1/#blendingnonseparable

template<typename V, typename C = typename V::value_type> C lum(V c);
template<typename V, typename C = typename V::value_type> V clip_color(V c);
template<typename V, typename C = typename V::value_type> V set_lum(V c, C l);
template<typename V, typename C = typename V::value_type> C sat(V c);
template<typename V, typename C = typename V::value_type> V set_sat(V c, C s);
template<typename V> V hue(V b, V s);
template<typename V> V saturation(V b, V s);
template<typename V> V color(V b, V s);
template<typename V> V luminosity(V b, V s);

template<typename V, typename C = typename V::value_type>
C lum(V c)
{
	return (C(0.3) * c.r) + (C(0.59) * c.g) + (C(0.11) * c.b);
}
template<typename V, typename C = typename V::value_type>
V clip_color(V c)
{
	auto l = lum(c);
	auto n = vmin(c.r, c.g, c.b);
	auto x = vmax(c.r, c.g, c.b);
	if (n < C(0))
		return l + (((c - l) * l) / (l - n));
	if (x > C(1))
		return l + (((c - l) * (C(1) - l)) / (x - l));
	return c;
}
template<typename V, typename C = typename V::value_type>
V set_lum(V c, C l)
{
	auto d = l - lum(c);
	c.r = c.r + d;
	c.g = c.g + d;
	c.b = c.b + d;
	return clip_color(c);
}
template<typename V, typename C = typename V::value_type>
C sat(V c)
{
	return vmax(c.r, c.g, c.b) - vmin(c.r, c.g, c.b);
}
template<typename V, typename C = typename V::value_type>
V set_sat(V c, C s)
{
	// Quick and dirty way to separate min(0), mid(1) and max(2)
	std::array<C*, 3> m{&c.r, &c.g, &c.b};
	std::sort(m.begin(), m.end(), [](C * a, C * b) { return *a < *b; });

	if (*m[2] > *m[0])
	{
		*m[1] = ((*m[1] - *m[0]) * s) / (*m[2] - *m[0]);
		*m[2] = s;
	}
	else
		*m[1] = *m[2] = C(0);
	*m[0] = C(0);
	return c;
}

template<typename V>
V hue(V b, V s)
{
	return set_lum(set_sat(s, sat(b)), lum(b));
}
template<typename V>
V saturation(V b, V s)
{
	return set_lum(set_sat(b, sat(s)), lum(b));
}
template<typename V>
V color(V b, V s)
{
	return set_lum(s, lum(b));
}
template<typename V>
V luminosity(V b, V s)
{
	return set_lum(b, lum(s));
}

}

template<typename T> using BlendMode = std::function<T(T, T)>;

inline glm::vec4 _blend_callback(glm::vec4 backdrop, glm::vec4 source, BlendMode<glm::vec4> mode)
{
	return mode(backdrop, source);
}
inline glm::vec4 _blend_callback(glm::vec4 backdrop, glm::vec4 source, BlendMode<float> mode)
{
	return glm::vec4{
		mode(backdrop.r, source.r),
		mode(backdrop.g, source.g),
		mode(backdrop.b, source.b),
		0
	};
}

static float alphaCompose(float backdropAlpha, float sourceAlpha, float compositeAlpha, float backdropColor, float sourceColor, float compositeColor)
{
	return (1.f - (sourceAlpha / compositeAlpha)) * toInt<int>(backdropColor) +
		(sourceAlpha / compositeAlpha) *
			glm::round((1.f - backdropAlpha) * toInt<int>(sourceColor) + backdropAlpha * toInt<int>(compositeColor));
}

template<typename T>
RGBA _blend(RGBA background, RGBA foreground, BlendMode<T> mode)
{
	const auto backdrop = clampColor(toFloat(background));
	const auto source = clampColor(toFloat(foreground));
	const auto a = source.a + backdrop.a - source.a * backdrop.a;
	const glm::vec4 composite = _blend_callback(backdrop, source, mode);
	const RGBA result{
		glm::roundEven(alphaCompose(backdrop.a, source.a, a, backdrop.r, source.r, composite.r)),
		glm::roundEven(alphaCompose(backdrop.a, source.a, a, backdrop.g, source.g, composite.g)),
		glm::roundEven(alphaCompose(backdrop.a, source.a, a, backdrop.b, source.b, composite.b)),
		toInt<RGBA::value_type>(a)
	};
	return clampColor(result);
}

// Separable
RGBA normal(RGBA background, RGBA foreground)
{
	return _blend<float>(background, foreground, blend::normal<float>);
}
RGBA multiply(RGBA background, RGBA foreground)
{
	return _blend<float>(background, foreground, blend::multiply<float>);
}
RGBA screen(RGBA background, RGBA foreground)
{
	return _blend<float>(background, foreground, blend::screen<float>);
}
RGBA overlay(RGBA background, RGBA foreground)
{
	return _blend<float>(background, foreground, blend::overlay<float>);
}
RGBA darken(RGBA background, RGBA foreground)
{
	return _blend<float>(background, foreground, blend::darken<float>);
}
RGBA lighten(RGBA background, RGBA foreground)
{
	return _blend<float>(background, foreground, blend::lighten<float>);
}
RGBA color_dodge(RGBA background, RGBA foreground)
{
	return _blend<float>(background, foreground, blend::color_dodge<float>);
}
RGBA color_burn(RGBA background, RGBA foreground)
{
	return _blend<float>(background, foreground, blend::color_burn<float>);
}
RGBA hard_light(RGBA background, RGBA foreground)
{
	return _blend<float>(background, foreground, blend::hard_light<float>);
}
RGBA soft_light(RGBA background, RGBA foreground)
{
	return _blend<float>(background, foreground, blend::soft_light<float>);
}
RGBA difference(RGBA background, RGBA foreground)
{
	return _blend<float>(background, foreground, blend::difference<float>);
}
RGBA exclusion(RGBA background, RGBA foreground)
{
	return _blend<float>(background, foreground, blend::exclusion<float>);
}
// Non-separable
RGBA hue(RGBA background, RGBA foreground)
{
	return _blend<glm::vec4>(background, foreground, blend::hue<glm::vec4>);
}
RGBA saturation(RGBA background, RGBA foreground)
{
	return _blend<glm::vec4>(background, foreground, blend::saturation<glm::vec4>);
}
RGBA color(RGBA background, RGBA foreground)
{
	return _blend<glm::vec4>(background, foreground, blend::color<glm::vec4>);
}
RGBA luminosity(RGBA background, RGBA foreground)
{
	return _blend<glm::vec4>(background, foreground, blend::luminosity<glm::vec4>);
}

}
}
