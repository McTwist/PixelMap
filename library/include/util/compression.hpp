#pragma once
#ifndef COMPRESSION_HPP
#define COMPRESSION_HPP

#include "vectorview.hpp"

#include <vector>
#include <cstdint>

#include <stddef.h>

namespace Compression
{

std::vector<uint8_t> loadZLib(const std::vector<uint8_t> & compressed);
std::vector<uint8_t> loadZLibRaw(const std::vector<uint8_t> & compressed);
std::vector<uint8_t> loadGZip(const std::vector<uint8_t> & compressed);
std::vector<uint8_t> loadZLib(const VectorView<const uint8_t> & compressed);
std::vector<uint8_t> loadZLibRaw(const VectorView<const uint8_t> & compressed);
std::vector<uint8_t> loadGZip(const VectorView<const uint8_t> & compressed);

std::vector<uint8_t> loadLZ4(const std::vector<uint8_t> & compressed);
std::vector<uint8_t> loadLZ4(const VectorView<const uint8_t> & compressed);

}

#endif // COMPRESSION_HPP
