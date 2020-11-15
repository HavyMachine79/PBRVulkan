#pragma once

#include "Vulkan.h"

#include <memory>
#include <vector>

namespace Vulkan
{
	class ComputePipeline final
	{
	public:
		NON_COPIABLE(ComputePipeline)

		ComputePipeline(const class SwapChain& swapChain,
		                const class Device& device,
		                class ImageView& inputImage,
		                class ImageView& outputImage);

		~ComputePipeline();

		[[nodiscard]] const std::vector<VkDescriptorSet>& GetDescriptorSets() const
		{
			return descriptorSets;
		}

		[[nodiscard]] VkRenderPass GetRenderPass() const;

	private:
		const Device& device;
		const SwapChain& swapChain;

		std::vector<VkPipeline> pipelines{};
		VkPipelineLayout pipelineLayout{};

		std::vector<VkDescriptorSet> descriptorSets;
		std::unique_ptr<class RenderPass> renderPass;
		std::unique_ptr<class DescriptorsManager> descriptorsManager;
	};
}
