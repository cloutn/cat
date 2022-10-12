#pragma once

#include "scl/type.h"

namespace cat {
	
class DeviceLimits 
{
public:
    uint32              maxImageDimension1D;
    uint32              maxImageDimension2D;
    uint32              maxImageDimension3D;
    uint32              maxImageDimensionCube;
    uint32              maxImageArrayLayers;
    uint32              maxTexelBufferElements;
    uint32              maxUniformBufferRange;
    uint32              maxStorageBufferRange;
    uint32              maxPushConstantsSize;
    uint32              maxMemoryAllocationCount;
    uint32              maxSamplerAllocationCount;
    uint64              bufferImageGranularity;
    uint64              sparseAddressSpaceSize;
    uint32              maxBoundDescriptorSets;
    uint32              maxPerStageDescriptorSamplers;
    uint32              maxPerStageDescriptorUniformBuffers;
    uint32              maxPerStageDescriptorStorageBuffers;
    uint32              maxPerStageDescriptorSampledImages;
    uint32              maxPerStageDescriptorStorageImages;
    uint32              maxPerStageDescriptorInputAttachments;
    uint32              maxPerStageResources;
    uint32              maxDescriptorSetSamplers;
    uint32              maxDescriptorSetUniformBuffers;
    uint32              maxDescriptorSetUniformBuffersDynamic;
    uint32              maxDescriptorSetStorageBuffers;
    uint32              maxDescriptorSetStorageBuffersDynamic;
    uint32              maxDescriptorSetSampledImages;
    uint32              maxDescriptorSetStorageImages;
    uint32              maxDescriptorSetInputAttachments;
    uint32              maxVertexInputAttributes;
    uint32              maxVertexInputBindings;
    uint32              maxVertexInputAttributeOffset;
    uint32              maxVertexInputBindingStride;
    uint32              maxVertexOutputComponents;
    uint32              maxTessellationGenerationLevel;
    uint32              maxTessellationPatchSize;
    uint32              maxTessellationControlPerVertexInputComponents;
    uint32              maxTessellationControlPerVertexOutputComponents;
    uint32              maxTessellationControlPerPatchOutputComponents;
    uint32              maxTessellationControlTotalOutputComponents;
    uint32              maxTessellationEvaluationInputComponents;
    uint32              maxTessellationEvaluationOutputComponents;
    uint32              maxGeometryShaderInvocations;
    uint32              maxGeometryInputComponents;
    uint32              maxGeometryOutputComponents;
    uint32              maxGeometryOutputVertices;
    uint32              maxGeometryTotalOutputComponents;
    uint32              maxFragmentInputComponents;
    uint32              maxFragmentOutputAttachments;
    uint32              maxFragmentDualSrcAttachments;
    uint32              maxFragmentCombinedOutputResources;
    uint32              maxComputeSharedMemorySize;
    uint32              maxComputeWorkGroupCount[3];
    uint32              maxComputeWorkGroupInvocations;
    uint32              maxComputeWorkGroupSize[3];
    uint32              subPixelPrecisionBits;
    uint32              subTexelPrecisionBits;
    uint32              mipmapPrecisionBits;
    uint32              maxDrawIndexedIndexValue;
    uint32              maxDrawIndirectCount;
    float               maxSamplerLodBias;
    float               maxSamplerAnisotropy;
    uint32              maxViewports;
    uint32              maxViewportDimensions[2];
    float               viewportBoundsRange[2];
    uint32              viewportSubPixelBits;
    size_t              minMemoryMapAlignment;
    uint64              minTexelBufferOffsetAlignment;
    uint64              minUniformBufferOffsetAlignment;
    uint64              minStorageBufferOffsetAlignment;
    int32               minTexelOffset;
    uint32              maxTexelOffset;
    int32               minTexelGatherOffset;
    uint32              maxTexelGatherOffset;
    float               minInterpolationOffset;
    float               maxInterpolationOffset;
    uint32              subPixelInterpolationOffsetBits;
    uint32              maxFramebufferWidth;
    uint32              maxFramebufferHeight;
    uint32              maxFramebufferLayers;
    uint32              framebufferColorSampleCounts;
    uint32              framebufferDepthSampleCounts;
    uint32              framebufferStencilSampleCounts;
    uint32              framebufferNoAttachmentsSampleCounts;
    uint32              maxColorAttachments;
    uint32              sampledImageColorSampleCounts;
    uint32              sampledImageIntegerSampleCounts;
    uint32              sampledImageDepthSampleCounts;
    uint32              sampledImageStencilSampleCounts;
    uint32              storageImageSampleCounts;
    uint32              maxSampleMaskWords;
    uint32              timestampComputeAndGraphics;
    float               timestampPeriod;
    uint32              maxClipDistances;
    uint32              maxCullDistances;
    uint32              maxCombinedClipAndCullDistances;
    uint32              discreteQueuePriorities;
    float               pointSizeRange[2];
    float               lineWidthRange[2];
    float               pointSizeGranularity;
    float               lineWidthGranularity;
    uint32              strictLines;
    uint32              standardSampleLocations;
    uint64              optimalBufferCopyOffsetAlignment;
    uint64              optimalBufferCopyRowPitchAlignment;
    uint64              nonCoherentAtomSize;

}; // class DeviceLimits


class DeviceInfo
{
public:
	enum DEVICE_TYPE 
	{
		DEVICE_TYPE_OTHER			= 0,
		DEVICE_TYPE_INTEGRATED_GPU	= 1,
		DEVICE_TYPE_DISCRETE_GPU	= 2,
		DEVICE_TYPE_VIRTUAL_GPU		= 3,
		DEVICE_TYPE_CPU				= 4,
	};

    //uint32		        apiVersion; //uint major = VK_VERSION_MAJOR(prop.apiVersion); //uint minor = VK_VERSION_MINOR(prop.apiVersion); //uint patch = VK_VERSION_PATCH(prop.apiVersion);
    uint16              apiVersionMajor;
    uint16              apiVersionMinor;
    uint16              apiVersionPatch;
    uint32		        driverVersion;
    uint32		        vendorID;
    uint32		        deviceID;
    DEVICE_TYPE	        deviceType;
    char                deviceName[256];
    uint8		        pipelineCacheUUID[16];
    DeviceLimits        limits;

}; // class DeviceInfo


} // namespace cat


