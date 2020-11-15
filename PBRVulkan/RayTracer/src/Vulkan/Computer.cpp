#include "Computer.h"

#include <iostream>


#include "Device.h"
#include "SwapChain.h"
#include "Image.h"
#include "ImageView.h"
#include "ComputePipeline.h"
#include "CommandPool.h"
#include "CommandBuffers.h"
#include "Semaphore.h"

namespace Vulkan
{
	Computer::Computer(const SwapChain& swapChain,
	                   const Device& device,
	                   const ImageView& inputImageView,
	                   const ImageView& normalsImageView,
	                   const ImageView& positionsImageView):

		swapChain(swapChain), device(device),
		inputImageView(inputImageView),
		normalsImageView(normalsImageView),
		positionsImageView(positionsImageView)
	{
		commandPool.reset(new CommandPool(device, device.ComputeFamilyIndex));
		commandBuffers.reset(new CommandBuffers(*commandPool, 1));
		semaphore.reset(new Semaphore(device));

		CreateOutputTexture();
		CreateComputePipeline();
	}

	Computer::~Computer()
	{
		commandBuffers.reset();
		commandPool.reset();
		computePipeline.reset();

		std::cout << "[COMPUTER] Compute pipeline has been deleted." << std::endl;
	}

	void Computer::Submit() const
	{
		VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		VkCommandBuffer commandBuffer[]{ (*commandBuffers)[0] };

		VkSubmitInfo computeSubmitInfo{};
		computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		computeSubmitInfo.pNext = nullptr;
		computeSubmitInfo.pWaitDstStageMask = &waitStageMask;
		computeSubmitInfo.commandBufferCount = 1;
		computeSubmitInfo.pCommandBuffers = commandBuffer;
		vkQueueSubmit(device.ComputeQueue, 1, &computeSubmitInfo, nullptr);
	}

	void Computer::CreateComputePipeline()
	{
		computePipeline.reset(new ComputePipeline(
			swapChain,
			device,
			inputImageView,
			*outputImageView,
			normalsImageView,
			positionsImageView));
		BuildCommand();
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

	void Computer::BuildCommand() const
	{
		vkQueueWaitIdle(device.ComputeQueue);

		VkImageSubresourceRange subresourceRange;
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;

		const auto extent = swapChain.Extent;

		VkDescriptorSet descriptorSets[] = { computePipeline->GetDescriptorSets()[0] };

		VkCommandBuffer commandBuffer = commandBuffers->Begin(0);
		{
			Image::MemoryBarrier(commandBuffer, outputImage->Get(), subresourceRange, 0,
			                     VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

			Image::MemoryBarrier(commandBuffer, inputImageView.GetImage(), subresourceRange, 0,
			                     VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

			Image::MemoryBarrier(commandBuffer, positionsImageView.GetImage(), subresourceRange, 0,
			                     VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

			Image::MemoryBarrier(commandBuffer, normalsImageView.GetImage(), subresourceRange, 0,
			                     VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline->GetComputePipelines()[0]);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline->GetPipelineLayout(),
			                        0, 1, descriptorSets, 0, nullptr);
			vkCmdDispatch(commandBuffer, extent.width / 16, extent.height / 16, 1);
		}
		commandBuffers->End(0);
	}
}
