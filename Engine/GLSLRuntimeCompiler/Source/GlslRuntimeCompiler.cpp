#include "GlslRuntimeCompiler.h"

#include <sstream>
#include <glslang/Include/glslang_c_interface.h>

using namespace toy::renderer::compiler;

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
{/*
	glslang::InitializeProcess();
	auto spirvStage = mapShaderStage(info.shaderStage);
	auto shader = glslang::TShader{ spirvStage };
	//auto shader = setUpShader(info);
	auto cstringChaderCode = info.shaderCode.c_str();
	shader.setStrings(&cstringChaderCode, 1);
	shader.setEnvInput(glslang::EShSource::EShSourceGlsl, spirvStage,
		glslang::EShClient::EShClientVulkan,
		100);
	
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
	spvOptions.validate = true;

	auto spirvLogger = spv::SpvBuildLogger{};
	glslang::GlslangToSpv(*shader.getIntermediate(), byteCode, &spirvLogger, &spvOptions);

	const auto messages = spirvLogger.getAllMessages();
	if(!messages.empty())
	{
		LOG("info", messages);
	}
	glslang::FinalizeProcess();
*/
	glslang_initialize_process();
	const auto stage = static_cast<glslang_stage_t>(mapShaderStage(info.shaderStage));
	const glslang_input_t input = {
		.language = GLSLANG_SOURCE_GLSL,
		.stage = stage,
		.client = GLSLANG_CLIENT_VULKAN,
		.client_version = GLSLANG_TARGET_VULKAN_1_3,
		.target_language = GLSLANG_TARGET_SPV,
		.target_language_version = GLSLANG_TARGET_SPV_1_6,
		.code = info.shaderCode.data(),
		.default_version = 460,
		.default_profile = GLSLANG_NO_PROFILE,
		.force_default_version_and_profile = false,
		.forward_compatible = false,
		.messages = GLSLANG_MSG_DEFAULT_BIT,
		.resource = reinterpret_cast<const glslang_resource_t*>(&defaultTBuiltInResource_),
	};

	glslang_shader_t* shader = glslang_shader_create(&input);

	if (!glslang_shader_preprocess(shader, &input)) {
		printf("GLSL preprocessing failed %s\n", "aa.txt");
		printf("%s\n", glslang_shader_get_info_log(shader));
		printf("%s\n", glslang_shader_get_info_debug_log(shader));
		printf("%s\n", input.code);
		glslang_shader_delete(shader);
		return CompilationResult::failed;
	}

	if (!glslang_shader_parse(shader, &input)) {
		printf("GLSL parsing failed %s\n", "aa.txt");
		printf("%s\n", glslang_shader_get_info_log(shader));
		printf("%s\n", glslang_shader_get_info_debug_log(shader));
		printf("%s\n", glslang_shader_get_preprocessed_code(shader));
		glslang_shader_delete(shader);
		return CompilationResult::failed;
	}

	glslang_program_t* program = glslang_program_create();
	glslang_program_add_shader(program, shader);

	if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
		printf("GLSL linking failed %s\n", "aa.txt");
		printf("%s\n", glslang_program_get_info_log(program));
		printf("%s\n", glslang_program_get_info_debug_log(program));
		glslang_program_delete(program);
		glslang_shader_delete(shader);
		return CompilationResult::failed;
	}

	//TODO: this options should be pass from the user land
	auto options = glslang_spv_options_t
	{
		.generate_debug_info = true,
		.strip_debug_info = false,
		.disable_optimizer = true,
		.optimize_size = false,
		.disassemble = true,
		.validate = true,
		.emit_nonsemantic_shader_debug_info = false,
		.emit_nonsemantic_shader_debug_source = false
	};

	//BUG: createModule function results an validation error
	auto optionsOptimized = glslang_spv_options_t
	{
		.generate_debug_info = false,
		.strip_debug_info = false,
		.disable_optimizer = false,
		.optimize_size = false,
		.disassemble = true,
		.validate = true,
		.emit_nonsemantic_shader_debug_info = true,
		.emit_nonsemantic_shader_debug_source = true
	};
	glslang_program_SPIRV_generate_with_options(program, stage, &options);
	
	byteCode.resize(glslang_program_SPIRV_get_size(program));
	glslang_program_SPIRV_get(program, byteCode.data());

	const char* spirv_messages = glslang_program_SPIRV_get_messages(program);
	if (spirv_messages)
		printf("(%s) %s\b", "aa.txt", spirv_messages);

	glslang_program_delete(program);
	glslang_shader_delete(shader);

	glslang_finalize_process();
	return CompilationResult::success;
}
