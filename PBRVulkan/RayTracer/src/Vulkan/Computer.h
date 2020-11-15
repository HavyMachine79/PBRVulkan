#pragma once

#include <memory>

#include "Vulkan.h"

namespace Vulkan
{
	class Computer final
	{
	public:
		NON_COPIABLE(Computer)

		Computer(
			const class SwapChain& swapChain,
			const class Device& device,
			const class ImageView& inputImageView,
			const class ImageView& normalsImageView,
			const class ImageView& positionsImageView);
		~Computer();

		void Submit() const;

		[[nodiscard]] const class Image& GetOutputImage() const
		{
			return *outputImage;
		}

	private:

		void CreateComputePipeline();
		void CreateOutputTexture();
		void BuildCommand() const;

		const SwapChain& swapChain;
		const Device& device;
		const ImageView& inputImageView;
		const ImageView& normalsImageView;
		const ImageView& positionsImageView;

		std::unique_ptr<class Image> outputImage;
		std::unique_ptr<class ImageView> outputImageView;
		
		std::unique_ptr<class CommandPool> commandPool;
		std::unique_ptr<class CommandBuffers> commandBuffers;
		std::unique_ptr<class ComputePipeline> computePipeline;
		std::unique_ptr<class Semaphore> semaphore;
	};
}
