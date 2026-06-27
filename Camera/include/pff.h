#ifndef PFF_H
#define PFF_H

#include <vector>
#include <string>
#include <cstdint>

namespace pff {

// Filter types
enum FilterType : uint8_t {
    FILTER_NORMAL = 0,
    FILTER_GRAYSCALE = 1,
    FILTER_SEPIA = 2,
    FILTER_INVERT = 3,
    FILTER_WARM = 4,
    FILTER_COOL = 5
};

// Compression modes
enum CompressionMode : uint8_t {
    COMPRESS_NONE = 0,
    COMPRESS_RLE = 1
};

#pragma pack(push, 1)
struct Header {
    char magic[4];          // "PFF!"
    uint32_t width;         // Width of the image
    uint32_t height;        // Height of the image
    uint8_t channels;       // Channels: 3 for RGB, 4 for RGBA
    uint8_t filter_type;    // Filter applied (FilterType)
    uint8_t compression;    // Compression type (CompressionMode)
    uint8_t reserved[9];    // Padding/reserved for future expansions
    uint64_t timestamp;     // Unix timestamp of creation (seconds since epoch)
};
#pragma pack(pop)

struct Image {
    Header header;
    std::vector<uint8_t> pixels; // Decoded pixel data in raw RGB/RGBA format
};

// Writes an image structure to a .pff file
bool save_pff(const std::string& filepath, const Image& img);

// Reads an image structure from a .pff file
bool load_pff(const std::string& filepath, Image& img);

// Apply filter to raw pixels in place
void apply_filter(std::vector<uint8_t>& pixels, int width, int height, int channels, uint8_t filter);

} // namespace pff

#endif // PFF_H
