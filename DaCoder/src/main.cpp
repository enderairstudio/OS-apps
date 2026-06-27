#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <opencv2/opencv.hpp>
#include "pff.h"

void print_usage() {
    std::cout << "DaCoder: Command line tool for encoding/decoding PFF (Pixel File Format) images\n";
    std::cout << "Usage:\n";
    std::cout << "  dacoder info <file.pff>\n";
    std::cout << "  dacoder decode <file.pff> <output_image.png/jpg/bmp>\n";
    std::cout << "  dacoder encode <input_image.png/jpg/bmp> <output_file.pff> [options]\n\n";
    std::cout << "Encode Options:\n";
    std::cout << "  --rle                     Enable Run-Length Encoding compression (default: uncompressed)\n";
    std::cout << "  --filter <filter_type>    Apply filter: normal, grayscale, sepia, invert, warm, cool (default: normal)\n";
}

std::string filter_to_string(uint8_t filter) {
    switch (filter) {
        case pff::FILTER_NORMAL: return "Normal";
        case pff::FILTER_GRAYSCALE: return "Grayscale";
        case pff::FILTER_SEPIA: return "Sepia";
        case pff::FILTER_INVERT: return "Inverted";
        case pff::FILTER_WARM: return "Warm";
        case pff::FILTER_COOL: return "Cool";
        default: return "Unknown";
    }
}

pff::FilterType parse_filter(const std::string& name) {
    if (name == "grayscale" || name == "gray") return pff::FILTER_GRAYSCALE;
    if (name == "sepia") return pff::FILTER_SEPIA;
    if (name == "invert" || name == "inversion") return pff::FILTER_INVERT;
    if (name == "warm") return pff::FILTER_WARM;
    if (name == "cool") return pff::FILTER_COOL;
    return pff::FILTER_NORMAL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        print_usage();
        return 1;
    }

    std::string command = argv[1];

    if (command == "info") {
        std::string filepath = argv[2];
        pff::Image img;
        if (!pff::load_pff(filepath, img)) {
            std::cerr << "Error: Failed to load PFF file: " << filepath << std::endl;
            return 1;
        }

        std::time_t raw_time = static_cast<std::time_t>(img.header.timestamp);
        std::tm* timeinfo = std::localtime(&raw_time);
        char time_str[64];
        std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);

        std::cout << "PFF File Info for '" << filepath << "':\n";
        std::cout << "  Magic Bytes:     " << std::string(img.header.magic, 4) << "\n";
        std::cout << "  Dimensions:      " << img.header.width << " x " << img.header.height << "\n";
        std::cout << "  Channels:        " << (int)img.header.channels << " (" << ((int)img.header.channels == 3 ? "RGB" : "RGBA") << ")\n";
        std::cout << "  Filter Applied:  " << filter_to_string(img.header.filter_type) << "\n";
        std::cout << "  Compression:     " << ((int)img.header.compression == pff::COMPRESS_RLE ? "RLE (Compressed)" : "None (Uncompressed)") << "\n";
        std::cout << "  Timestamp:       " << time_str << " (" << img.header.timestamp << ")\n";
        std::cout << "  Pixel Data Size: " << img.pixels.size() << " bytes (decompressed)\n";

    } else if (command == "decode") {
        if (argc < 4) {
            std::cerr << "Error: Missing output filepath.\n";
            print_usage();
            return 1;
        }
        std::string filepath = argv[2];
        std::string out_filepath = argv[3];

        pff::Image img;
        if (!pff::load_pff(filepath, img)) {
            std::cerr << "Error: Failed to load PFF file: " << filepath << std::endl;
            return 1;
        }

        // Convert raw pixels (RGB or RGBA) to OpenCV Mat
        int type = (img.header.channels == 4) ? CV_8UC4 : CV_8UC3;
        cv::Mat mat(img.header.height, img.header.width, type, img.pixels.data());

        // OpenCV expects BGR or BGRA, convert from RGB/RGBA
        cv::Mat out_mat;
        if (img.header.channels == 4) {
            cv::cvtColor(mat, out_mat, cv::COLOR_RGBA2BGRA);
        } else {
            cv::cvtColor(mat, out_mat, cv::COLOR_RGB2BGR);
        }

        if (!cv::imwrite(out_filepath, out_mat)) {
            std::cerr << "Error: Failed to write image to " << out_filepath << std::endl;
            return 1;
        }
        std::cout << "Successfully decoded '" << filepath << "' to '" << out_filepath << "'\n";

    } else if (command == "encode") {
        if (argc < 4) {
            std::cerr << "Error: Missing output filepath.\n";
            print_usage();
            return 1;
        }
        std::string in_filepath = argv[2];
        std::string out_filepath = argv[3];

        // Parse optional arguments
        bool use_rle = false;
        pff::FilterType filter = pff::FILTER_NORMAL;

        for (int i = 4; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--rle") {
                use_rle = true;
            } else if (arg == "--filter" && i + 1 < argc) {
                filter = parse_filter(argv[++i]);
            }
        }

        // Load input image using OpenCV
        cv::Mat mat = cv::imread(in_filepath, cv::IMREAD_UNCHANGED);
        if (mat.empty()) {
            std::cerr << "Error: Failed to read image: " << in_filepath << std::endl;
            return 1;
        }

        // Convert OpenCV mat (BGR/BGRA) to RGB/RGBA
        cv::Mat rgb_mat;
        int channels = mat.channels();
        if (channels == 4) {
            cv::cvtColor(mat, rgb_mat, cv::COLOR_BGRA2RGBA);
        } else if (channels == 3) {
            cv::cvtColor(mat, rgb_mat, cv::COLOR_BGR2RGB);
        } else if (channels == 1) {
            // Grayscale
            cv::cvtColor(mat, rgb_mat, cv::COLOR_GRAY2RGB);
            channels = 3;
        } else {
            std::cerr << "Error: Unsupported number of channels: " << channels << std::endl;
            return 1;
        }

        // Build PFF Image
        pff::Image img;
        std::memcpy(img.header.magic, "PFF!", 4);
        img.header.width = rgb_mat.cols;
        img.header.height = rgb_mat.rows;
        img.header.channels = channels;
        img.header.filter_type = filter;
        img.header.compression = use_rle ? pff::COMPRESS_RLE : pff::COMPRESS_NONE;
        
        // Use current time
        auto now = std::chrono::system_clock::now();
        img.header.timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        
        // Zero-fill reserved bytes
        std::memset(img.header.reserved, 0, sizeof(img.header.reserved));

        // Copy raw pixel data
        size_t pixel_data_size = static_cast<size_t>(img.header.width) * img.header.height * img.header.channels;
        img.pixels.resize(pixel_data_size);
        std::memcpy(img.pixels.data(), rgb_mat.data, pixel_data_size);

        // Apply filter if specified
        if (filter != pff::FILTER_NORMAL) {
            pff::apply_filter(img.pixels, img.header.width, img.header.height, img.header.channels, filter);
        }

        // Save PFF
        if (!pff::save_pff(out_filepath, img)) {
            std::cerr << "Error: Failed to save PFF to " << out_filepath << std::endl;
            return 1;
        }

        std::cout << "Successfully encoded '" << in_filepath << "' to '" << out_filepath << "'\n";
    } else {
        std::cerr << "Error: Unknown command '" << command << "'\n";
        print_usage();
        return 1;
    }

    return 0;
}
