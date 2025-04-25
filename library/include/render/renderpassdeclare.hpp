#pragma once
#ifndef RENDERPASSDECLARE_HPP
#define RENDERPASSDECLARE_HPP

#include "render/passbuilder.hpp"
#include "render/utility.hpp"

#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>

// Forward declarations
class Chunk;
struct ChunkRenderData;
struct RegionRenderData;

// Pass function declarations
using ChunkPassFunction = std::function<std::shared_ptr<ChunkRenderData>(const Chunk &)>;
using RegionPassFunction = std::function<std::shared_ptr<RegionRenderData>(int x, int z, const std::vector<std::shared_ptr<ChunkRenderData>> &)>;
using WorldPassFunction = std::function<void(const std::unordered_map<utility::RegionPosition, std::shared_ptr<RegionRenderData>> &)>;

#endif // RENDERPASSDECLARE_HPP
