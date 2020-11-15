#include "ComputePipeline.h"

#include <array>

#include "RenderPass.h"
#include "Shader.h"
#include "SwapChain.h"
#include "Device.h"
#include "ImageView.h"
#include "DescriptorSetLayout.h"
#include "DescriptorsManager.h"

namespace Vulkan
{
	ComputePipeline::ComputePipeline(const SwapChain& swapChain,
	                                 const Device& device,
	                                 ImageView& inputImage,
	                                 ImageView& outputImage): device(device), swapChain(swapChain)
	{
		const Shader computeShader(device, "Denoiser.comp.spv");

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages =
		{
			computeShader.CreateShaderStage(VK_SHADER_STAGE_COMPUTE_BIT)
		};

		const std::vector<DescriptorBinding> descriptorBindings =
		{
			{ 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT },
			{ 1, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT },
		};

		descriptorsManager.reset(new DescriptorsManager(device, swapChain, descriptorBindings));

		std::vector<VkDescriptorSetLayout> layouts(swapChain.GetImage().size(),
		                                           descriptorsManager->GetDescriptorSetLayout().Get());

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorsManager->GetDescriptorPool();
		allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChain.GetImage().size());
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(swapChain.GetImage().size());

		VK_CHECK(vkAllocateDescriptorSets(device.Get(), &allocInfo, descriptorSets.data()),
		         "Allocate descriptor sets");

		for (size_t imageIndex = 0; imageIndex < swapChain.GetImage().size(); imageIndex++)
		{
			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			// Input image
			VkDescriptorImageInfo inputImageInfo = {};
			inputImageInfo.imageView = inputImage.Get();
			inputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[imageIndex];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pImageInfo = &inputImageInfo;

			// Output image
			VkDescriptorImageInfo outputImageInfo = {};
			outputImageInfo.imageView = outputImage.Get();
			outputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[imageIndex];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &outputImageInfo;

			vkUpdateDescriptorSets(device.Get(), static_cast<uint32_t>(descriptorWrites.size()),
			                       descriptorWrites.data(), 0, nullptr);
		}

		VkDescriptorSetLayout descriptorSetLayouts[] = { descriptorsManager->GetDescriptorSetLayout().Get() };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;

		VK_CHECK(vkCreatePipelineLayout(device.Get(), &pipelineLayoutInfo, nullptr, &pipelineLayout),
		         "Create compute pipeline layout");

		VkComputePipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.flags = 0;
		pipelineInfo.pNext = nullptr;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.basePipelineHandle = nullptr;
		pipelineInfo.basePipelineIndex = -1;

		for (auto& shader : shaderStages)
		{
			VkPipeline pipeline;
			pipelineInfo.stage = shader;
			VK_CHECK(vkCreateComputePipelines(device.Get(), nullptr, 1, &pipelineInfo, nullptr, &pipeline),
			         "Create compute pipeline");
			pipelines.push_back(pipeline);
		}
	}

	ComputePipeline::~ComputePipeline()
	{
		for (auto* pipeline : pipelines)
			vkDestroyPipeline(device.Get(), pipeline, nullptr);

		pipelines.clear();
		renderPass.reset();

		if (pipelineLayout != nullptr)
		{
			vkDestroyPipelineLayout(device.Get(), pipelineLayout, nullptr);
			pipelineLayout = nullptr;
		}

		descriptorSets.clear();
	}
}
