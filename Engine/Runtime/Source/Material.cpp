#include "Material.h"

#include <Core.h>
#include <GlslRuntimeCompiler.h>
#include <Logger.h>
#include <RenderInterface.h>
#include <RenderInterfaceTypes.h>


#include <regex>

using namespace toy::graphics::compiler;
namespace toy
{
	auto MaterialGenerator::compilePipeline(RenderInterface& renderer, const MaterialCompilationDescriptor& descriptor) -> SharedMaterial
	{
		const std::string generatedFragmentCode =
			"vec3 evaluateMaterialLighting(Attributes attributes)"
			"{"
			"	vec3 diffuse = texture(sampler2D(textures2D[6], textureSampler), vec2(uv)).xyz;"
			"   vec3 normalTS = texture(sampler2D(textures2D[7], textureSampler), vec2(uv)).xyz;"
			"    vec3 normalWS = ntb * normalTS;"
			"    vec3 lightDirection = normalize(vec3(1.0, 1.0, 1.0));"
			"    float radiance = max(0, dot(lightDirection, normalize(normalWS)));"
			"    vec3 shadowColor = vec3(130.0/255.0, 163.0/255.0, 255.0/255.0) * 0.15;"
			"    vec3 diffuseColor = vec3(0.9, 0.4, 0.1);"
			"    vec3 color = mix(shadowColor, diffuse, radiance);"
			"    return color;"
			"}";

		const std::string evaluateMaterialFragment
			= "evaluateMaterialLighting";

		auto generatedVertexShaderGlslCode = descriptor.materialTemplate.vertexShaderTemplate;
		auto generatedFragmentShaderGlslCode = descriptor.materialTemplate.fragmentShaderTemplate;

		generatedFragmentShaderGlslCode = std::regex_replace(generatedFragmentShaderGlslCode, std::regex{ std::string{"<\\[generated_fragment_code\\]>"} }, generatedFragmentCode);
		generatedFragmentShaderGlslCode = std::regex_replace(generatedFragmentShaderGlslCode, std::regex{ std::string{"<\\[evaluate_material_fragment\\]>"} }, evaluateMaterialFragment);



		const auto vertexShaderInfo =
			GlslRuntimeCompiler::ShaderInfo
		{
			.entryPoint = "main",
			.compilationDefines = {},
			.shaderStage = compiler::ShaderStage::vertex,
			.shaderCode = generatedVertexShaderGlslCode,
			.enableDebugCompilation = true
		};

		const auto fragmentShaderInfo =
			GlslRuntimeCompiler::ShaderInfo
		{
			.entryPoint = "main",
			.compilationDefines = {},
			.shaderStage = compiler::ShaderStage::fragment,
			.shaderCode = generatedFragmentShaderGlslCode,
			.enableDebugCompilation = true
		};


		auto vsSpirvCode = ShaderByteCode{};
		auto fsSpirvCode = ShaderByteCode{};

		auto result = GlslRuntimeCompiler::compileToSpirv(vertexShaderInfo, vsSpirvCode);
		TOY_ASSERT(result == CompilationResult::success);

		result = GlslRuntimeCompiler::compileToSpirv(fragmentShaderInfo, fsSpirvCode);
		TOY_ASSERT(result == CompilationResult::success);

		const auto vertexShaderModule = renderer.createShaderModule(
			toy::graphics::rhi::ShaderStage::vertex,
			{
				ShaderLanguage::spirv1_6,
				vsSpirvCode
			}
		);

		const auto fragmentShaderModule = renderer.createShaderModule(
			toy::graphics::rhi::ShaderStage::vertex,
			{
				ShaderLanguage::spirv1_6,
				fsSpirvCode
			}
		);

		const auto pipeline = renderer.createPipeline(
			GraphicsPipelineDescriptor
			{
				.vertexShader = vertexShaderModule,
				.fragmentShader = fragmentShaderModule,
				.renderTargetDescriptor = RenderTargetsDescriptor
				{
					.colorRenderTargets = std::vector
					{
						ColorRenderTargetDescriptor{ ColorFormat::rgba8 }
					},
					.depthRenderTarget = DepthRenderTargetDescriptor{ DepthFormat::d32 }
				},
				.state = PipelineState
				{
					.depthTestEnabled = true,
					.faceCulling = FaceCull::back
				}
			},
				{
					SetBindGroupMapping{0, descriptor.frameContext.geometryLayout},
					SetBindGroupMapping{1, descriptor.frameContext.perFrameDataLayout},
					SetBindGroupMapping{2, descriptor.frameContext.perInstanceDataLayout},
					SetBindGroupMapping{3, descriptor.frameContext.texturesLayout},
					SetBindGroupMapping{4, descriptor.frameContext.samplerLayout}
				},
				{
					PushConstant({.size = sizeof(u32)})
				}
				);

		SharedMaterial sharedMaterial;
		sharedMaterial.pipeline = pipeline;
		return sharedMaterial;
	}
}