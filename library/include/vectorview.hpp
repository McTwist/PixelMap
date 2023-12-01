#ifndef VECTORVIEW_HPP
#define VECTORVIEW_HPP

#include <cstddef>
#include <vector>
#include <iterator>

// TODO: Replace with std::span (C++20)

/**
 * @brief Vector View
 * Takes a span of memory to be viewed as a normal container.
 * Memory is never allocated, but accessed directly in a neat way.
 */
template<class T>
class VectorView
{
public:
	/**
	 * @brief Default constructor
	 */
	VectorView() = default;
	/**
	 * @brief Initialize constructor
	 * @param ptr The start of the view.
	 * @param size The size of the view.
	 */
	VectorView(T * ptr, std::size_t size) noexcept : _ptr{ptr}, _size{size} {}

	/**
	 * Several STL compliant methods.
	 */
	T & operator[](int i) noexcept { return _ptr[i]; }
	const T & operator[](int i) const noexcept { return _ptr[i]; }
	auto size() const noexcept { return _size; }
	bool empty() const noexcept { return _size == 0; }
	T * data() noexcept { return _ptr; }
	const T * data() const noexcept { return _ptr; }

	auto begin() noexcept { return _ptr; }
	auto end() noexcept { return _ptr + _size; }
	auto begin() const noexcept { return _ptr; }
	auto end() const noexcept { return _ptr + _size; }

private:
	T * _ptr;
	std::size_t _size;
};

#endif // VECTORVIEW_HPP
