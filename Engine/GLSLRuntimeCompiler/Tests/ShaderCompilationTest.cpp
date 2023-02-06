#include <fstream>
#include <GlslRuntimeCompiler.h>
#include <gtest/gtest.h>
#include <CommonTypes.h>

using namespace toy::core;
using namespace toy::renderer::compiler;

class ShaderCompilationTest : public ::testing::Test {
protected:
    

    std::vector<toy::core::u32> loadBinaryShaderFile(const std::string& filePath)
    {
        auto length = uint32_t{};
        auto buffer = std::vector<char>{};
        {
            using namespace std::string_literals;
            auto fileStream = std::ifstream{ filePath, std::ios::binary | std::ios::ate };
            assert(fileStream.is_open());
            length = static_cast<uint32_t>(fileStream.tellg());
            fileStream.seekg(0);

            buffer.resize(length);
            fileStream.read(buffer.data(), length);
            fileStream.close();
        }
        auto shaderSourceCode = std::vector<toy::core::u32>{};
        shaderSourceCode.resize(length / 4);
        shaderSourceCode.insert(shaderSourceCode.begin(), *buffer.data());
        return shaderSourceCode;
    }

    std::string loadTextShaderFile(const std::string& filePath)
    {
        auto length = uint32_t{};
        auto buffer = std::string{};
        buffer = "";
        {
            using namespace std::string_literals;
            
            auto fileStream = std::ifstream{ filePath,  std::ios::ate };
            assert(fileStream.is_open());
            length = static_cast<uint32_t>(fileStream.tellg());
            fileStream.seekg(0);

            buffer.resize(length);
            fileStream.read(buffer.data(), length);
            fileStream.close();
        }
        return buffer;
    }
    
};

TEST_F(ShaderCompilationTest, CompilationFaild)
{
    
    const auto shaderCode = (GlslShaderCode)loadTextShaderFile("Resources/compilationTest.frag");
    auto byteCode = ShaderByteCode{};
    const auto result = GlslRuntimeCompiler::compileToSpirv(
        GlslRuntimeCompiler::ShaderInfo
	    {
	        "entry",
	        {  },
	        ShaderStage::fragment,
	        shaderCode,
	        true
	    },
	    byteCode);
    EXPECT_TRUE(result == CompilationResult::failed);
}

TEST_F(ShaderCompilationTest, PreprocessUserDefine)
{
    const auto shaderCode = (GlslShaderCode)loadTextShaderFile("Resources/preprocessTest.glsl");
    auto byteCode = ShaderByteCode{};
    const auto debugCompilation = true;


    auto preprocessedShader = std::string{};
    const auto result1 = GlslRuntimeCompiler::preprocessGlslShader(
        GlslRuntimeCompiler::ShaderInfo
        {
            "",
            { "A" },
            ShaderStage::fragment,
            shaderCode,
            debugCompilation
        },
        preprocessedShader);

    const auto found = preprocessedShader.find("a1234") != preprocessedShader.npos;
    
    EXPECT_TRUE(found);
}

TEST_F(ShaderCompilationTest, CompilationSuccess)
{

    const auto shaderCode = (GlslShaderCode)loadTextShaderFile("Resources/compilationTest.frag");
    auto byteCode = ShaderByteCode{};
    const auto debugCompilation = true;

    const auto result = GlslRuntimeCompiler::compileToSpirv(
        GlslRuntimeCompiler::ShaderInfo
        {
            "wrong_entry",
            { "DEFINE1" },
            ShaderStage::fragment,
            shaderCode,
            debugCompilation
        },
        byteCode);
    EXPECT_TRUE(result == CompilationResult::success);
}
