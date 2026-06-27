#include "pff.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <algorithm>

namespace pff {

bool save_pff(const std::string& filepath, const Image& img) {
    std::ofstream out(filepath, std::ios::binary);
    if (!out) {
        std::cerr << "PFF Error: Cannot open file for writing: " << filepath << std::endl;
        return false;
    }

    // Write header
    out.write(reinterpret_cast<const char*>(&img.header), sizeof(Header));

    if (img.header.compression == COMPRESS_NONE) {
        // Write raw pixels
        out.write(reinterpret_cast<const char*>(img.pixels.data()), img.pixels.size());
    } else if (img.header.compression == COMPRESS_RLE) {
        int channels = img.header.channels;
        size_t num_pixels = img.pixels.size() / channels;
        
        size_t i = 0;
        while (i < num_pixels) {
            size_t run_len = 1;
            const uint8_t* current_pixel = &img.pixels[i * channels];

            while (i + run_len < num_pixels && run_len < 255) {
                const uint8_t* next_pixel = &img.pixels[(i + run_len) * channels];
                bool match = true;
                for (int c = 0; c < channels; ++c) {
                    if (current_pixel[c] != next_pixel[c]) {
                        match = false;
                        break;
                    }
                }
                if (!match) break;
                run_len++;
            }

            // Write 1 byte run length
            uint8_t count = static_cast<uint8_t>(run_len);
            out.write(reinterpret_cast<const char*>(&count), 1);
            // Write pixel data
            out.write(reinterpret_cast<const char*>(current_pixel), channels);

            i += run_len;
        }
    }

    return out.good();
}

bool load_pff(const std::string& filepath, Image& img) {
    std::ifstream in(filepath, std::ios::binary);
    if (!in) {
        std::cerr << "PFF Error: Cannot open file for reading: " << filepath << std::endl;
        return false;
    }

    // Read header
    in.read(reinterpret_cast<char*>(&img.header), sizeof(Header));
    if (!in || std::strncmp(img.header.magic, "PFF!", 4) != 0) {
        std::cerr << "PFF Error: Invalid magic bytes in file: " << filepath << std::endl;
        return false;
    }

    size_t expected_size = static_cast<size_t>(img.header.width) * img.header.height * img.header.channels;
    img.pixels.resize(expected_size);

    if (img.header.compression == COMPRESS_NONE) {
        in.read(reinterpret_cast<char*>(img.pixels.data()), expected_size);
        if (in.gcount() != static_cast<std::streamsize>(expected_size)) {
            std::cerr << "PFF Warning: Read fewer bytes than expected for uncompressed image." << std::endl;
        }
    } else if (img.header.compression == COMPRESS_RLE) {
        int channels = img.header.channels;
        size_t bytes_written = 0;

        while (bytes_written < expected_size) {
            uint8_t count = 0;
            in.read(reinterpret_cast<char*>(&count), 1);
            if (!in) break;

            std::vector<uint8_t> pixel(channels);
            in.read(reinterpret_cast<char*>(pixel.data()), channels);
            if (!in) break;

            for (uint8_t r = 0; r < count; ++r) {
                if (bytes_written + channels <= expected_size) {
                    std::memcpy(&img.pixels[bytes_written], pixel.data(), channels);
                    bytes_written += channels;
                } else {
                    // Overflow guard
                    break;
                }
            }
        }

        if (bytes_written < expected_size) {
            std::cerr << "PFF Warning: RLE stream ended early. Read " << bytes_written 
                      << " of " << expected_size << " bytes." << std::endl;
        }
    }

    return true;
}

void apply_filter(std::vector<uint8_t>& pixels, int width, int height, int channels, uint8_t filter) {
    size_t num_pixels = static_cast<size_t>(width) * height;
    
    for (size_t i = 0; i < num_pixels; ++i) {
        uint8_t* p = &pixels[i * channels];
        uint8_t r = p[0];
        uint8_t g = p[1];
        uint8_t b = p[2];

        switch (filter) {
            case FILTER_GRAYSCALE: {
                uint8_t gray = static_cast<uint8_t>(0.299f * r + 0.587f * g + 0.114f * b);
                p[0] = gray;
                p[1] = gray;
                p[2] = gray;
                break;
            }
            case FILTER_SEPIA: {
                int tr = static_cast<int>(0.393f * r + 0.769f * g + 0.189f * b);
                int tg = static_cast<int>(0.349f * r + 0.686f * g + 0.168f * b);
                int tb = static_cast<int>(0.272f * r + 0.534f * g + 0.131f * b);
                p[0] = static_cast<uint8_t>(std::min(tr, 255));
                p[1] = static_cast<uint8_t>(std::min(tg, 255));
                p[2] = static_cast<uint8_t>(std::min(tb, 255));
                break;
            }
            case FILTER_INVERT: {
                p[0] = 255 - r;
                p[1] = 255 - g;
                p[2] = 255 - b;
                break;
            }
            case FILTER_WARM: {
                // Enhance reds and yellows
                int tr = r + 30;
                int tg = g + 15;
                p[0] = static_cast<uint8_t>(std::min(tr, 255));
                p[1] = static_cast<uint8_t>(std::min(tg, 255));
                break;
            }
            case FILTER_COOL: {
                // Enhance blues
                int tb = b + 35;
                int tg = g + 10;
                p[2] = static_cast<uint8_t>(std::min(tb, 255));
                p[1] = static_cast<uint8_t>(std::min(tg, 255));
                break;
            }
            case FILTER_NORMAL:
            default:
                break;
        }
    }
}

} // namespace pff
