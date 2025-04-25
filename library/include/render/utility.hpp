#pragma once
#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <functional>

namespace utility
{
	using RGBA = glm::u8vec4;
	// Generic tile position
	using TilePosition = glm::ivec3;
	// Generic plane position
	using PlanePosition = glm::ivec2;
	using Vector = glm::vec3;
	using Direction = glm::vec3;

	using RegionPosition = glm::ivec2;
	using ChunkPosition = glm::ivec2;
	using SectionPosition = glm::ivec3;
	using BlockPosition = glm::ivec3;

	namespace color
	{
		RGBA blend(const RGBA & a, const RGBA & b, int h = 128);
		RGBA interpolate(const RGBA & a, const RGBA & b, float n);
		RGBA lerp(const RGBA & a, const RGBA & b, float n);
	}

	namespace space
	{
		struct RayTracing
		{
		public:
			RayTracing(Vector pos, Direction dir);
			RayTracing(const RayTracing &) = default;
			Vector next();
		private:
			Vector pos;
			Direction step;
			Vector tMax;
			Direction tDelta;
		};
		TilePosition to(Vector pos);
		PlanePosition toPlane(Vector pos);
		PlanePosition::value_type proj(Vector::value_type p,
			Vector::value_type a1, Vector::value_type b1,
			PlanePosition::value_type a2, PlanePosition::value_type b2);
	}

	namespace coord
	{
		RegionPosition toRegion(const ChunkPosition & cpos);
		ChunkPosition toChunk(const SectionPosition & spos);
		SectionPosition toSection(const BlockPosition & bpos);

		BlockPosition modSection(const BlockPosition & bpos);

		int32_t toSection(int32_t b);
	}

	namespace math
	{
		std::size_t mod(int32_t v, std::size_t m);
		std::size_t indexMod2d(std::size_t width, int32_t x, int32_t z);
		std::size_t index2d(std::size_t width, int32_t x, int32_t z);
	}
}

#endif // UTILITY_HPP
