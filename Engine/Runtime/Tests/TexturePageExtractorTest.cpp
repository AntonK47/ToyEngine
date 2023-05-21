#include <gtest/gtest.h>
#include "DDSLoader.h"
#include "TexturePageExtractor.h"
#include <variant>
#include <fstream>
#include <array>

using namespace toy::io::loaders::dds;

TEST(TexturePageExtractor, readOnePage)
{
    auto dataSource = FilestreamTextureDataSourceDescriptor{ "Resources/test_image_argb_8888.DDS" };

    auto info = dataSource.getTextureInfo();
    const auto bytes = lodSizeInBytes(info, 0);

    auto data = std::vector<std::byte>{};
    data.resize(bytes);
    auto dataSpan = std::span(data);
    loadTexture2D(dataSource, (TextureData)dataSpan, static_cast<uint8_t>(0));
}
