#include <gtest/gtest.h>
#include "DDSLoader.h"
#include <variant>
#include <fstream>
#include <array>

using namespace toy::io::loaders::dds;



TEST(DDSLoaderTest, readTextureInfo)
{

    auto file = std::fstream("Resources/test_image_argb_8888.DDS", std::fstream::in | std::fstream::binary);
    const auto info = readTextureInfo(file);
   
    EXPECT_TRUE(info.arrayCount == 1);
    EXPECT_TRUE(info.format == TextureFormat::bgra8888unorm);
    EXPECT_TRUE(std::holds_alternative<Texture2DDimensionInfo>(info.dimensionInfo));
    
    const auto dimensionInfo = std::get<Texture2DDimensionInfo>(info.dimensionInfo);

    EXPECT_EQ(dimensionInfo.width, 256);
    EXPECT_EQ(dimensionInfo.height, 256);
    EXPECT_EQ(info.mipCount, 9);
}

TEST(DDSLoaderTest, readTextureInfoOverStreamOperator)
{

    auto file = std::fstream("Resources/test_image_argb_8888.DDS", std::fstream::in | std::fstream::binary);
    auto info = TextureInfo{};
    file >> info;

    EXPECT_TRUE(info.arrayCount == 1);
    EXPECT_TRUE(info.format == TextureFormat::bgra8888unorm);
    EXPECT_TRUE(std::holds_alternative<Texture2DDimensionInfo>(info.dimensionInfo));

    const auto dimensionInfo = std::get<Texture2DDimensionInfo>(info.dimensionInfo);

    EXPECT_EQ(dimensionInfo.width, 256);
    EXPECT_EQ(dimensionInfo.height, 256);
    EXPECT_EQ(info.mipCount, 9);
}

TEST(DDSLoaderTest, lodSizesInBytes)
{
    auto dataSource = FilestreamTextureDataSourceDescriptor{ "Resources/test_image_argb_8888.DDS" };
   
    auto info = dataSource.getTextureInfo();

    const auto testValues = std::array{ 65536 * 4, 16384 * 4, 4096 * 4, 1024 * 4, 256 * 4, 64 * 4, 16 * 4, 4 * 4, 4 };

    for (auto i = 0; i < info.mipCount; i++)
    {
        const auto bytes = lodSizeInBytes(info, i);
        EXPECT_EQ(bytes, testValues[i]);
    }
}

TEST(DDSLoaderTest, lastMipLevelValue)
{
    auto dataSource = FilestreamTextureDataSourceDescriptor{ "Resources/test_image_argb_8888.DDS" };


    auto info = dataSource.getTextureInfo();
    const auto bytes = lodSizeInBytes(info, info.mipCount-1);

    EXPECT_EQ(bytes, 4);

    auto testValue = std::array<std::uint8_t, 4>{26, 83, 133, 255};

    auto data = std::vector<std::byte>{};
    data.resize(bytes);
    auto dataSpan = std::span(data);
    loadTexture2D(dataSource, (TextureData)dataSpan, static_cast<uint8_t>(info.mipCount-1));

    //bgra8888
    EXPECT_EQ(dataSpan[0], (std::byte)testValue[0]);
    EXPECT_EQ(dataSpan[1], (std::byte)testValue[1]);
    EXPECT_EQ(dataSpan[2], (std::byte)testValue[2]);
    EXPECT_EQ(dataSpan[3], (std::byte)testValue[3]);

}

//TEST(DDSLoaderTest, lodSizesInBytesCompressed)
//{
//    auto dataSource = FilestreamTextureDataSourceDescriptor{ "Resources/test_image_bc7.DDS" };
//
//    auto info = dataSource.getTextureInfo();
//
//    const auto testValues = std::array{ 65536 * 4, 16384 * 4, 4096 * 4, 1024 * 4, 256 * 4, 64 * 4, 16 * 4, 4 * 4, 4 };
//
//    for (auto i = 0; i < info.mipCount; i++)
//    {
//        const auto bytes = lodSizeInBytes(info, i);
//        EXPECT_EQ(bytes, testValues[i]);
//    }
//}