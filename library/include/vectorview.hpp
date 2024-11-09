#ifndef VECTORVIEW_HPP
#define VECTORVIEW_HPP

#include <cstddef>
#include <vector>

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
	using reference = T &;
	using const_reference = const T &;
	using pointer = T *;
	using const_pointer = const T *;
	using iterator = T *;
	using const_iterator = const T *;

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
	 * @brief Initialize constructor
	 * @param start The start of the view.
	 * @param end The end of the view.
	 */
	VectorView(T * start, T * end) noexcept : _ptr(start), _size(std::distance(start, end)) {}

	/**
	 * Several STL compliant methods.
	 */
	reference operator[](int i) noexcept { return _ptr[i]; }
	const_reference operator[](int i) const noexcept { return _ptr[i]; }
	auto size() const noexcept { return _size; }
	bool empty() const noexcept { return _size == 0; }
	pointer data() noexcept { return _ptr; }
	const_pointer data() const noexcept { return _ptr; }

	iterator begin() noexcept { return _ptr; }
	iterator end() noexcept { return _ptr + _size; }
	const_iterator begin() const noexcept { return _ptr; }
	const_iterator end() const noexcept { return _ptr + _size; }

private:
	T * _ptr = nullptr;
	std::size_t _size = 0;
};

template<typename T>
inline const std::vector<T> & toVector(const std::vector<T> & v)
{
	return v;
}

template<typename T>
inline std::vector<T> toVector(const VectorView<T> & v)
{
	return std::vector<T>{v.data(), v.data() + v.size()};
}

#endif // VECTORVIEW_HPP
