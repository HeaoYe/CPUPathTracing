#include "image/image.hpp"
#include "thread/thread_pool.hpp"
#include "util/rgb.hpp"
#include <fstream>
#include <ImfHeader.h>
#include <ImfChannelList.h>
#include <ImfOutputFile.h>
#include <ImfInputFile.h>
#include <ImfFrameBuffer.h>

Image::Image(const std::filesystem::path &filename) {
    Imf::InputFile file(filename.string().c_str());

    auto data_window = file.header().dataWindow();
    width = data_window.max.x - data_window.min.x + 1;
    height = data_window.max.y - data_window.min.y + 1;
    pixels.resize(width * height);

    Imf::FrameBuffer framebuffer {};
    framebuffer.insert(
        "R",
        {
            Imf::FLOAT,
            reinterpret_cast<char *>(pixels.data()) + offsetof(glm::vec3, r),
            sizeof(glm::vec3),
            sizeof(glm::vec3) * width
        }
    );
    framebuffer.insert(
        "G",
        {
            Imf::FLOAT,
            reinterpret_cast<char *>(pixels.data()) + offsetof(glm::vec3, g),
            sizeof(glm::vec3),
            sizeof(glm::vec3) * width
        }
    );
    framebuffer.insert(
        "B",
        {
            Imf::FLOAT,
            reinterpret_cast<char *>(pixels.data()) + offsetof(glm::vec3, b),
            sizeof(glm::vec3),
            sizeof(glm::vec3) * width
        }
    );
    file.setFrameBuffer(framebuffer);

    file.readPixels(data_window.min.y, data_window.max.y);

    printf("Load Image '%s', Size(%lld, %lld)\n", filename.string().c_str(), width, height);
}

void Image::save(const std::filesystem::path &filename) const {
    if (filename.extension() == ".ppm") {
        savePPM(filename);
    } else if (filename.extension() == ".exr") {
        saveEXR(filename);
    }
}

void Image::savePPM(const std::filesystem::path &filename) const {
    std::ofstream file(filename, std::ios::binary);
    file << "P6\n" << width << ' ' << height << "\n255\n";

    std::vector<uint8_t> buffer(width * height * 3);

    thread_pool.parallelFor(width, height, [&](size_t x, size_t y) {
        size_t idx = (y * width + x) * 3;
        RGB rgb { getPixel(x, y) };
        buffer[idx + 0] = rgb.r;
        buffer[idx + 1] = rgb.g;
        buffer[idx + 2] = rgb.b;
    }, false);
    thread_pool.wait();

    file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
}

void Image::saveEXR(const std::filesystem::path &filename) const {
    Imf::Header header { static_cast<int>(width), static_cast<int>(height) };
    header.channels().insert("R", { Imf::FLOAT });
    header.channels().insert("G", { Imf::FLOAT });
    header.channels().insert("B", { Imf::FLOAT });
    header.compression() = Imf::ZIP_COMPRESSION;
    header.dataWindow() = {
        { 0, 0 },
        { static_cast<int>(width - 1), static_cast<int>(height - 1) }
    };
    Imf::OutputFile file(filename.string().c_str(), header);

    Imf::FrameBuffer framebuffer {};
    framebuffer.insert(
        "R",
        {
            Imf::FLOAT,
            const_cast<char *>(reinterpret_cast<const char *>(pixels.data())) + offsetof(glm::vec3, r),
            sizeof(glm::vec3),
            sizeof(glm::vec3) * width
        }
    );
    framebuffer.insert(
        "G",
        {
            Imf::FLOAT,
            const_cast<char *>(reinterpret_cast<const char *>(pixels.data())) + offsetof(glm::vec3, g),
            sizeof(glm::vec3),
            sizeof(glm::vec3) * width
        }
    );
    framebuffer.insert(
        "B",
        {
            Imf::FLOAT,
            const_cast<char *>(reinterpret_cast<const char *>(pixels.data())) + offsetof(glm::vec3, b),
            sizeof(glm::vec3),
            sizeof(glm::vec3) * width
        }
    );
    file.setFrameBuffer(framebuffer);
    file.writePixels(height);
}
