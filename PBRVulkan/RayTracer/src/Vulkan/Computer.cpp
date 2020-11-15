#include "Computer.h"

#include "Device.h"
#include "SwapChain.h"
#include "Image.h"
#include "ImageView.h"
#include "ComputePipeline.h"
#include "CommandPool.h"
#include "CommandBuffers.h"

namespace Vulkan
{
	Computer::Computer(const SwapChain& swapChain, const Device& device, ImageView& inputImageView):
		swapChain(swapChain), device(device), inputImageView(inputImageView)
	{
		CreateOutputTexture();
		CreateComputePipeline();
		
		commandPool.reset(new CommandPool(device, device.ComputeFamilyIndex));
		commandBuffers.reset(new CommandBuffers(*commandPool, static_cast<uint32_t>(swapChain.GetImage().size())));
	}

	Computer::~Computer()
	{
		computePipeline.reset();
		commandBuffers.reset();
		commandPool.reset();
	}

	void Computer::Submit()
	{
		
	}

	void Computer::CreateComputePipeline()
	{
		computePipeline.reset(new ComputePipeline(swapChain, device, inputImageView, *outputImageView));
	}

	void Computer::CreateOutputTexture()
	{
		const auto extent = swapChain.Extent;
		const auto outputFormat = swapChain.Format;
		const auto tiling = VK_IMAGE_TILING_OPTIMAL;

		outputImage.reset(
			new Image(device, extent, outputFormat, tiling, VK_IMAGE_TYPE_2D,
			          VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

		outputImageView.reset(new ImageView(device, outputImage->Get(), outputFormat));
	}
}
