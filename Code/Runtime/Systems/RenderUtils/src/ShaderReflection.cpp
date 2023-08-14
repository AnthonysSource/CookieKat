#include "CookieKat/Systems/RenderUtils/ShaderReflection.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"

#include "spirv_reflect.h"

namespace CKE {
	Vector<SpvReflectDescriptorSet*> ReflectShaderDescriptorSets(spv_reflect::ShaderModule& shaderModule) {
		// Reflect DescriptorSets from Spirv code
		u32 size;
		shaderModule.EnumerateDescriptorSets(&size, nullptr);
		Vector<SpvReflectDescriptorSet*> descriptorSets(size);
		shaderModule.EnumerateDescriptorSets(&size, descriptorSets.data());
		return descriptorSets;
	}

	Vector<SpvReflectInterfaceVariable*> ReflectInput(spv_reflect::ShaderModule& shaderModule) {
		// Reflect DescriptorSets from Spirv code
		u32 size;
		shaderModule.EnumerateInputVariables(&size, nullptr);
		Vector<SpvReflectInterfaceVariable*> inputVars(size);
		shaderModule.EnumerateInputVariables(&size, inputVars.data());
		return inputVars;
	}

	ShaderBindingType GetReflectedShaderBindingType(SpvReflectDescriptorType type) {
		switch (type) {
		case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return ShaderBindingType::ImageViewSampler;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return ShaderBindingType::UniformBuffer;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER: return ShaderBindingType::StorageBuffer;
		default: CKE_UNREACHABLE_CODE();
		}
		return (ShaderBindingType)0;
	}

	VertexInputFormat GetReflectedVertexInputFormat(SpvReflectFormat format) {
		switch (format) {
		case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT: return VertexInputFormat::Float_R32G32B32;
		case SPV_REFLECT_FORMAT_R32G32_SFLOAT: return VertexInputFormat::Float_R32G32;
		default: CKE_UNREACHABLE_CODE();
		}
		return (VertexInputFormat)0;
	}
}

namespace CKE {
	PipelineLayoutDesc ShaderReflectionUtils::ReflectLayout(Vector<u8> const& vertSource, Vector<u8> const& fragSource) {
		PipelineLayoutDesc    layoutDesc{};
		Vector<ShaderBinding> shaderBindings{};

		if (!fragSource.empty()) {
			spv_reflect::ShaderModule        shaderModule{fragSource};
			Vector<SpvReflectDescriptorSet*> reflDescriptorSets = ReflectShaderDescriptorSets(shaderModule);

			// Iterate reflected data and create pipeline layout
			for (SpvReflectDescriptorSet* reflSet : reflDescriptorSets) {
				u32 set = reflSet->set;
				for (int i = 0; i < reflSet->binding_count; ++i) {
					SpvReflectDescriptorBinding* b = reflSet->bindings[i];
					ShaderBindingType            bindingType = GetReflectedShaderBindingType(b->descriptor_type);
					shaderBindings.push_back(ShaderBinding{set, b->binding, bindingType, b->count, ShaderStageMask::Fragment});
				}
			}
		}
		if (!vertSource.empty()) {
			spv_reflect::ShaderModule        shaderModule{vertSource};
			Vector<SpvReflectDescriptorSet*> reflDescriptorSets = ReflectShaderDescriptorSets(shaderModule);

			// Iterate reflected data and create pipeline layout
			for (SpvReflectDescriptorSet* reflSet : reflDescriptorSets) {
				u32 set = reflSet->set;
				for (int i = 0; i < reflSet->binding_count; ++i) {
					SpvReflectDescriptorBinding* b = reflSet->bindings[i];
					ShaderBindingType            bindingType = GetReflectedShaderBindingType(b->descriptor_type);
					shaderBindings.push_back(ShaderBinding{set, b->binding, bindingType, b->count, ShaderStageMask::Vertex});
				}
			}
		}

		layoutDesc.SetShaderBindings(shaderBindings);
		return layoutDesc;
	}

	VertexInputLayoutDesc ShaderReflectionUtils::ReflectVertexInput(Vector<u8> const& vertSource) {
		VertexInputLayoutDesc   desc{};
		Vector<VertexInputInfo> vertexInputs{};

		if (!vertSource.empty()) {
			spv_reflect::ShaderModule            shaderModule{vertSource};
			Vector<SpvReflectInterfaceVariable*> reflInput = ReflectInput(shaderModule);

			for (SpvReflectInterfaceVariable* input : reflInput) {
				if (input->built_in != -1) {
					continue;
				}
				VertexInputFormat vertFormat = GetReflectedVertexInputFormat(input->format);
				vertexInputs.push_back(VertexInputInfo{vertFormat, input->location});
			}
		}

		// TODO: Look into this implicit bug-prone requirement
		std::sort(std::begin(vertexInputs),
		          std::end(vertexInputs),
		          [](const VertexInputInfo& a,
		             const VertexInputInfo& b) {
			          return a.m_Location < b.m_Location;
		          });

		desc.SetVertexInput(vertexInputs);
		return desc;
	}
}
