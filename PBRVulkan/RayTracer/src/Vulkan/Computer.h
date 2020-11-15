#pragma once

#include <memory>

#include "Vulkan.h"

namespace Vulkan
{
	class Computer final
	{
	public:
		NON_COPIABLE(Computer)

		Computer(const class SwapChain& swapChain, const class Device& device, class ImageView& inputImageView);
		~Computer();

		void Submit();

		[[nodiscard]] const class Image& GetOutputImage() const
		{
			return *outputImage;
		}
		
	private:

		void CreateComputePipeline();
		void CreateOutputTexture();

		const SwapChain& swapChain;
		const Device& device;
		ImageView& inputImageView;
		std::unique_ptr<class Image> outputImage;
		std::unique_ptr<class ImageView> outputImageView;
		std::unique_ptr<class CommandPool> commandPool;
		std::unique_ptr<class CommandBuffers> commandBuffers;
		std::unique_ptr<class ComputePipeline> computePipeline;
	};
}
