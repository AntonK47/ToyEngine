#include "GlslRuntimeCompiler.h"
#include <iostream>
#include <sstream>
using namespace toy::renderer::compiler;


#define LOG(severity, msg) std::cout << "[" << severity << "]: " << msg << std::endl

namespace 
{
	EShLanguage mapShaderStage(const ShaderStage stage)
	{
		switch(stage)
		{
		case ShaderStage::vertex: return EShLangVertex;
		case ShaderStage::fragment: return EShLangFragment;
		case ShaderStage::geometry: return EShLangGeometry;
		case ShaderStage::tessellationControl: return EShLangTessControl;
		case ShaderStage::tessellationEvaluation: return EShLangTessEvaluation;
		case ShaderStage::task: return EShLangTaskNV;//soon it will change to non vender specific version
		case ShaderStage::mesh: return EShLangMeshNV;
		case ShaderStage::anyHit: return EShLangAnyHit;
		case ShaderStage::closestHit: return EShLangClosestHit;
		case ShaderStage::miss: return EShLangMiss;
		case ShaderStage::rayGeneration: return EShLangRayGen;
		case ShaderStage::intersection: return EShLangIntersect;
		}
		return {};
	}

	std::string buildShaderPreamble(const std::vector<std::string>& defines)
	{
		auto preambleString = std::ostringstream{};

		auto preamble = std::string{ };
		for (const auto& define : defines)
		{
			preambleString << "#define " << define << "\n";
		}

		return preambleString.str();
	}
}

CompilationResult GlslRuntimeCompiler::preprocessGlslShader(const ShaderInfo& info, std::string& glslShaderCode)
{
	glslang::InitializeProcess();
	auto spirvStage = mapShaderStage(info.shaderStage);
	auto shader = glslang::TShader{ spirvStage };

	auto cstringChaderCode = info.shaderCode.c_str();
	shader.setStrings(&cstringChaderCode, 1);
	shader.setEnvInput(glslang::EShSource::EShSourceGlsl, spirvStage,
		glslang::EShClient::EShClientVulkan,
		glslang::EShTargetClientVersion::EShTargetVulkan_1_3);

	const auto preamble = buildShaderPreamble(info.compilationDefines);
	shader.setPreamble(preamble.c_str());
	
	shader.setEnvClient(glslang::EShClient::EShClientVulkan,
		glslang::EShTargetClientVersion::EShTargetVulkan_1_3);
	shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv,
		glslang::EShTargetLanguageVersion::EShTargetSpv_1_6);
	shader.setEntryPoint(info.entryPoint.c_str());
	shader.setSourceEntryPoint("main");


	glslang::TShader::ForbidIncluder i;

	shader.preprocess(&defaultTBuiltInResource_,
		460, EProfile::ENoProfile, false, false, static_cast<EShMessages>(EShMessages::EShMsgVulkanRules | EShMessages::EShMsgSpvRules), &glslShaderCode, i);

	glslang::FinalizeProcess();
	return CompilationResult::success;
}


CompilationResult GlslRuntimeCompiler::compileToSpirv(const ShaderInfo& info, ShaderByteCode& byteCode)
{
	glslang::InitializeProcess();
	auto spirvStage = mapShaderStage(info.shaderStage);
	auto shader = glslang::TShader{ spirvStage };
	//auto shader = setUpShader(info);
	auto cstringChaderCode = info.shaderCode.c_str();
	shader.setStrings(&cstringChaderCode, 1);
	shader.setEnvInput(glslang::EShSource::EShSourceGlsl, spirvStage,
		glslang::EShClient::EShClientVulkan,
		glslang::EShTargetClientVersion::EShTargetVulkan_1_3);
	
	const auto preamble = buildShaderPreamble(info.compilationDefines);
	shader.setPreamble(preamble.c_str());
	shader.setEnvClient(glslang::EShClient::EShClientVulkan,
		glslang::EShTargetClientVersion::EShTargetVulkan_1_3);
	shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv,
		glslang::EShTargetLanguageVersion::EShTargetSpv_1_6);
	shader.setEntryPoint(info.entryPoint.c_str());
	shader.setSourceEntryPoint("main");
	auto resources = TBuiltInResource{};

	const auto hasShaderParseErrors = !shader.parse(&defaultTBuiltInResource_,
	                                                460,
	                                                false,
	                                                static_cast<EShMessages>(EShMessages::EShMsgVulkanRules | EShMessages::EShMsgSpvRules));

	if (hasShaderParseErrors)
	{
		LOG("error", shader.getInfoLog());
		LOG("error", shader.getInfoDebugLog());
		glslang::FinalizeProcess();
		return CompilationResult::failed;
		
	}

	auto spvOptions = glslang::SpvOptions{};

	if(info.enableDebugCompilation)
	{
		spvOptions.generateDebugInfo = true;
		spvOptions.disableOptimizer = true;
		spvOptions.optimizeSize = false;
	}
	else
	{
		spvOptions.generateDebugInfo = false;
		spvOptions.disableOptimizer = false;
		spvOptions.optimizeSize = true;//TODO: Need more research!
	}

	auto spirvLogger = spv::SpvBuildLogger{};
	glslang::GlslangToSpv(*shader.getIntermediate(), byteCode, &spirvLogger, &spvOptions);

	const auto messages = spirvLogger.getAllMessages();
	if(!messages.empty())
	{
		LOG("info", messages);
	}
	glslang::FinalizeProcess();

	return CompilationResult::success;
}
