#pragma once

#include <RenderInterface.h>
#include <RenderInterfaceTypes.h>
#include <Logger.h>
#include <type_traits>
#include <iostream>

using namespace toy::graphics::rhi;
namespace toy::graphics::features
{
	//

	struct OutlineFeatureDescriptor
	{};

	class OutlineFeature
	{
	public:

		OutlineFeature(RenderInterface& rhi) : rhi_{rhi}{}

		void initialize()
		{
		std::cout << "outline init\n";
			

			Flags<ImageAccessUsage> depthUsage = ImageAccessUsage::depthStencilAttachment;
			depthUsage |= ImageAccessUsage::sampled;

			const auto imageDescriptor = ImageDescriptor
			{
		        .format = Format::d16,
		        .extent = Extent{100, 50},
		        .mips = 1,
		        .layers = 1,
		        .accessUsage = depthUsage,
		    };

			depth_ = rhi_.createImage(imageDescriptor);
		}

			

		void deinitialize()
		{

		}

		void render()
		{
		}
	private:


		Handle<Image> depth_;
		Handle<Image> ping_;
		Handle<Image> pong_;
		RenderInterface& rhi_;
		
	};
}
