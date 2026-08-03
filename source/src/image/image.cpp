#include "image/image.hpp"
#include "thread/thread_pool.hpp"
#include <fstream>
#include <ImfHeader.h>
#include <ImfChannelList.h>
#include <ImfOutputFile.h>
#include <ImfInputFile.h>
#include <ImfFrameBuffer.h>
#include <ImfStandardAttributes.h>

Image::Image(const std::filesystem::path &filename) {
    Imf::InputFile file(filename.string().c_str());

    auto &header = file.header();
    auto data_window = header.dataWindow();
    width = data_window.max.x - data_window.min.x + 1;
    height = data_window.max.y - data_window.min.y + 1;
    pixels.resize(width * height);

    size_t x_stride = sizeof(LinearRGB);
    size_t y_stride = sizeof(LinearRGB) * width;
    auto *base = reinterpret_cast<char *>(pixels.data()) - data_window.min.x * x_stride - data_window.min.y * y_stride;

    Imf::FrameBuffer framebuffer {};
    framebuffer.insert(
        "R",
        {
            Imf::FLOAT,
            base + offsetof(LinearRGB, data.r),
            x_stride,
            y_stride
        }
    );
    framebuffer.insert(
        "G",
        {
            Imf::FLOAT,
            base + offsetof(LinearRGB, data.g),
            x_stride,
            y_stride
        }
    );
    framebuffer.insert(
        "B",
        {
            Imf::FLOAT,
            base + offsetof(LinearRGB, data.b),
            x_stride,
            y_stride
        }
    );
    file.setFrameBuffer(framebuffer);

    file.readPixels(data_window.min.y, data_window.max.y);

    if (Imf::hasChromaticities(header)) {
        color_space = ColorSpace_DCI_P3;
        auto chroma = Imf::chromaticities(header);
        ColorSpace image_color_space {
            { chroma.red.x, chroma.red.y },
            { chroma.green.x, chroma.green.y },
            { chroma.blue.x, chroma.blue.y },
            { chroma.white.x, chroma.white.y },
            {}
        };
        for (auto &linear_rgb : pixels) {
            auto temp_xyz = image_color_space.XYZFromRGB(linear_rgb);
            linear_rgb = color_space->RGBFromXYZ(temp_xyz);
        }
        printf(
            "Load From Chromaticities: R { %f, %f }, G { %f, %f }, B { %f, %f }, W { %f, %f }, convert to DCI-P3\n",
            color_space->getRed().x, color_space->getRed().y,
            color_space->getGreen().x, color_space->getGreen().y,
            color_space->getBlue().x, color_space->getBlue().y,
            color_space->getWhite().x, color_space->getWhite().y
        );
    } else {
        color_space = ColorSpace_sRGB;
    }

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
        auto rgb = getPixel(x, y);
        if (color_space != ColorSpace_sRGB) {
            auto temp_xyz = color_space->XYZFromRGB(rgb);
            rgb = ColorSpace_sRGB->RGBFromXYZ(temp_xyz);
        }
        auto encoded_rgb = ColorSpace_sRGB->encode(rgb).toBytes(8);
        buffer[idx + 0] = encoded_rgb.r;
        buffer[idx + 1] = encoded_rgb.g;
        buffer[idx + 2] = encoded_rgb.b;
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
    Imf::addChromaticities(header, {
        { color_space->getRed().x, color_space->getRed().y },
        { color_space->getGreen().x, color_space->getGreen().y },
        { color_space->getBlue().x, color_space->getBlue().y },
        { color_space->getWhite().x, color_space->getWhite().y },
    });
    Imf::OutputFile file(filename.string().c_str(), header);

    Imf::FrameBuffer framebuffer {};
    framebuffer.insert(
        "R",
        {
            Imf::FLOAT,
            const_cast<char *>(reinterpret_cast<const char *>(pixels.data())) + offsetof(LinearRGB, data.r),
            sizeof(LinearRGB),
            sizeof(LinearRGB) * width
        }
    );
    framebuffer.insert(
        "G",
        {
            Imf::FLOAT,
            const_cast<char *>(reinterpret_cast<const char *>(pixels.data())) + offsetof(LinearRGB, data.g),
            sizeof(LinearRGB),
            sizeof(LinearRGB) * width
        }
    );
    framebuffer.insert(
        "B",
        {
            Imf::FLOAT,
            const_cast<char *>(reinterpret_cast<const char *>(pixels.data())) + offsetof(LinearRGB, data.b),
            sizeof(LinearRGB),
            sizeof(LinearRGB) * width
        }
    );
    file.setFrameBuffer(framebuffer);
    file.writePixels(height);
}
