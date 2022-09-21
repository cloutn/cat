#include "./simplevulkan.h"

#include "./load_png.h"

#include "scl/math.h"

#include <spirv_cross/spirv_glsl.hpp>
#include <spirv_cross/spirv_cross_c.h>
#include <spirv_cross/spirv_common.hpp>

#include <vulkan/vulkan.h>

#include <assert.h>
#include <windows.h>

#define USE_EXTERNAL_GFX_LIB

#ifdef USE_EXTERNAL_GFX_LIB
#include "libimg/image.h"
#endif

#define memclr(s) memset(&s, 0, sizeof(s))
#define countof(s) (sizeof(s)/sizeof(s[0]))
#define offset(type, member) ((unsigned char*)(&(((type*)0)->member)))
#define vkcheck(x)	do\
{ \
	VkResult _res_ = (x); \
	_last_error_ = _res_; \
	assert( _res_ == VK_SUCCESS ); \
} while (false)

typedef unsigned int uint;

struct  example_vertex
{
	float	pos[4];
	float	texcoord[2];
};

static VkResult _last_error_;

enum SHADER_TYPE
{
	SHADER_TYPE_VERT,
	SHADER_TYPE_TCS,
	SHADER_TYPE_TES,
	SHADER_TYPE_GEO,
	SHADER_TYPE_FRAG,
	SHADER_TYPE_COMP,
};

static shaderc_shader_kind _toShadercType(SHADER_TYPE type)
{
	switch (type)
	{
	case SHADER_TYPE_VERT	: return shaderc_glsl_vertex_shader;
	case SHADER_TYPE_TCS	: return shaderc_glsl_tess_control_shader;
	case SHADER_TYPE_TES	: return shaderc_glsl_tess_evaluation_shader;
	case SHADER_TYPE_GEO	: return shaderc_glsl_geometry_shader;
	case SHADER_TYPE_FRAG	: return shaderc_glsl_fragment_shader;
	case SHADER_TYPE_COMP	: return shaderc_glsl_compute_shader;
	default					: assert(false); return static_cast<shaderc_shader_kind>(-1);
	}
}

static VkShaderStageFlagBits _toShaderStageFlagBits(SHADER_TYPE type)
{
	switch (type)
	{
	case SHADER_TYPE_VERT	: return VK_SHADER_STAGE_VERTEX_BIT;
	case SHADER_TYPE_TCS	: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case SHADER_TYPE_TES	: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case SHADER_TYPE_GEO	: return VK_SHADER_STAGE_GEOMETRY_BIT;
	case SHADER_TYPE_FRAG	: return VK_SHADER_STAGE_FRAGMENT_BIT;
	case SHADER_TYPE_COMP	: return VK_SHADER_STAGE_COMPUTE_BIT;
	default					: assert(false); return static_cast<VkShaderStageFlagBits>(0);
	}
}


static int _getMemoryTypeIndex(uint typeBits, const VkMemoryType* types, VkFlags require)
{
	int i = 0;
	while (true)
	{
		if (((typeBits & 1) == 1 && ((types[i].propertyFlags & require) == require)) || i > 32)
			break;

		typeBits >>= 1;
		++i;
	}
	if (i >= 32 || i < 0)
		i = -1;
	return i;
}

static VkBuffer _genBuffer(svkDevice& device, VkBufferUsageFlags usage, const int dataSize, VkDeviceMemory* outputMemory)
{
	VkResult err;
	VkBufferCreateInfo bufferCreateInfo;
	memclr(bufferCreateInfo);
	bufferCreateInfo.sType	= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage	= usage;
	bufferCreateInfo.size	= dataSize;

	VkBuffer buffer;
	err = vkCreateBuffer(device.device, &bufferCreateInfo, NULL, &buffer);
	assert(!err);

	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(device.device, buffer, &memReq);

	VkMemoryAllocateInfo memAlloc;
	memclr(memAlloc);
	memAlloc.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.pNext				= NULL;
	memAlloc.allocationSize		= memReq.size;
	memAlloc.memoryTypeIndex	= _getMemoryTypeIndex(memReq.memoryTypeBits, device.memoryProperties.memoryTypes, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	assert(memAlloc.memoryTypeIndex != -1);

	VkDeviceMemory memory;
	err = vkAllocateMemory(device.device, &memAlloc, NULL, &memory);
	assert(!err);

	err = vkBindBufferMemory(device.device, buffer, memory, 0);
	assert(!err);
	if (NULL != outputMemory)
		*outputMemory = memory;
	return buffer;
}

static void _createTextureImage(svkDevice& device, svkTexture* texObj, const int32_t texWidth, const int32_t texHeight, const VkFormat texFormat, VkImageTiling tiling, VkImageUsageFlags usage, VkFlags requiredProps, VkDeviceSize& memoryAllocationSize)
{
	VkResult err;

	VkImageCreateInfo imageCreateInfo;
	memclr(imageCreateInfo);
	imageCreateInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext			= NULL;
	imageCreateInfo.imageType		= VK_IMAGE_TYPE_2D;
	imageCreateInfo.format			= texFormat;
	imageCreateInfo.extent.width	= (uint32_t)texWidth;
	imageCreateInfo.extent.height	= (uint32_t)texHeight;
	imageCreateInfo.extent.depth	= 1;
	imageCreateInfo.mipLevels		= 1;
	imageCreateInfo.arrayLayers		= 1;
	imageCreateInfo.samples			= VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling			= tiling;
	imageCreateInfo.usage			= usage;
	imageCreateInfo.flags			= 0;
	imageCreateInfo.initialLayout	= VK_IMAGE_LAYOUT_PREINITIALIZED;

	err = vkCreateImage(device.device, &imageCreateInfo, NULL, &texObj->image);
	assert(!err);

	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(device.device, texObj->image, &memReq);

	VkMemoryAllocateInfo memAlloc;
	memclr(memAlloc);
	memAlloc.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.pNext				= NULL;
	memAlloc.allocationSize		= memReq.size;
	memAlloc.memoryTypeIndex	= _getMemoryTypeIndex(memReq.memoryTypeBits, device.memoryProperties.memoryTypes, requiredProps);
	assert(memAlloc.memoryTypeIndex != -1);

	memoryAllocationSize		= memAlloc.allocationSize;

	// allocate memory 
	err = vkAllocateMemory(device.device, &memAlloc, NULL, &(texObj->memory));
	assert(!err);

	// bind memory 
	err = vkBindImageMemory(device.device, texObj->image, texObj->memory, 0);
	assert(!err);
}

static bool _copyFromFileToTexture(svkDevice& device, FILE* f, svkTexture* texObj, const VkFlags requiredProps, const VkDeviceSize memroyAllocationSize)
{
	VkResult err = VK_SUCCESS;
	bool result = true;
	if (requiredProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) 
	{
		VkImageSubresource subres;
		memclr(subres);
		subres.aspectMask	= VK_IMAGE_ASPECT_COLOR_BIT;
		subres.mipLevel		= 0;
		subres.arrayLayer	= 0;
		VkSubresourceLayout layout;
		void* data			= NULL;

		vkGetImageSubresourceLayout(device.device, texObj->image, &subres, &layout);

		err = vkMapMemory(device.device, texObj->memory, 0, memroyAllocationSize, 0, &data);
		assert(!err);

#ifdef USE_EXTERNAL_GFX_LIB
		if (NULL == img::load_img_to_buffer(f, (unsigned char*)data, NULL, NULL, NULL, NULL))
#else
		if (NULL == load_png_data_to_memory(f, (unsigned char*)data, NULL, NULL, NULL, NULL))
#endif
		{
			//printf("Error loading texture: %s\n", filename);
			assert(false);
			result = false;
		}
		vkUnmapMemory(device.device, texObj->memory);
	}

	texObj->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	return result;
}

static void _createTextureImageWithSize(svkDevice& device, const int width, const int height, svkTexture* texObj, VkImageTiling tiling, VkImageUsageFlags usage, VkFlags requiredProps) 
{
	texObj->width	= width;
	texObj->height	= height;

	const VkFormat texFormat = VK_FORMAT_R8G8B8A8_UNORM;

	VkDeviceSize memroyAllocationSize = 0;
	_createTextureImage(device, texObj, width, height, texFormat, tiling, usage, requiredProps, memroyAllocationSize);
}

static void _createTextureImageFromFile(svkDevice& device, const char* const filename, svkTexture* texObj, VkImageTiling tiling, VkImageUsageFlags usage, VkFlags requiredProps) 
{
	int32_t texWidth	= 0;
	int32_t texHeight	= 0;
	int		pixel		= 0;

#pragma warning(push)
#pragma warning(disable:4996)
	FILE* f = fopen(filename, "rb");
#pragma warning(pop)

#ifdef USE_EXTERNAL_GFX_LIB
	img::get_img_size(f, &texWidth, &texHeight, &pixel);
#else
	load_png_data_to_memory(f, NULL, &texWidth, &texHeight, NULL, NULL);
#endif

	texObj->width	= texWidth;
	texObj->height	= texHeight;

	VkFormat texFormat = VK_FORMAT_R8G8B8A8_UNORM;
	//if (pixel == 3) // pixel is 3 bytes
	//	texFormat = VK_FORMAT_R8G8B8_UNORM;

	VkDeviceSize memroyAllocationSize = 0;
	_createTextureImage(device, texObj, texWidth, texHeight, texFormat, tiling, usage, requiredProps, memroyAllocationSize);

	bool copyResult = _copyFromFileToTexture(device, f, texObj, requiredProps, memroyAllocationSize);
	if (!copyResult)
	{
		printf("Error loading texture: %s\n", filename);
	}

	fclose(f);
}

static void _setImageLayout(
	VkCommandBuffer			commandBuffer,
	VkImage					image, 
	VkImageAspectFlags		aspectMask, 
	VkImageLayout			oldImageLayout,
	VkImageLayout			newImageLayout, 
	VkAccessFlagBits		srcAccessMask, 
	VkPipelineStageFlags	srcStages,
	VkPipelineStageFlags	destStages) 
{
	VkImageMemoryBarrier imageMemoryBarrier;
	memclr(imageMemoryBarrier);
	imageMemoryBarrier.sType				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext				= NULL;
	imageMemoryBarrier.srcAccessMask		= srcAccessMask;
	imageMemoryBarrier.dstAccessMask		= 0;
	imageMemoryBarrier.srcQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.oldLayout			= oldImageLayout;
	imageMemoryBarrier.newLayout			= newImageLayout;
	imageMemoryBarrier.image				= image;
	imageMemoryBarrier.subresourceRange.aspectMask		= aspectMask;
	imageMemoryBarrier.subresourceRange.baseMipLevel	= 0;
	imageMemoryBarrier.subresourceRange.levelCount		= 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer	= 0;
	imageMemoryBarrier.subresourceRange.layerCount		= 1;

	switch (newImageLayout) {
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL				: imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL			: imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL	: imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL			: imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT; break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL				: imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT; break;
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR					: imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT; break;
	default													: imageMemoryBarrier.dstAccessMask = 0; break;
	}

	VkImageMemoryBarrier* pMemoryBarrier = &imageMemoryBarrier;
	vkCmdPipelineBarrier(commandBuffer, srcStages, destStages, 0, 0, NULL, 0, NULL, 1, pMemoryBarrier);
}

static bool _isLayerExists(VkLayerProperties* properties, const int propertyCount, const char* layerName)
{
	for (int i = 0; i < propertyCount; ++i)
	{
		if (0 == strcmp(layerName, properties[i].layerName))
			return true;
	}
	return false;
}

static VkPhysicalDevice _choosePhysicalDevice(VkInstance inst)
{
	VkPhysicalDevice phys[4];
	int physCount = 4;
	vkcheck(vkEnumeratePhysicalDevices(inst, (uint*)&physCount, phys));
	if (physCount <= 0)
	{
		assert(false);
		return NULL;
	}
	int chooseDevice = 0;
	for (int i = 0; i < physCount; ++i)
	{
		VkPhysicalDeviceProperties prop;
		vkGetPhysicalDeviceProperties(phys[i], &prop);
		//printf("device [%d] : %d, %d, %d, %d, %d, %s\n", i, prop.apiVersion, prop.driverVersion, prop.vendorID, prop.deviceID, prop.deviceType, prop.deviceName);
		//uint major = VK_VERSION_MAJOR(prop.apiVersion); //uint minor = VK_VERSION_MINOR(prop.apiVersion); //uint patch = VK_VERSION_PATCH(prop.apiVersion);
		//printf("device limit : minUniformBufferOffsetAlignment = [%lld]\n", prop.limits.minUniformBufferOffsetAlignment);
		if (prop.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			continue;
		chooseDevice = i;
		break;
	}
	return phys[chooseDevice];
}

static VkShaderModule _createShader(VkDevice device, const uint32_t *code, size_t size)
{
	VkShaderModule module;

	VkShaderModuleCreateInfo moduleCreateInfo;
	memclr(moduleCreateInfo);
	moduleCreateInfo.sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext		= NULL;
	moduleCreateInfo.flags		= 0;
	moduleCreateInfo.codeSize	= size;
	moduleCreateInfo.pCode		= code;

	vkcheck( vkCreateShaderModule(device, &moduleCreateInfo, NULL, &module) );

	return module;
}

void sprvc_error_callback(void *userdata, const char *error)
{
	printf("spirv-cross error = %s\n", error);
}

char* spvc_resource_type_names[14] = 
{
	"UNKNOWN",
	"UNIFORM_BUFFER",
	"STORAGE_BUFFER",
	"STAGE_INPUT",
	"STAGE_OUTPUT",
	"SUBPASS_INPUT",
	"STORAGE_IMAGE",
	"SAMPLED_IMAGE",
	"ATOMIC_COUNTER",
	"PUSH_CONSTANT",
	"SEPARATE_IMAGE",
	"SEPARATE_SAMPLERS",
	"ACCELERATION_STRUCTURE",
	"RAY_QUERY",
};

static void __testSpirv_Cross(const char* bytes, const int bytesLen, const char* const filename)
{
	const SpvId*					spirv			= (const SpvId*)bytes;
	size_t							word_count		= bytesLen / sizeof(SpvId);
	spvc_context					context			= NULL;
	spvc_parsed_ir					ir				= NULL;
	spvc_compiler					compiler_glsl	= NULL;
	spvc_compiler_options			options			= NULL;
	spvc_resources					resources		= NULL;
	const spvc_reflected_resource*	list			= NULL;
	const char*						spvc_result		= NULL;
	size_t							count			= 0;

	// Create context.
	spvc_context_create(&context);

	// Set debug callback.
	spvc_context_set_error_callback(context, sprvc_error_callback, NULL);

	// Parse the SPIR-V.
	spvc_context_parse_spirv(context, spirv, word_count, &ir);

	// Hand it off to a compiler instance and give it ownership of the IR.
	spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler_glsl);

	// Do some basic reflection.
	if (NULL != compiler_glsl)
	{
		spvc_compiler_create_shader_resources(compiler_glsl, &resources);

		printf("\n================== start to print file = [%s]==================\n", filename);
		for (int t = SPVC_RESOURCE_TYPE_UNIFORM_BUFFER; t < SPVC_RESOURCE_TYPE_RAY_QUERY; ++t)
		{
			//spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, &list, &count);
			spvc_resources_get_resource_list_for_type(resources, (spvc_resource_type)t, &list, &count);
			printf("type [%s] count = %zd\n", spvc_resource_type_names[t], count);

			for (size_t i = 0; i < count; i++)
			{
				printf("\tID: %u, BaseTypeID: %u, TypeID: %u, Name: %s\n", list[i].id, list[i].base_type_id, list[i].type_id,
					list[i].name);

				spvc_type		typeHandle				= spvc_compiler_get_type_handle(compiler_glsl, list[i].type_id);
				spvc_basetype	type					= spvc_type_get_basetype(typeHandle);
				uint			typeDimensionCount		= spvc_type_get_num_array_dimensions(typeHandle);
				printf("\t\tdim count = %d\n", typeDimensionCount);
				for (uint d = 0; d < typeDimensionCount; ++d)
				{
					SpvId dim = spvc_type_get_array_dimension(typeHandle, d);
					printf("\t\tdim = %d\n", dim);
				}

				spvc_type		baseTypeHandle			= spvc_compiler_get_type_handle(compiler_glsl, list[i].base_type_id);
				spvc_basetype	baseType				= spvc_type_get_basetype(baseTypeHandle);
				uint			baseTypeDimensionCount	= spvc_type_get_num_array_dimensions(baseTypeHandle);


				if (baseType == SPVC_BASETYPE_STRUCT)
				{
					int memberCount = spvc_type_get_num_member_types(baseTypeHandle);
					for (int m = 0; m < memberCount; ++m)
					{
						spvc_type_id	memberBaseTypeID		= spvc_type_get_member_type(baseTypeHandle, m);		
						spvc_type		memberBaseTypeHandle	= spvc_compiler_get_type_handle(compiler_glsl, memberBaseTypeID);
						spvc_basetype	memberBaseType			= spvc_type_get_basetype(memberBaseTypeHandle);
						printf("\tmtype id = %d\n", memberBaseType);
					}
				}
				printf("\tSet: %u, Binding: %u, buildInType : %u\n",
					spvc_compiler_get_decoration(compiler_glsl, list[i].id, SpvDecorationDescriptorSet),
					spvc_compiler_get_decoration(compiler_glsl, list[i].id, SpvDecorationBinding),
					spvc_compiler_get_decoration(compiler_glsl, list[i].id, SpvDecorationBuiltIn)
				);
			}
		}
		printf("\n================== end to print ==================\n");
	}

	spvc_context_release_allocations(context);
	spvc_context_destroy(context);
}

static int _getDescriptorCountFromShader(spvc_compiler compiler_glsl, spvc_type_id type_id)
{
	int				descriptorCount			= 0;
	spvc_type		typeHandle				= spvc_compiler_get_type_handle(compiler_glsl, type_id);
	spvc_basetype	type					= spvc_type_get_basetype(typeHandle);
	uint			typeDimensionCount		= spvc_type_get_num_array_dimensions(typeHandle);
	for (uint d = 0; d < typeDimensionCount; ++d)
	{
		SpvId dim = spvc_type_get_array_dimension(typeHandle, d);
		descriptorCount += dim;
	}
	if (descriptorCount == 0)
		descriptorCount = 1;

	return descriptorCount;
}


static void _getLayoutBindsFromShader(const char* bytes, const int bytesLen, SHADER_TYPE shaderType, const char* const filename, VkDescriptorSetLayoutBinding* layoutBinds, int* pLayoutBindCount, int layoutBindCapacity)
{
	const SpvId*					spirv			= (const SpvId*)bytes;
	size_t							word_count		= bytesLen / sizeof(SpvId);
	spvc_context					context			= NULL;
	spvc_parsed_ir					ir				= NULL;
	spvc_compiler					compiler_glsl	= NULL;
	spvc_compiler_options			options			= NULL;
	spvc_resources					resources		= NULL;
	const spvc_reflected_resource*	list			= NULL;
	const char*						spvc_result		= NULL;
	size_t							count			= 0;

	// Create context.
	spvc_context_create(&context);

	// Set debug callback.
	spvc_context_set_error_callback(context, sprvc_error_callback, NULL);

	// Parse the SPIR-V.
	spvc_context_parse_spirv(context, spirv, word_count, &ir);

	// Hand it off to a compiler instance and give it ownership of the IR.
	spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler_glsl);

	// Do some basic reflection.
	if (NULL != compiler_glsl)
	{
		spvc_compiler_create_shader_resources(compiler_glsl, &resources);

		spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, &list, &count);
		for (size_t i = 0; i < count; i++)
		{
			int& bindCount = *pLayoutBindCount;
			if (bindCount + 1 >= layoutBindCapacity) 
			{
				assert(false);
				return;
			}

			VkDescriptorSetLayoutBinding& bind = layoutBinds[bindCount];	
			memset(&bind, 0, sizeof(bind));
			bind.binding			= spvc_compiler_get_decoration(compiler_glsl, list[i].id, SpvDecorationBinding);
			bind.descriptorCount	= _getDescriptorCountFromShader(compiler_glsl, list[i].type_id);
			bind.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			bind.stageFlags			= _toShaderStageFlagBits(shaderType);
			bind.pImmutableSamplers	= NULL;

			++bindCount;
		}

		spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_SAMPLED_IMAGE, &list, &count);
		for (size_t i = 0; i < count; i++)
		{
			int& bindCount = *pLayoutBindCount;
			if (bindCount + 1 >= layoutBindCapacity) 
			{
				assert(false);
				return;
			}

			VkDescriptorSetLayoutBinding& bind = layoutBinds[bindCount];	
			memset(&bind, 0, sizeof(bind));
			bind.binding			= spvc_compiler_get_decoration(compiler_glsl, list[i].id, SpvDecorationBinding);
			bind.descriptorCount	= _getDescriptorCountFromShader(compiler_glsl, list[i].type_id);
			bind.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			bind.stageFlags			= _toShaderStageFlagBits(shaderType);
			bind.pImmutableSamplers	= NULL;

			++bindCount;
		}
	}

	spvc_context_release_allocations(context);
	spvc_context_destroy(context);
}


static VkShaderModule _createShaderFromCode(
	svkDevice&						device, 
	const char* const				code, 
	const int						codelen, 
	SHADER_TYPE						shaderType, 
	const char* const				filename, 
	VkDescriptorSetLayoutBinding*	layoutBinds, 
	int*							layoutBindCount, 
	int								layoutBindCapacity)
{
	shaderc_compiler_t				compiler	= device.shaderCompiler;
	shaderc_compilation_result_t	result		= shaderc_compile_into_spv(
		compiler,
		code,
		codelen,
		_toShadercType(shaderType),
		filename,
		"main",
		nullptr);

	shaderc_compilation_status status = shaderc_result_get_compilation_status(result);
	if (status != shaderc_compilation_status_success)
	{
		printf(shaderc_result_get_error_message(result));

		shaderc_result_release(result);

		return NULL;
	}
	const char*	bytes		= shaderc_result_get_bytes(result);
	const int	bytesLen	= shaderc_result_get_length(result);

	//__testSpirv_Cross(bytes, bytesLen, filename);
	_getLayoutBindsFromShader(bytes, bytesLen, shaderType, filename, layoutBinds, layoutBindCount, layoutBindCapacity);

	VkShaderModule shaderModule = _createShader(device.device, (uint*)bytes, bytesLen);
	shaderc_result_release(result);

	//shaderc_compiler_release(compiler);
	return shaderModule;
}

static VkShaderModule _createShaderFromFile(svkDevice& device, const char* const filename, SHADER_TYPE shaderType, VkDescriptorSetLayoutBinding* layoutBinds, int* layoutBindCount, int layoutBindCapacity)
{
#pragma warning(push)
#pragma warning(disable:4996)
	FILE* f = fopen(filename, "rb");
#pragma warning(pop)
	const int	BUF_SIZE	= 1024 * 1024;
	char*		buf			= new char[BUF_SIZE];
	memset(buf, 0, BUF_SIZE);
	int len = fread(buf, 1, BUF_SIZE, f);

	VkShaderModule shaderModule = _createShaderFromCode(device, buf, len, shaderType, filename, layoutBinds, layoutBindCount, layoutBindCapacity);
	fclose(f);
	delete[] buf;

	assert(NULL != shaderModule);
	return shaderModule;
}

VkRenderPass svkCreateRenderPass(VkDevice device, VkFormat format, VkFormat depthFormat)
{
	// render pass
	VkAttachmentDescription attachments[2];
	memset(attachments, 0, sizeof(attachments));
	attachments[0].flags			= 0;
	attachments[0].format			= format;
	attachments[0].samples			= VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	attachments[1].flags			= 0;
	attachments[1].format			= depthFormat;
	attachments[1].samples			= VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference;
	memclr(colorReference);
	colorReference.attachment		= 0;
	colorReference.layout			= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference;
	memclr(depthReference);
	depthReference.attachment		= 1;
	depthReference.layout			= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass;
	memclr(subpass);
	subpass.flags					= 0;
	subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount	= 0;
	subpass.pInputAttachments		= NULL;
	subpass.colorAttachmentCount	= 1;
	subpass.pColorAttachments		= &colorReference;
	subpass.pResolveAttachments		= NULL;
	subpass.pDepthStencilAttachment	= &depthReference;
	subpass.preserveAttachmentCount	= 0;
	subpass.pPreserveAttachments	= NULL;

	VkSubpassDependency subpassDep[2];
	memset(subpassDep, 0, sizeof(subpassDep));
	subpassDep[0].srcSubpass		= VK_SUBPASS_EXTERNAL;
	subpassDep[0].dstSubpass		= 0;
	subpassDep[0].srcStageMask		= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	subpassDep[0].dstStageMask		= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	subpassDep[0].srcAccessMask		= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	subpassDep[0].dstAccessMask		= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	subpassDep[0].dependencyFlags	= 0;

	subpassDep[1].srcSubpass		= VK_SUBPASS_EXTERNAL;
	subpassDep[1].dstSubpass		= 0;
	subpassDep[1].srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDep[1].dstStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDep[1].srcAccessMask		= 0;
	subpassDep[1].dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	subpassDep[1].dependencyFlags	= 0;

	VkRenderPassCreateInfo renderPassCreateInfo;
	memclr(renderPassCreateInfo);
	renderPassCreateInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pNext				= NULL;
    renderPassCreateInfo.flags				= 0;
    renderPassCreateInfo.attachmentCount	= 2;
    renderPassCreateInfo.pAttachments		= attachments;
    renderPassCreateInfo.subpassCount		= 1;
    renderPassCreateInfo.pSubpasses			= &subpass;
    renderPassCreateInfo.dependencyCount	= 2;
    renderPassCreateInfo.pDependencies		= subpassDep;

	VkRenderPass renderPass;
	VkResult err = vkCreateRenderPass(device, &renderPassCreateInfo, NULL, &renderPass);
	assert(!err);
	return renderPass;
}

VkFramebuffer svkCreateFrameBuffer(VkDevice device, VkRenderPass renderPass, VkImageView* attachments, const int attachmentCount, const uint32_t width, const uint32_t height)
{
	VkFramebuffer frameBuffer = NULL;
	VkFramebufferCreateInfo fbCreateInfo;
	memclr(fbCreateInfo);
	fbCreateInfo.sType				= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbCreateInfo.pNext				= NULL;
	fbCreateInfo.renderPass			= renderPass;
	fbCreateInfo.attachmentCount	= attachmentCount;
	fbCreateInfo.pAttachments		= attachments;
	fbCreateInfo.width				= width;
	fbCreateInfo.height				= height;
	fbCreateInfo.layers				= 1;

	VkResult err = vkCreateFramebuffer(device, &fbCreateInfo, NULL, &frameBuffer);
	assert(!err);
	return frameBuffer;
}

svkImage svkCreateImage(svkDevice& device, VkFormat format, const int width, const int height)
{
	svkImage image;
	memclr(image);
	VkResult err = VK_SUCCESS;

	image.format = format;

	////////////////////////////////////
	// depth buffer	
	////////////////////////////////////
	//const VkFormat depth_format = VK_FORMAT_D16_UNORM;
	//VkFormat depthFormat = VK_FORMAT_D16_UNORM;
	VkImageCreateInfo imageCreateInfo;
	memclr(imageCreateInfo);
	imageCreateInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext			= NULL;
	imageCreateInfo.imageType		= VK_IMAGE_TYPE_2D;
	imageCreateInfo.format			= format;
	imageCreateInfo.extent.width	= width;
	imageCreateInfo.extent.height	= height;
	imageCreateInfo.extent.depth	= 1;
	imageCreateInfo.mipLevels		= 1;
	imageCreateInfo.arrayLayers		= 1;
	imageCreateInfo.samples			= VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling			= VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage			= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageCreateInfo.flags			= 0;

	err = vkCreateImage(device.device, &imageCreateInfo, NULL, &image.image);
	assert(!err);

	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(device.device, image.image, &memReq);
	assert(!err);

	VkMemoryAllocateInfo allocInfo;
	memclr(allocInfo);
	allocInfo.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext				= NULL;
	allocInfo.allocationSize	= memReq.size;
	allocInfo.memoryTypeIndex	= _getMemoryTypeIndex(memReq.memoryTypeBits, device.memoryProperties.memoryTypes, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	assert(allocInfo.memoryTypeIndex != -1);

	err = vkAllocateMemory(device.device, &allocInfo, NULL, &image.memory);
	assert(!err);

	err = vkBindImageMemory(device.device, image.image, image.memory, 0);
	assert(!err);

	VkImageViewCreateInfo viewCreateInfo;
	memclr(viewCreateInfo);
	viewCreateInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.pNext			= NULL;
	viewCreateInfo.flags			= 0;
	viewCreateInfo.image			= VK_NULL_HANDLE;
	viewCreateInfo.viewType			= VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format			= format;
	viewCreateInfo.image			= image.image;
	viewCreateInfo.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_DEPTH_BIT;
	viewCreateInfo.subresourceRange.baseMipLevel	= 0;
	viewCreateInfo.subresourceRange.levelCount		= 1;
	viewCreateInfo.subresourceRange.baseArrayLayer	= 0;
	viewCreateInfo.subresourceRange.layerCount		= 1;
	err = vkCreateImageView(device.device, &viewCreateInfo, NULL, &image.imageView);
	assert(!err);

	return image;
}

void svkDestroyImage(svkDevice& device, svkImage& image)
{
	image.format = VK_FORMAT_UNDEFINED;
	vkDestroyImageView	(device.device, image.imageView, NULL);
	vkDestroyImage		(device.device, image.image, NULL);
	vkFreeMemory		(device.device, image.memory, NULL);
}

int svkCreateFrames(
	svkDevice&		device, 
	svkSwapchain&	swapchain, 
	VkImageView		depthImageView, 
	VkRenderPass	renderPass, 
	const int		width, 
	const int		height, 
	svkFrame*		outputFrames, 
	const int		outputFrameCapacity)
{
	assert(outputFrameCapacity >= swapchain.imageCount);
	assert(swapchain.imageCount <= svkSwapchain::MAX_IMAGE_COUNT);

	VkResult err = VK_SUCCESS;

	VkImage swapchainImages[svkSwapchain::MAX_IMAGE_COUNT] = { NULL };
	vkcheck( vkGetSwapchainImagesKHR(device.device, swapchain.swapchain, &swapchain.imageCount, swapchainImages) );

	int i = 0;
	for (; i < swapchain.imageCount && i < outputFrameCapacity; ++i)
	{
		svkFrame& frame = outputFrames[i];

		VkImageViewCreateInfo colorImageViewCreateInfo;
		memclr(colorImageViewCreateInfo);
		colorImageViewCreateInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		colorImageViewCreateInfo.pNext			= NULL;
		colorImageViewCreateInfo.flags			= 0;
		colorImageViewCreateInfo.image			= swapchainImages[i];
		colorImageViewCreateInfo.viewType		= VK_IMAGE_VIEW_TYPE_2D;
		colorImageViewCreateInfo.format			= swapchain.format;
		colorImageViewCreateInfo.components.r	= VK_COMPONENT_SWIZZLE_R;
		colorImageViewCreateInfo.components.g	= VK_COMPONENT_SWIZZLE_G;
		colorImageViewCreateInfo.components.b	= VK_COMPONENT_SWIZZLE_B;
		colorImageViewCreateInfo.components.a	= VK_COMPONENT_SWIZZLE_A;
		colorImageViewCreateInfo.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		colorImageViewCreateInfo.subresourceRange.baseMipLevel		= 0;
		colorImageViewCreateInfo.subresourceRange.levelCount		= 1;
		colorImageViewCreateInfo.subresourceRange.baseArrayLayer	= 0;
		colorImageViewCreateInfo.subresourceRange.layerCount		= 1;

		err = vkCreateImageView(device.device, &colorImageViewCreateInfo, NULL, &frame.imageView);
		assert(!err);

		VkImageView attachments[]				= { frame.imageView, depthImageView };
		frame.framebuffer						= svkCreateFrameBuffer	(device.device, renderPass, attachments, 2, width, height);
		frame.commandBuffer						= svkCreateCommandBuffer(device);

		VkFenceCreateInfo fenceCreateInfo;
		memclr(fenceCreateInfo);
		fenceCreateInfo.sType					= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext					= NULL;
		fenceCreateInfo.flags					= VK_FENCE_CREATE_SIGNALED_BIT;

		VkSemaphoreCreateInfo semaphoreCreateInfo;
		memclr(semaphoreCreateInfo);
		semaphoreCreateInfo.sType				= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext				= NULL;
		semaphoreCreateInfo.flags				= 0;

		vkcheck( vkCreateFence		(device.device, &fenceCreateInfo,		NULL, &frame.fence) );
		vkcheck( vkCreateSemaphore	(device.device, &semaphoreCreateInfo,	NULL, &frame.imageAcquireSemaphore) );
		vkcheck( vkCreateSemaphore	(device.device, &semaphoreCreateInfo,	NULL, &frame.drawCompleteSemaphore) );
	}
	return i;
}

void svkDestroyFrames(svkDevice& device, svkFrame* frames, const int frameCount)
{
	for (int i = 0; i < frameCount; ++i)
	{
		svkFrame& f = frames[i];

		if (NULL != f.imageView);
			vkDestroyImageView(device.device, f.imageView, NULL);

		if (NULL != f.framebuffer)
			vkDestroyFramebuffer(device.device, f.framebuffer, NULL);

		if (NULL != f.commandBuffer)
			vkFreeCommandBuffers(device.device, device.commandPool, 1, &f.commandBuffer);

		if (NULL != f.fence)
			vkDestroyFence(device.device, f.fence, NULL);

		if (NULL != f.imageAcquireSemaphore)
			vkDestroySemaphore(device.device, f.imageAcquireSemaphore, NULL);

		if (NULL != f.drawCompleteSemaphore)
			vkDestroySemaphore(device.device, f.drawCompleteSemaphore, NULL);

		memclr(f);
	}
}

VkInstance svkCreateInstance(bool enableValidationLayer)
{
	uint					allExtensionCount = 32;
	VkExtensionProperties	allExtensions[32];
	memset(allExtensions, 0, sizeof(allExtensions));
	vkcheck(vkEnumerateInstanceExtensionProperties(NULL, &allExtensionCount, allExtensions));

	const char*				extensions[64]			= { 0 };
	uint					extensionCount			= 0;
	bool					surfaceExtFound			= false;
	bool					platformSurfaceExtFound	= false;
	for (uint i = 0; i < allExtensionCount; ++i)
	{
		if (0 == strcmp(VK_KHR_SURFACE_EXTENSION_NAME, allExtensions[i].extensionName))
		{
			surfaceExtFound = true;
			extensions[extensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
		}
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		if (!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, allExtensions[i].extensionName)) {
			platformSurfaceExtFound = true;
			extensions[extensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
		}
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
		if (!strcmp(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, allExtensions[i].extensionName)) {
			platformSurfaceExtFound = true;
			extensions[extensionCount++] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
		}
#elif defined(VK_USE_PLATFORM_METAL_EXT)
		if (!strcmp(VK_EXT_METAL_SURFACE_EXTENSION_NAME, allExtensions[i].extensionName)) {
			platformSurfaceExtFound = true;
			extensions[extensionCount++] = VK_EXT_METAL_SURFACE_EXTENSION_NAME;
		}
#endif
		if (!strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, allExtensions[i].extensionName)) 
		{
			extensions[extensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		}
	}
	assert(surfaceExtFound && platformSurfaceExtFound);

	// instance
	VkApplicationInfo app;
	memclr(app);
	app.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pNext				= NULL;
	app.pApplicationName	= "app0";
	app.applicationVersion	= 0;
	app.pEngineName			= "engine0";
	app.engineVersion		= 0;
	app.apiVersion			= VK_API_VERSION_1_1;

	
	// check layers
	VkLayerProperties allLayers[32];
	uint32_t allLayerCount = 32;
	vkEnumerateInstanceLayerProperties(&allLayerCount, allLayers);

	char* enabledLayers[1] = { NULL };
	if (_isLayerExists(allLayers, allLayerCount, "VK_LAYER_KHRONOS_validation"))
		enabledLayers[0] = "VK_LAYER_KHRONOS_validation";
	else if (_isLayerExists(allLayers, allLayerCount, "VK_LAYER_LUNARG_standard_validation"))
		enabledLayers[0] = "VK_LAYER_LUNARG_standard_validation";

	VkInstanceCreateInfo createInfo;
	memclr(createInfo);
	createInfo.sType					= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo			= &app;
	if (enableValidationLayer)
	{
		createInfo.enabledLayerCount	= enabledLayers[0] == NULL ? 0 : 1;;
		createInfo.ppEnabledLayerNames	= enabledLayers;
	}
	createInfo.enabledExtensionCount	= extensionCount;
	createInfo.ppEnabledExtensionNames	= extensions;

	VkInstance inst;
	vkcheck(vkCreateInstance(&createInfo, NULL, &inst));

	return inst;
}

svkDevice svkCreateDevice(VkInstance inst)
{
	svkDevice _svkDevice;
	memclr(_svkDevice);

	VkPhysicalDevice gpu = _choosePhysicalDevice(inst);
	_svkDevice.gpu = gpu;
	vkGetPhysicalDeviceProperties(gpu, &_svkDevice.gpuProperties);

	uint queueCount = 16;
	VkQueueFamilyProperties queues[16];
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueCount, queues);
	assert(queueCount >= 1);
	_svkDevice.queueFamilyCount = queueCount;

	uint queueFamilyIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queueCount; i++)
	{
		if ((queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
			continue;
		queueFamilyIndex = i;
		break;
	}
	if (queueFamilyIndex == UINT32_MAX)
	{
		assert(false);
		return _svkDevice;
	}
	_svkDevice.queueFamilyIndex = queueFamilyIndex;

	float queuePriorities[1] = { 0.0 };
	VkDeviceQueueCreateInfo queueCreateInfos[2];
	memset(queueCreateInfos, 0, sizeof(queueCreateInfos));
	queueCreateInfos[0].sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfos[0].pNext				= NULL;
	queueCreateInfos[0].queueFamilyIndex	= queueFamilyIndex;
	queueCreateInfos[0].queueCount			= 1;
	queueCreateInfos[0].pQueuePriorities	= queuePriorities;
	queueCreateInfos[0].flags				= 0;

	const int				MAX_DEVICE_EXTENSION_COUNT	= 512;
	uint					allExtensionCount			= MAX_DEVICE_EXTENSION_COUNT;
	VkExtensionProperties	allExtensions[MAX_DEVICE_EXTENSION_COUNT];
	memset(allExtensions, 0, sizeof(allExtensions));
	
	vkcheck( vkEnumerateDeviceExtensionProperties(gpu, NULL, &allExtensionCount, allExtensions) );

	bool		swapchainExtFound						= false;
	const char*	extensions[MAX_DEVICE_EXTENSION_COUNT]	= { NULL };
	uint		extensionCount							= 0;
	for (uint32_t i = 0; i < allExtensionCount; i++)
	{
		if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, allExtensions[i].extensionName)) {
			swapchainExtFound = true;
			extensions[extensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
		}
	}

	VkDeviceCreateInfo deviceCreateInfo;
	memclr(deviceCreateInfo);
	deviceCreateInfo.sType						= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext						= NULL;
    deviceCreateInfo.flags						= 0;
    deviceCreateInfo.queueCreateInfoCount		= 1;
    deviceCreateInfo.pQueueCreateInfos			= queueCreateInfos;
    deviceCreateInfo.enabledLayerCount			= 0;
    deviceCreateInfo.ppEnabledLayerNames		= NULL;
    deviceCreateInfo.enabledExtensionCount		= extensionCount;
    deviceCreateInfo.ppEnabledExtensionNames	= extensions;
    deviceCreateInfo.pEnabledFeatures			= NULL;

	VkDevice device;
	vkcheck( vkCreateDevice(gpu, &deviceCreateInfo, NULL, &device) );
	_svkDevice.device = device;

	_svkDevice.shaderCompiler = shaderc_compiler_initialize();

	vkGetDeviceQueue(device, queueFamilyIndex, 0, &_svkDevice.queue);

	vkGetPhysicalDeviceMemoryProperties(gpu, &_svkDevice.memoryProperties);

	vkGetPhysicalDeviceFormatProperties(gpu, VK_FORMAT_R8G8B8A8_UNORM, &_svkDevice.formatProperties);
	return _svkDevice;
}

svkSurface svkCreateSurface(VkInstance inst, svkDevice& device, void* hinstance, void* hwnd)
{
	svkSurface surface;

	////////////////////////////////////
	// create device surface handle	
	////////////////////////////////////
	// Create a WSI surface for the window:
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	VkWin32SurfaceCreateInfoKHR createInfo;
	memclr(createInfo);
	createInfo.sType		= VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext		= NULL;
	createInfo.flags		= 0;
	createInfo.hinstance	= (HINSTANCE)hinstance;
	createInfo.hwnd			= (HWND)hwnd;

	VkSurfaceKHR deviceSurfaceHandle;
	vkcheck( vkCreateWin32SurfaceKHR(inst, &createInfo, NULL, &deviceSurfaceHandle) );

#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	VkAndroidSurfaceCreateInfoKHR createInfo;
	memclr(createInfo);
	createInfo.sType	= VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext	= NULL;
	createInfo.flags	= 0;
	createInfo.window	= (struct ANativeWindow *)(demo->window);

	err = vkCreateAndroidSurfaceKHR(demo->inst, &createInfo, NULL, &demo->surface);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
	VkMetalSurfaceCreateInfoEXT deviceSurfaceHandle;
	memclr(createInfo);
	deviceSurfaceHandle.sType	= VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
	deviceSurfaceHandle.pNext	= NULL;
	deviceSurfaceHandle.flags	= 0;
	deviceSurfaceHandle.pLayer	= caMetalLayer;

	err = vkCreateMetalSurfaceEXT(inst, &deviceSurfaceHandle, NULL, &surface);
#endif
	surface.surface = deviceSurfaceHandle;


	// Iterate over each queue to learn whether it supports presenting:
	VkBool32* supportsPresent = (VkBool32 *)malloc(device.queueFamilyCount * sizeof(VkBool32));
	for (uint32_t i = 0; i < device.queueFamilyCount; i++) 
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(device.gpu, i, surface.surface, &supportsPresent[i]);
	}
	assert(supportsPresent[device.queueFamilyIndex] == 1);
	free(supportsPresent);
	
	vkcheck( vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.gpu, surface.surface, &surface.capabilities) );
	surface.width = surface.capabilities.currentExtent.width;
	surface.height = surface.capabilities.currentExtent.height;

	return surface;
}

svkSwapchain svkCreateSwapchain(svkDevice& device, const svkSurface& surface, const svkSwapchain& oldSwapchain, int imageCount, bool deleteOldSwapchainHandle)
{
	svkSwapchain swapchain;
	VkResult err;

	if (surface.capabilities.currentExtent.width == 0xFFFFFFFF) 
		assert(false);

	uint formatCount = 32;
	VkSurfaceFormatKHR formats[32];
	err = vkGetPhysicalDeviceSurfaceFormatsKHR(device.gpu, surface.surface, &formatCount, formats);
	assert(formatCount > 0 && formats[0].format == VK_FORMAT_B8G8R8A8_UNORM);
	assert(!err);
	swapchain.format = formats[0].format;
	swapchain.colorSpace = formats[0].colorSpace;

	assert(surface.capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
	assert(surface.capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);

	uint32_t swapchainImagesCount = imageCount;
	assert(swapchainImagesCount >= surface.capabilities.minImageCount);
	assert(surface.capabilities.maxImageCount == 0 || swapchainImagesCount <= surface.capabilities.maxImageCount);

	VkSwapchainCreateInfoKHR swapchainCreateInfo;
	memset(&swapchainCreateInfo, 0, sizeof(swapchainCreateInfo));
	swapchainCreateInfo.sType					= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext					= NULL;
	swapchainCreateInfo.surface					= surface.surface;
	swapchainCreateInfo.minImageCount			= swapchainImagesCount;
	swapchainCreateInfo.imageFormat				= swapchain.format;
	swapchainCreateInfo.imageColorSpace			= formats[0].colorSpace;
	swapchainCreateInfo.imageExtent.width		= surface.width;
	swapchainCreateInfo.imageExtent.height		= surface.height;
	swapchainCreateInfo.imageUsage				= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.preTransform			= VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreateInfo.compositeAlpha			= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.imageArrayLayers		= 1;
	swapchainCreateInfo.imageSharingMode		= VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount	= 0;
	swapchainCreateInfo.pQueueFamilyIndices		= NULL;
	swapchainCreateInfo.presentMode				= VK_PRESENT_MODE_FIFO_KHR;
	swapchainCreateInfo.oldSwapchain			= oldSwapchain.swapchain;
	swapchainCreateInfo.clipped					= true;

	VkSwapchainKHR oldHandle = oldSwapchain.swapchain;
	err = vkCreateSwapchainKHR(device.device, &swapchainCreateInfo, NULL, &swapchain.swapchain);
	assert(!err);
	if (deleteOldSwapchainHandle && NULL != oldHandle)
		vkDestroySwapchainKHR(device.device, oldHandle, NULL);

	swapchain.imageCount = svkSwapchain::MAX_IMAGE_COUNT;
	VkImage swapchainImages[svkSwapchain::MAX_IMAGE_COUNT] = { NULL };
	vkcheck( vkGetSwapchainImagesKHR(device.device, swapchain.swapchain, &swapchain.imageCount, NULL) );

	return swapchain;
}

VkCommandPool svkCreateCommandPool(svkDevice& device, bool needResetIndividual)
{
	VkResult err;
	VkCommandPool commandPool;

	VkCommandPoolCreateInfo cmdPoolCreateInfo;
	memclr(cmdPoolCreateInfo);
	cmdPoolCreateInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCreateInfo.pNext				= NULL;
	cmdPoolCreateInfo.flags				= needResetIndividual ? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : 0;
	cmdPoolCreateInfo.queueFamilyIndex	= device.queueFamilyIndex;

	err = vkCreateCommandPool(device.device, &cmdPoolCreateInfo, NULL, &commandPool);
	assert(!err);

	return commandPool;
}

VkCommandBuffer svkCreateCommandBuffer(svkDevice& device)
{
	if (NULL == device.commandPool)
		device.commandPool = svkCreateCommandPool(device, true);
	VkCommandBuffer cb;
	svkCreateCommandBuffer(device, device.commandPool, true, 1, &cb);
	return cb;
}


void svkCreateCommandBuffer(svkDevice& device, VkCommandPool pool, bool isPrimary, int count, VkCommandBuffer* output)
{
	VkResult err;

	VkCommandBufferAllocateInfo allocInfo;
	memclr(allocInfo);
	allocInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext					= NULL;
	allocInfo.commandPool			= pool;
	allocInfo.level					= isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandBufferCount	= count;

	err = vkAllocateCommandBuffers(device.device, &allocInfo, output);
	assert(!err);
}

void svkBeginCommandBuffer(VkCommandBuffer cb, bool oneTime)
{
	VkCommandBufferBeginInfo cmdBufferBeginInfo;
	memclr(cmdBufferBeginInfo);
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	if (oneTime)
		cmdBufferBeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkcheck( vkBeginCommandBuffer(cb, &cmdBufferBeginInfo) );
}

void svkBeginSecondaryCommandBuffer(VkCommandBuffer& cb, const VkRenderPass& renderPass, const VkFramebuffer& framebuffer)
{
	VkCommandBufferInheritanceInfo inheritInfo = {};
	inheritInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO, inheritInfo.pNext = NULL;
	inheritInfo.renderPass				= renderPass;
	inheritInfo.subpass					= 0;
	inheritInfo.framebuffer				= framebuffer;
	inheritInfo.occlusionQueryEnable	= VK_FALSE;
	inheritInfo.queryFlags				= 0;
	inheritInfo.pipelineStatistics		= 0;

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType						= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext						= NULL;
	beginInfo.flags						= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	beginInfo.pInheritanceInfo			= &inheritInfo;
	vkcheck( vkBeginCommandBuffer(cb, &beginInfo) );
}


void _createSamplerAndImageView(svkDevice& device, svkTexture& _svkTexture, const VkFormat texFormat)
{
	VkResult err;

	VkSamplerCreateInfo samplerCreateInfo;
	memclr(samplerCreateInfo);
	samplerCreateInfo.sType							= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.pNext							= NULL;
	samplerCreateInfo.magFilter						= VK_FILTER_NEAREST;
	samplerCreateInfo.minFilter						= VK_FILTER_NEAREST;
	samplerCreateInfo.mipmapMode					= VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerCreateInfo.addressModeU					= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV					= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW					= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.mipLodBias					= 0.0f;
	samplerCreateInfo.anisotropyEnable				= VK_FALSE;
	samplerCreateInfo.maxAnisotropy					= 1;
	samplerCreateInfo.compareOp						= VK_COMPARE_OP_NEVER;
	samplerCreateInfo.minLod						= 0.0f;
	samplerCreateInfo.maxLod						= 0.0f;
	samplerCreateInfo.borderColor					= VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerCreateInfo.unnormalizedCoordinates		= VK_FALSE;

	VkImageViewCreateInfo viewCreateInfo;
	memclr(viewCreateInfo);
	viewCreateInfo.sType							= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.pNext							= NULL;
	viewCreateInfo.flags							= 0;
	viewCreateInfo.image							= VK_NULL_HANDLE;
	viewCreateInfo.viewType							= VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format							= texFormat;
	viewCreateInfo.components.r						= VK_COMPONENT_SWIZZLE_R;
	viewCreateInfo.components.g						= VK_COMPONENT_SWIZZLE_G;
	viewCreateInfo.components.b						= VK_COMPONENT_SWIZZLE_B;
	viewCreateInfo.components.a						= VK_COMPONENT_SWIZZLE_A;
	viewCreateInfo.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	viewCreateInfo.subresourceRange.baseMipLevel	= 0;
	viewCreateInfo.subresourceRange.levelCount		= 1;
	viewCreateInfo.subresourceRange.baseArrayLayer	= 0;
	viewCreateInfo.subresourceRange.layerCount		= 1;

	// sampler
	err = vkCreateSampler(device.device, &samplerCreateInfo, NULL, &_svkTexture.sampler);
	assert(!err);

	// view
	viewCreateInfo.image = _svkTexture.image;
	err = vkCreateImageView(device.device, &viewCreateInfo, NULL, &_svkTexture.view);
	assert(!err);
}

svkTexture svkCreateTexture(svkDevice& device, const char* const filename, VkCommandBuffer outCommandBuffer)
{
	const VkFormat texFormat = VK_FORMAT_R8G8B8A8_UNORM;

	svkTexture _svkTexture;
	memclr(_svkTexture);
	//VkResult err;

	_createTextureImageFromFile(device, filename, &_svkTexture, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Nothing in the pipeline needs to be complete to start, and don't allow fragment // shader to run until layout transition completes
	VkCommandBuffer commandBuffer = outCommandBuffer;
	if (NULL == outCommandBuffer)
	{
		commandBuffer = svkCreateCommandBuffer(device);
		svkBeginCommandBuffer(commandBuffer);
	}
	_setImageLayout(commandBuffer, _svkTexture.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, _svkTexture.imageLayout, (VkAccessFlagBits)0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	if (NULL == outCommandBuffer)
	{
		svkEndCommandBuffer(device, commandBuffer);
	}

	_createSamplerAndImageView(device, _svkTexture, texFormat);

	return _svkTexture;
}

svkTexture svkCreateTexture(svkDevice& device, const int width, const int height, VkCommandBuffer outCommandBuffer)
{
	const VkFormat texFormat = VK_FORMAT_R8G8B8A8_UNORM;

	svkTexture _svkTexture;

	_createTextureImageWithSize(device, width, height, &_svkTexture, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Nothing in the pipeline needs to be complete to start, and don't allow fragment // shader to run until layout transition completes
	VkCommandBuffer commandBuffer = outCommandBuffer;
	if (NULL == outCommandBuffer)
	{
		commandBuffer = svkCreateCommandBuffer(device);
		svkBeginCommandBuffer(commandBuffer);
	}
	_setImageLayout(commandBuffer, _svkTexture.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, _svkTexture.imageLayout, (VkAccessFlagBits)0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	if (NULL == outCommandBuffer)
	{
		svkEndCommandBuffer(device, commandBuffer);
	}

	_createSamplerAndImageView(device, _svkTexture, texFormat);

	return _svkTexture;
}

void svkCopyTexture(svkDevice& device, svkTexture& texture, const void* const data, const int sizeofData)
{
	assert(false && "not implemented!");

	//VkMemoryRequirements memReq;
	//vkGetImageMemoryRequirements(device.device, texture.image, &memReq);
	//VkResult err = VK_SUCCESS;
	//bool result = true;

	////if (requiredProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) 
	//{
	//	VkImageSubresource subres;
	//	memclr(subres);
	//	subres.aspectMask	= VK_IMAGE_ASPECT_COLOR_BIT;
	//	subres.mipLevel		= 0;
	//	subres.arrayLayer	= 0;
	//	VkSubresourceLayout layout;
	//	void* imageMem		= NULL;

	//	vkGetImageSubresourceLayout(device.device, texture.image, &subres, &layout);

	//	err = vkMapMemory(device.device, texture.memory, 0, sizeofData, 0, &imageMem);
	//	assert(!err);
	//	memcpy(imageMem, data, static_cast<size_t>(sizeofData));
	//	vkUnmapMemory(device.device, texture.memory);
	//}
	//texObj->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

void svkEndCommandBuffer(svkDevice& device, VkCommandBuffer commandBuffer)
{
	VkResult err;

	vkEndCommandBuffer(commandBuffer);

	VkFence fence;
	
	VkFenceCreateInfo fenceCreateInfo;
	memclr(fenceCreateInfo);
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.pNext = NULL;
	fenceCreateInfo.flags = 0;

	err = vkCreateFence(device.device, &fenceCreateInfo, NULL, &fence);
	assert(!err);

	const VkCommandBuffer cmdBufs[] = { commandBuffer };
	VkSubmitInfo submitInfo;
	memclr(submitInfo);
	submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext				= NULL;
	submitInfo.waitSemaphoreCount	= 0;
	submitInfo.pWaitSemaphores		= NULL;
	submitInfo.pWaitDstStageMask	= NULL;
	submitInfo.commandBufferCount	= 1;
	submitInfo.pCommandBuffers		= cmdBufs;
	submitInfo.signalSemaphoreCount	= 0;
	submitInfo.pSignalSemaphores	= NULL;

	err = vkQueueSubmit(device.queue, 1, &submitInfo, fence);
	assert(!err);

	err = vkWaitForFences(device.device, 1, &fence, VK_TRUE, UINT64_MAX);
	assert(!err);

	vkFreeCommandBuffers(device.device, device.commandPool, 1, cmdBufs);
	vkDestroyFence(device.device, fence, NULL);
}

svkBuffer svkCreateVertexBuffer(svkDevice& device, const void* data, const int dataSize)
{
	VkDeviceMemory	memory;
	VkBuffer		buffer = _genBuffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, dataSize, &memory);
	svkBuffer		svkbuf { buffer, memory }; 

	if (NULL != data)
		svkCopyBuffer(device, svkbuf, data, dataSize);

	return svkbuf;
}

svkBuffer svkCreateIndexBuffer(svkDevice& device, const void* data, const int dataSize)
{
	VkResult		err;
	VkDeviceMemory	memory;
	VkBuffer		buffer = _genBuffer(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, dataSize, &memory);

	void* mapmm = NULL;
	err = vkMapMemory(device.device, memory, 0, VK_WHOLE_SIZE, 0, &mapmm);
	assert(!err);
	memcpy(mapmm, data, dataSize);
	vkUnmapMemory(device.device, memory);

	return svkBuffer{ buffer, memory };
}

svkBuffer svkCreateUniformBuffer(svkDevice& device, void* data, const int dataSize)
{
	svkBuffer		_svkBuffer;
	VkDeviceMemory	memory;
	VkBuffer		buffer = _genBuffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, dataSize, &memory);

	_svkBuffer.buffer = buffer;
	_svkBuffer.memory = memory;
	if (NULL != data)
	{
		svkCopyBuffer(device, _svkBuffer, data, dataSize);
	}

	return _svkBuffer;
}

void svkCopyBuffer(svkDevice& device, svkBuffer& buffer, const void* data, const int dataSize)
{
	if (NULL == data)
		return;

	void* mapmm = svkMapBuffer(device, buffer);

	memcpy(mapmm, data, dataSize);

	svkUnmapBuffer(device, buffer);
}

void* svkMapBuffer(svkDevice& device, svkBuffer& buffer)
{
	VkResult	err;
	void*		mapmm = NULL;
	err = vkMapMemory(device.device, buffer.memory, 0, VK_WHOLE_SIZE, 0, &mapmm);
	assert(!err);
	return mapmm;
}

void svkUnmapBuffer(svkDevice& device, svkBuffer& buffer)
{
	if (NULL == device.device)
		return;
	if (NULL == buffer.memory)
		return;
	vkUnmapMemory(device.device, buffer.memory);
}

VkDescriptorSetLayout svkCreateDescriptorLayout(svkDevice& device)
{
	VkDescriptorSetLayoutBinding layoutBindings[2];
	memset(layoutBindings, 0, sizeof(layoutBindings));
	layoutBindings[0].binding				= 0;
    layoutBindings[0].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[0].descriptorCount		= 1;
    layoutBindings[0].stageFlags			= VK_SHADER_STAGE_VERTEX_BIT;
    layoutBindings[0].pImmutableSamplers	= NULL;

	layoutBindings[1].binding				= 1;
    layoutBindings[1].descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layoutBindings[1].descriptorCount		= 1;
    layoutBindings[1].stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
    layoutBindings[1].pImmutableSamplers	= NULL;


	VkDescriptorSetLayoutCreateInfo descriptorLayoutCreateInfo;
	memclr(descriptorLayoutCreateInfo);
    descriptorLayoutCreateInfo.sType		= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayoutCreateInfo.pNext		= NULL;
    descriptorLayoutCreateInfo.flags		= 0;
    descriptorLayoutCreateInfo.bindingCount	= 2;
    descriptorLayoutCreateInfo.pBindings	= layoutBindings;

	VkResult err;
	VkDescriptorSetLayout layout;
	err = vkCreateDescriptorSetLayout(device.device, &descriptorLayoutCreateInfo, NULL, &layout);
	assert(!err);

	return layout;
}



VkDescriptorSetLayout svkCreateDescriptorLayoutEx(svkDevice& device, const VkDescriptorSetLayoutBinding* layoutBindings, const int bindCount)
{
	VkDescriptorSetLayoutCreateInfo descriptorLayoutCreateInfo;
	memclr(descriptorLayoutCreateInfo);
	descriptorLayoutCreateInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutCreateInfo.pNext			= NULL;
	descriptorLayoutCreateInfo.flags			= 0;
	descriptorLayoutCreateInfo.bindingCount		= bindCount;
	descriptorLayoutCreateInfo.pBindings		= layoutBindings;

	VkResult err;
	VkDescriptorSetLayout layout;
	err = vkCreateDescriptorSetLayout(device.device, &descriptorLayoutCreateInfo, NULL, &layout);
	assert(!err);

	return layout;
}

VkDescriptorPool svkCreateDescriptorPoolEx(svkDevice& device, const unsigned int size, const VkDescriptorSetLayoutBinding* binds, const int bindCount) 
{
	VkDescriptorPoolSize poolSizes[64];
	memset(poolSizes, 0, sizeof(poolSizes));

	for (int i = 0; i < bindCount; ++i)
	{
		poolSizes[i].type				= binds[i].descriptorType;
		poolSizes[i].descriptorCount	= size * binds[i].descriptorCount;
	}

	VkDescriptorPoolCreateInfo poolCreateInfo;
	memclr(poolCreateInfo);
	poolCreateInfo.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.pNext				= NULL;
	poolCreateInfo.flags				= 0;
	poolCreateInfo.maxSets				= size;
	poolCreateInfo.poolSizeCount		= bindCount;
	poolCreateInfo.pPoolSizes			= poolSizes;

	VkDescriptorPool pool;
	vkcheck( vkCreateDescriptorPool(device.device, &poolCreateInfo, NULL, &pool) );
	return pool;
}


VkDescriptorPool svkCreateDescriptorPoolEx2(svkDevice& device, const int poolSize, const int* countPerType) 
{
	VkDescriptorPoolSize poolSizes[64];
	memset(poolSizes, 0, sizeof(poolSizes));

	int sizeCount = 0;
	for (int i = VK_DESCRIPTOR_TYPE_BEGIN_RANGE; i <= VK_DESCRIPTOR_TYPE_END_RANGE; ++i)
	{
		if (countPerType[i] <= 0)
			continue;
		poolSizes[sizeCount].type				= static_cast<VkDescriptorType>(i);
		poolSizes[sizeCount].descriptorCount	= poolSize * countPerType[i];
		++sizeCount;
	}

	VkDescriptorPoolCreateInfo poolCreateInfo;
	memclr(poolCreateInfo);
	poolCreateInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.pNext			= NULL;
	poolCreateInfo.flags			= 0; //VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
	poolCreateInfo.maxSets			= poolSize; // TODO should be poolSize * sizeCount ???
	poolCreateInfo.poolSizeCount	= sizeCount;
	poolCreateInfo.pPoolSizes		= poolSizes;

	VkDescriptorPool pool;
	vkcheck( vkCreateDescriptorPool(device.device, &poolCreateInfo, NULL, &pool) );
	return pool;
}

VkDescriptorSet svkCreateDescriptorSet(
	svkDevice&				device, 
	VkDescriptorPool		pool, 
	VkDescriptorSetLayout	layout, 
	VkBuffer				uniformBuffer,
	const int				uniformCount, 
	const int				sizeofUniform, 
	const int				textureCount, 
	svkTexture*				textures) 
{
	assert(textureCount < 64);
	VkResult err;

	VkDescriptorSetAllocateInfo allocInfo;
	memclr(allocInfo);
	allocInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext					= NULL;
    allocInfo.descriptorPool		= pool;
    allocInfo.descriptorSetCount	= 1;
    allocInfo.pSetLayouts			= &layout;

	VkDescriptorBufferInfo bufferInfo;
	memclr(bufferInfo);
	bufferInfo.offset				= 0;
	bufferInfo.range				= sizeofUniform;

	VkDescriptorImageInfo texDescs[64];
	memset(&texDescs, 0, sizeof(texDescs));
	for (int i = 0; i < textureCount ; i++) 
	{
		texDescs[i].sampler			= textures[i].sampler;
		texDescs[i].imageView		= textures[i].view;
		texDescs[i].imageLayout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	VkWriteDescriptorSet writes[2];
	memset(&writes, 0, sizeof(writes));
	writes[0].sType					= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].descriptorCount		= 1;
	writes[0].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].pBufferInfo			= &bufferInfo;

	writes[1].sType					= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[1].dstBinding			= 1;
	writes[1].descriptorCount		= textureCount;
	writes[1].descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writes[1].pImageInfo			= texDescs;

	VkDescriptorSet descriptorSet;
	err = vkAllocateDescriptorSets(device.device, &allocInfo, &descriptorSet);
	assert(!err);
	bufferInfo.buffer				= uniformBuffer;
	writes[0].dstSet				= descriptorSet;
	writes[1].dstSet				= descriptorSet;
	vkUpdateDescriptorSets(device.device, 2, writes, 0, NULL);
	return descriptorSet;
}

VkDescriptorSet svkCreateDescriptorSet(svkDevice& device, svkDescriptorCreator& creator, VkBuffer uniformBuffer, const int uniformCount, const int sizeofUniform, const int textureCount, svkTexture* textures)
{
	return svkCreateDescriptorSet(device, creator.pool, creator.layout, uniformBuffer, uniformCount, sizeofUniform, textureCount, textures);
}

void svkRefreshSurfaceSize(svkDevice& device,svkSurface& surface)
{
	vkcheck( vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.gpu, surface.surface, &surface.capabilities) );
	surface.width	= surface.capabilities.currentExtent.width;
	surface.height	= surface.capabilities.currentExtent.height;
}

svkPipeline svkCreatePipeline(svkDevice& device, VkDescriptorSetLayout descLayout, VkRenderPass renderPass, VkPipelineCache* oldPipelineCache)
{
	svkPipeline pipeline;
	VkResult err;

	VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
	VkPipelineDynamicStateCreateInfo dynamicState;
	memclr(dynamicStateEnables);
	memclr(dynamicState);
	dynamicState.sType			= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates	= dynamicStateEnables;

	//vertex input
	VkVertexInputBindingDescription viBinding;
	memclr(viBinding);
	viBinding.binding			= 0;
	viBinding.stride			= sizeof(example_vertex);
	viBinding.inputRate			= VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription viAttribs[2];
	memset(viAttribs, 0, sizeof(viAttribs));
	viAttribs[0].location		= 0;
	viAttribs[0].binding		= 0;
	viAttribs[0].format			= VK_FORMAT_R32G32B32A32_SFLOAT;
	viAttribs[0].offset			= 0;

	viAttribs[1].location		= 1;
	viAttribs[1].binding		= 0;
	viAttribs[1].format			= VK_FORMAT_R32G32_SFLOAT;
	viAttribs[1].offset			= (uint32_t)(uintptr_t)offset(example_vertex, texcoord);

	VkPipelineVertexInputStateCreateInfo viCreateInfo;
	memclr(viCreateInfo);
	viCreateInfo.sType								= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	viCreateInfo.vertexBindingDescriptionCount		= 1;
	viCreateInfo.pVertexBindingDescriptions			= &viBinding;
	viCreateInfo.vertexAttributeDescriptionCount	= 2;
	viCreateInfo.pVertexAttributeDescriptions		= viAttribs;

	VkPipelineInputAssemblyStateCreateInfo iaCreateInfo;
	memclr(iaCreateInfo);
	iaCreateInfo.sType			= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	iaCreateInfo.topology		= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineRasterizationStateCreateInfo rs;
	memclr(rs);
	rs.sType					= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rs.polygonMode				= VK_POLYGON_MODE_FILL;
	rs.cullMode					= VK_CULL_MODE_BACK_BIT;
	rs.frontFace				= VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rs.depthClampEnable			= VK_FALSE;
	rs.rasterizerDiscardEnable	= VK_FALSE;
	rs.depthBiasEnable			= VK_FALSE;
	rs.lineWidth				= 1.0f;

	VkPipelineColorBlendStateCreateInfo cb;
	memclr(cb);
	cb.sType					= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	VkPipelineColorBlendAttachmentState attState[1];
	memclr(attState);
	attState[0].colorWriteMask	= 0xf;
	attState[0].blendEnable		= VK_FALSE;
	cb.attachmentCount			= 1;
	cb.pAttachments				= attState;

	VkPipelineViewportStateCreateInfo vp;
	memclr(vp);
	vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.viewportCount			= 1;
	dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	vp.scissorCount				= 1;
	dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

	VkPipelineDepthStencilStateCreateInfo ds;
	memclr(ds);
	ds.sType					= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable			= VK_TRUE;
	ds.depthWriteEnable			= VK_TRUE;
	ds.depthCompareOp			= VK_COMPARE_OP_LESS_OR_EQUAL;
	ds.depthBoundsTestEnable	= VK_FALSE;
	ds.back.failOp				= VK_STENCIL_OP_KEEP;
	ds.back.passOp				= VK_STENCIL_OP_KEEP;
	ds.back.compareOp			= VK_COMPARE_OP_ALWAYS;
	ds.stencilTestEnable		= VK_FALSE;
	ds.front = ds.back;

	VkPipelineMultisampleStateCreateInfo ms;
	memclr(ms);
	ms.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.pSampleMask				= NULL;
	ms.rasterizationSamples		= VK_SAMPLE_COUNT_1_BIT;

	VkShaderModule vertShader	= _createShaderFromFile(device, "1.vert", SHADER_TYPE_VERT, NULL, NULL, 0);
	VkShaderModule fragShader	= _createShaderFromFile(device, "1.frag", SHADER_TYPE_FRAG, NULL, NULL, 0);

	//// Two stages: vs and fs
	VkPipelineShaderStageCreateInfo shaderStages[2];
	memset(&shaderStages, 0, 2 * sizeof(VkPipelineShaderStageCreateInfo));

	shaderStages[0].sType		= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage		= VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module		= vertShader;
	shaderStages[0].pName		= "main";

	shaderStages[1].sType		= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage		= VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module		= fragShader;
	shaderStages[1].pName		= "main";

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	memclr(pipelineLayoutCreateInfo);
	pipelineLayoutCreateInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pNext					= NULL;
    pipelineLayoutCreateInfo.flags					= 0;
    pipelineLayoutCreateInfo.setLayoutCount			= 1;
    pipelineLayoutCreateInfo.pSetLayouts			= &descLayout;
    pipelineLayoutCreateInfo.pushConstantRangeCount	= 0;
    pipelineLayoutCreateInfo.pPushConstantRanges	= NULL;

	err = vkCreatePipelineLayout(device.device, &pipelineLayoutCreateInfo, NULL, &pipeline.layout);
	assert(!err);

	VkPipelineCache pipelineCache = NULL;
	if (NULL != oldPipelineCache)
	{
		if (*oldPipelineCache == NULL)
		{
			VkPipelineCacheCreateInfo pipelineCacheCreateInfo;
			memclr(pipelineCacheCreateInfo);
			pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
			err = vkCreatePipelineCache(device.device, &pipelineCacheCreateInfo, NULL, &pipelineCache);
			assert(!err);
		}
		else
			pipelineCache = *oldPipelineCache;
	}
	pipeline.cache = pipelineCache;

	VkGraphicsPipelineCreateInfo pipelineCreateInfo;
	memclr(pipelineCreateInfo);
	pipelineCreateInfo.sType				= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.layout				= pipeline.layout;
	pipelineCreateInfo.pVertexInputState	= &viCreateInfo;
	pipelineCreateInfo.pInputAssemblyState	= &iaCreateInfo;
	pipelineCreateInfo.pRasterizationState	= &rs;
	pipelineCreateInfo.pColorBlendState		= &cb;
	pipelineCreateInfo.pMultisampleState	= &ms;
	pipelineCreateInfo.pViewportState		= &vp;
	pipelineCreateInfo.pDepthStencilState	= &ds;
	pipelineCreateInfo.stageCount			= countof(shaderStages);
	pipelineCreateInfo.pStages				= shaderStages;
	pipelineCreateInfo.renderPass			= renderPass;
	pipelineCreateInfo.pDynamicState		= &dynamicState;

	err = vkCreateGraphicsPipelines(device.device, pipelineCache, 1, &pipelineCreateInfo, NULL, &pipeline.pipeline);
	assert(!err);

	vkDestroyShaderModule(device.device, fragShader, NULL);
	vkDestroyShaderModule(device.device, vertShader, NULL);
	return pipeline;
}

void svkCmdBeginRenderPass(VkCommandBuffer cb, float clearColorR, float clearColorG, float clearColorB, float clearColorA, float depth, unsigned int stencil, VkRenderPass renderPass, VkFramebuffer framebuffer, uint32_t width, uint32_t height, bool useSecondaryCommandBuffer)
{
	VkClearValue clearValues[2];
	memset(clearValues, 0, sizeof(clearValues));
	float rs[4] = {clearColorR, clearColorG, clearColorB, clearColorA};
	memcpy(clearValues[0].color.float32, rs, sizeof(rs));
	clearValues[1].depthStencil.depth	= depth;
	clearValues[1].depthStencil.stencil = stencil;

	VkRenderPassBeginInfo renderPassBeginInfo;
	memclr(renderPassBeginInfo);
	renderPassBeginInfo.sType						= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext						= NULL;
	renderPassBeginInfo.renderPass					= renderPass;
	renderPassBeginInfo.framebuffer					= framebuffer;
	renderPassBeginInfo.renderArea.offset.x			= 0;
	renderPassBeginInfo.renderArea.offset.y			= 0;
	renderPassBeginInfo.renderArea.extent.width		= width;
	renderPassBeginInfo.renderArea.extent.height	= height;
	renderPassBeginInfo.clearValueCount				= 2;
	renderPassBeginInfo.pClearValues				= clearValues;

	VkSubpassContents contents = useSecondaryCommandBuffer ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : VK_SUBPASS_CONTENTS_INLINE;
	vkCmdBeginRenderPass(cb, &renderPassBeginInfo, contents);
}

void svkCmdSetViewPortCubic(VkCommandBuffer cb, uint32_t width, uint32_t height)
{
	VkViewport viewport;
	memset(&viewport, 0, sizeof(viewport));
	float viewportDimension;
	if (width < height) 
	{
		viewportDimension	= (float)width;
		viewport.y			= (height - width) / 2.0f;
	} 
	else 
	{
		viewportDimension	= (float)height;
		viewport.x			= (width - height) / 2.0f;
	}
	viewport.height			= viewportDimension;
	viewport.width			= viewportDimension;
	viewport.minDepth		= (float)0.0f;
	viewport.maxDepth		= (float)1.0f;
	vkCmdSetViewport(cb, 0, 1, &viewport);
}

void svkCmdSetViewPortByGLParams(VkCommandBuffer cb, const int x, const int y, const int width, const int height)
{
	svkCmdSetViewPortDirectly(cb, x, y + height, width, -height);
}

void svkCmdSetViewPortDirectly(VkCommandBuffer cb, const int x, const int y, const int width, const int height)
{
	VkViewport viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.x				= (float)x;
	viewport.y				= (float)y;
	viewport.height			= (float)height;
	viewport.width			= (float)width;
	viewport.minDepth		= (float)0.0f;
	viewport.maxDepth		= (float)1.0f;
	vkCmdSetViewport(cb, 0, 1, &viewport);
}

void svkCmdBindVertexBuffers(VkCommandBuffer cb, int firstBinding, void** vertexBuffers, int attrCount)
{
	VkDeviceSize	offsets[64]			= { 0 };
	VkBuffer		vkVertexBuffers[64] = { 0 };
	int				vkVertexBufferCount = 0;
	for (int i = 0; i < attrCount; ++i)
	{
		svkBuffer* svkBuf= static_cast<svkBuffer*>(vertexBuffers[i]);
		if (NULL == svkBuf)
			continue;
		vkVertexBuffers[vkVertexBufferCount] = svkBuf->buffer;
		offsets[vkVertexBufferCount] = 0;
		++vkVertexBufferCount;
	}
	vkCmdBindVertexBuffers(cb, firstBinding, vkVertexBufferCount, vkVertexBuffers, offsets);                
}

void svkCmdSetScissor(VkCommandBuffer cb, uint32_t width, uint32_t height)
{
	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	scissor.extent.width	= width;
	scissor.extent.height	= height;
	scissor.offset.x		= 0;
	scissor.offset.y		= 0;
	vkCmdSetScissor(cb, 0, 1, &scissor);
}

int svkAcquireNextImage(svkDevice& device, svkSwapchain& swapchain, svkFrame* frames, const int frame, void* userData, presentResultCallback callback)
{
	VkResult	err;
	uint32_t	nextFrame		= -1;

	vkWaitForFences	(device.device, 1, &frames[frame].fence, VK_TRUE, UINT64_MAX);
	do 
	{
		err = vkAcquireNextImageKHR(
				device.device, 
				swapchain.swapchain, 
				UINT64_MAX,
				frames[frame].imageAcquireSemaphore, 
				VK_NULL_HANDLE, 
				&nextFrame);
		if (err != VK_SUCCESS)
		{
			if (NULL != callback)
				callback(userData, err);
		}
	} while (err != VK_SUCCESS);

	return (int)nextFrame;
}

void svkQueueSubmit(svkDevice& device, const VkCommandBuffer* commandBuffers, const int commandBufferCount, VkSemaphore& waitSemaphore, VkSemaphore& signalSemaphore, VkFence& fence)
{
	vkResetFences(device.device, 1, &fence);

	// Wait for the image acquired semaphore to be signaled to ensure
	// that the image won't be rendered to until the presentation
	// engine has fully released ownership to the application, and it is
	// okay to render to the image.
	VkPipelineStageFlags pipeStageFlags;
	VkSubmitInfo submitInfo;
	memclr(submitInfo);
	submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext				= NULL;
	submitInfo.pWaitDstStageMask	= &pipeStageFlags;
	pipeStageFlags					= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.waitSemaphoreCount	= 1;
	submitInfo.pWaitSemaphores		= &waitSemaphore; //&frames[prevFrame].imageAcquireSemaphore;
	submitInfo.commandBufferCount	= commandBufferCount;
	submitInfo.pCommandBuffers		= commandBuffers;
	submitInfo.signalSemaphoreCount	= 1;
	submitInfo.pSignalSemaphores	= &signalSemaphore;	//&frames[frame].drawCompleteSemaphore;
	VkResult err = vkQueueSubmit(device.queue, 1, &submitInfo, fence);
	assert(!err);
}


void svkQueueSubmitFrame(svkDevice& device, svkFrame* frames, const int frame, const int prevFrame)
{
	svkQueueSubmit(
		device, 
		&frames[frame].commandBuffer, 
		1, 
		frames[prevFrame].imageAcquireSemaphore,  
		frames[frame].drawCompleteSemaphore, 
		frames[frame].fence);
}

void svkPresent(svkDevice& device, svkSwapchain& swapchain, svkFrame* frames, const int frame,  void* userData, presentResultCallback callback)
{
	// If we are using separate queues we have to wait for image ownership,
	// otherwise wait for draw complete
	VkPresentInfoKHR present;
	memclr(present);
	present.sType					= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.pNext					= NULL;
	present.waitSemaphoreCount		= 1;
	present.pWaitSemaphores			= &frames[frame].drawCompleteSemaphore;
	present.swapchainCount			= 1;
	present.pSwapchains				= &swapchain.swapchain;
	present.pImageIndices			= (uint32_t*)&frame;

	VkResult err = vkQueuePresentKHR(device.queue, &present);
	if (err != VK_SUCCESS)
	{
		if (NULL != callback)
			callback(userData, err);
	}
}

//void svkSubmitAndPresent(svkDevice& device, svkSwapchain& swapchain, const int frameLagIndex, void* userData, presentResultCallback callback)
//{
//	VkResult err;
//
//	uint32_t currentSwapchainIndex = 0;
//
//	// Ensure no more than FRAME_LAG renderings are outstanding
//	vkWaitForFences	(device.device, 1, &swapchain.fences[frameLagIndex], VK_TRUE, UINT64_MAX);
//	vkResetFences	(device.device, 1, &swapchain.fences[frameLagIndex]);
//
//	do {
//		// Get the index of the next available swapchain image:
//		err = vkAcquireNextImageKHR(
//				device.device, 
//				swapchain.swapchain, 
//				UINT64_MAX,
//				swapchain.imageAcquireSemaphores[frameLagIndex], 
//				VK_NULL_HANDLE, 
//				&currentSwapchainIndex);
//		if (err != VK_SUCCESS)
//		{
//			if (NULL != callback)
//				callback(userData, err);
//		}
//	} while (err != VK_SUCCESS);
//
//	// Wait for the image acquired semaphore to be signaled to ensure
//	// that the image won't be rendered to until the presentation
//	// engine has fully released ownership to the application, and it is
//	// okay to render to the image.
//	VkPipelineStageFlags pipeStageFlags;
//	VkSubmitInfo submitInfo;
//	memclr(submitInfo);
//	submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
//	submitInfo.pNext				= NULL;
//	submitInfo.pWaitDstStageMask	= &pipeStageFlags;
//	pipeStageFlags					= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	submitInfo.waitSemaphoreCount	= 1;
//	submitInfo.pWaitSemaphores		= &swapchain.imageAcquireSemaphores[frameLagIndex];
//	submitInfo.commandBufferCount	= 1;
//	submitInfo.pCommandBuffers		= &swapchain.commandBuffers[currentSwapchainIndex];
//	submitInfo.signalSemaphoreCount	= 1;
//	submitInfo.pSignalSemaphores	= &swapchain.drawCompleteSemaphores[frameLagIndex];
//	err = vkQueueSubmit(device.queue, 1, &submitInfo, swapchain.fences[frameLagIndex]);
//	assert(!err);
//
//	// If we are using separate queues we have to wait for image ownership,
//	// otherwise wait for draw complete
//	VkPresentInfoKHR present;
//	memclr(present);
//	present.sType					= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//	present.pNext					= NULL;
//	present.waitSemaphoreCount		= 1;
//	present.pWaitSemaphores			= &swapchain.drawCompleteSemaphores[frameLagIndex];
//	present.swapchainCount			= 1;
//	present.pSwapchains				= &swapchain.swapchain;
//	present.pImageIndices			= &currentSwapchainIndex;
//
//	err = vkQueuePresentKHR(device.queue, &present);
//	if (err != VK_SUCCESS)
//	{
//		if (NULL != callback)
//			callback(userData, err);
//	}
//}

svkDescriptorCreator svkCreateDescriptorCreatorEx(svkDevice& device, const unsigned int poolSize, const VkDescriptorSetLayoutBinding* binds, const int bindCount)
{
	svkDescriptorCreator creator;
	memclr(creator);
	creator.layout	= svkCreateDescriptorLayoutEx	(device, binds, bindCount);
	creator.pool	= svkCreateDescriptorPoolEx		(device, poolSize, binds, bindCount);
	return creator;
}

void svkDestroySwapchain(svkDevice& device, svkSwapchain& swapchain, bool deleteSelfHandle)
{
	if (NULL == device.device)
		return;

	swapchain.imageCount	= 0;
	swapchain.format		= VK_FORMAT_UNDEFINED;
	swapchain.colorSpace	= VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	if (deleteSelfHandle)
		vkDestroySwapchainKHR(device.device, swapchain.swapchain, NULL);
}

void svkDestroyTexture(svkDevice& device, svkTexture& texture)
{
	vkDestroyImageView	(device.device, texture.view, NULL);
	vkDestroyImage		(device.device, texture.image, NULL);
	vkFreeMemory		(device.device, texture.memory, NULL);
	vkDestroySampler	(device.device, texture.sampler, NULL);
}

void svkDestroySurface(VkInstance inst, svkDevice& device, svkSurface& surface)
{
	if (NULL == device.device || NULL == inst)
		return;
	vkDestroySurfaceKHR(inst, surface.surface, NULL);
}

void svkWaitQueue(svkDevice& device)
{
	vkQueueWaitIdle(device.queue);
}

void svkDestroyBuffer(svkDevice& device, svkBuffer& buffer)
{
	vkDestroyBuffer	(device.device, buffer.buffer, NULL);
	buffer.buffer = NULL;
	vkFreeMemory	(device.device, buffer.memory, NULL);
	buffer.memory = NULL;
}

void svkDestroyPipeline(svkDevice& device, svkPipeline& pipeline, bool deleteCache)
{
	vkDestroyPipeline		(device.device, pipeline.pipeline, NULL);
	vkDestroyPipelineLayout	(device.device, pipeline.layout, NULL);
	if (deleteCache)
		vkDestroyPipelineCache(device.device, pipeline.cache, NULL);
}

void svkDestroyDescriptorCreator(svkDevice& device, svkDescriptorCreator& descriptorCreator)
{
	if (NULL == device.device)
		return;
	vkDestroyDescriptorPool		(device.device, descriptorCreator.pool, NULL);
	vkDestroyDescriptorSetLayout(device.device, descriptorCreator.layout, NULL);
}

void svkDestroyDevice(svkDevice& device)
{
	if (NULL == device.device)
		return;
	shaderc_compiler_release	(device.shaderCompiler);
	vkDeviceWaitIdle			(device.device);
	vkDestroyCommandPool		(device.device, device.commandPool, NULL);
	vkDestroyDevice				(device.device, NULL);
}

svkShaderProgram svkCreateShaderProgramFromCode(
	svkDevice&						device,
	const char* const				vertCode, 
	const char* const				tcsCode, 
	const char* const				tesCode, 
	const char* const				geoCode, 
	const char* const				fragCode, 
	const char* const				compCode)
{
	svkShaderProgram prog;
	memclr(prog);

	const int MAX_LAYOUT_BIND = 128;
	VkDescriptorSetLayoutBinding binds[MAX_LAYOUT_BIND] = { 0 };
	int bindCount = 0;

	if (NULL != vertCode && 0 != vertCode[0])
		prog.vert	= _createShaderFromCode(device, vertCode, strlen(vertCode), SHADER_TYPE_VERT, "[vert_code]", binds, &bindCount, countof(binds));
	if (NULL != tcsCode && 0 != tcsCode[0])
		prog.tcs	= _createShaderFromCode(device, tcsCode, strlen(tcsCode), SHADER_TYPE_TCS, "[tcs_code]", binds, &bindCount, countof(binds));
	if (NULL != tesCode && 0 != tesCode[0])
		prog.tes	= _createShaderFromCode(device, tesCode, strlen(tesCode), SHADER_TYPE_TES, "[tes_code]", binds, &bindCount, countof(binds));
	if (NULL != geoCode && 0 != geoCode[0])
		prog.geo	= _createShaderFromCode(device, geoCode, strlen(geoCode), SHADER_TYPE_GEO, "[geo_code]", binds, &bindCount, countof(binds));
	if (NULL != fragCode && 0 != fragCode[0])
		prog.frag	= _createShaderFromCode(device, fragCode, strlen(fragCode), SHADER_TYPE_FRAG, "[frag_code]", binds, &bindCount, countof(binds));
	if (NULL != compCode && 0 != compCode[0])
		prog.comp	= _createShaderFromCode(device, compCode, strlen(compCode), SHADER_TYPE_COMP, "[comp_code]", binds, &bindCount, countof(binds));

	if (bindCount > 0)
	{
		assert(NULL == prog.layoutBinds);
		prog.layoutBinds = new VkDescriptorSetLayoutBinding[bindCount];
		memcpy(prog.layoutBinds, binds, sizeof(VkDescriptorSetLayoutBinding) * bindCount);
		prog.layoutBindCount = bindCount;
	}

	return prog;
}

svkShaderProgram svkCreateShaderProgramFromFile(
	svkDevice&						device,
	const char* const				vertFilename, 
	const char* const				tcsFilename, 
	const char* const				tesFilename, 
	const char* const				geoFilename, 
	const char* const				fragFilename, 
	const char* const				compFilename)
{
	svkShaderProgram prog;
	memclr(prog);

	const int MAX_LAYOUT_BIND = 128;
	VkDescriptorSetLayoutBinding binds[MAX_LAYOUT_BIND] = { 0 };
	int bindCount = 0;

	if (NULL != vertFilename && 0 != vertFilename[0])
		prog.vert	= _createShaderFromFile(device, vertFilename, SHADER_TYPE_VERT, binds, &bindCount, countof(binds));
	if (NULL != tcsFilename && 0 != tcsFilename[0])
		prog.tcs	= _createShaderFromFile(device, tcsFilename, SHADER_TYPE_TCS, binds, &bindCount, countof(binds));
	if (NULL != tesFilename && 0 != tesFilename[0])
		prog.tes	= _createShaderFromFile(device, tesFilename, SHADER_TYPE_TES, binds, &bindCount, countof(binds));
	if (NULL != geoFilename && 0 != geoFilename[0])
		prog.geo	= _createShaderFromFile(device, geoFilename, SHADER_TYPE_GEO, binds, &bindCount, countof(binds));
	if (NULL != fragFilename && 0 != fragFilename[0])
		prog.frag	= _createShaderFromFile(device, fragFilename, SHADER_TYPE_FRAG, binds, &bindCount, countof(binds));
	if (NULL != compFilename && 0 != compFilename[0])
		prog.comp	= _createShaderFromFile(device, compFilename, SHADER_TYPE_COMP, binds, &bindCount, countof(binds));

	if (bindCount > 0)
	{
		assert(NULL == prog.layoutBinds);
		prog.layoutBinds = new VkDescriptorSetLayoutBinding[bindCount];
		memcpy(prog.layoutBinds, binds, sizeof(VkDescriptorSetLayoutBinding) * bindCount);
		prog.layoutBindCount = bindCount;
	}

	return prog;
}

void svkDestroyShaderProgram(svkDevice& device, svkShaderProgram& shaderProgram)
{
	if (NULL != shaderProgram.vert)
		vkDestroyShaderModule(device.device, shaderProgram.vert, NULL);

	if (NULL != shaderProgram.tcs)
		vkDestroyShaderModule(device.device, shaderProgram.tcs, NULL);

	if (NULL != shaderProgram.tes)
		vkDestroyShaderModule(device.device, shaderProgram.tes, NULL);

	if (NULL != shaderProgram.geo)
		vkDestroyShaderModule(device.device, shaderProgram.geo, NULL);

	if (NULL != shaderProgram.frag)
		vkDestroyShaderModule(device.device, shaderProgram.frag, NULL);

	if (NULL != shaderProgram.comp)
		vkDestroyShaderModule(device.device, shaderProgram.comp, NULL);

	if (NULL != shaderProgram.layoutBinds)
		delete[] shaderProgram.layoutBinds;
}

int _buildShaderStageCreateInfo(VkPipelineShaderStageCreateInfo* shaderStages, const int shaderStagesCapacity, svkShaderProgram& prog)
{
	int shaderStageCount = 0;
	if (NULL != prog.vert)
	{
		VkPipelineShaderStageCreateInfo& stage = shaderStages[shaderStageCount];
		stage.sType		= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage.stage		= VK_SHADER_STAGE_VERTEX_BIT;
		stage.module	= prog.vert;
		stage.pName		= "main";
		++shaderStageCount;
	}
	assert(shaderStageCount < shaderStagesCapacity);

	if (NULL != prog.tcs)
	{
		VkPipelineShaderStageCreateInfo& stage = shaderStages[shaderStageCount];
		stage.sType		= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage.stage		= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		stage.module	= prog.tcs;
		stage.pName		= "main";
		++shaderStageCount;
	}
	assert(shaderStageCount < shaderStagesCapacity);

	if (NULL != prog.tes)
	{
		VkPipelineShaderStageCreateInfo& stage = shaderStages[shaderStageCount];
		stage.sType		= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage.stage		= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		stage.module	= prog.tes;
		stage.pName		= "main";
		++shaderStageCount;
	}
	assert(shaderStageCount < shaderStagesCapacity);

	if (NULL != prog.geo)
	{
		VkPipelineShaderStageCreateInfo& stage = shaderStages[shaderStageCount];
		stage.sType		= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage.stage		= VK_SHADER_STAGE_GEOMETRY_BIT;
		stage.module	= prog.geo;
		stage.pName		= "main";
		++shaderStageCount;
	}
	assert(shaderStageCount < shaderStagesCapacity);

	if (NULL != prog.frag)
	{
		VkPipelineShaderStageCreateInfo& stage = shaderStages[shaderStageCount];
		stage.sType		= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage.stage		= VK_SHADER_STAGE_FRAGMENT_BIT;
		stage.module	= prog.frag;
		stage.pName		= "main";
		++shaderStageCount;
	}
	assert(shaderStageCount < shaderStagesCapacity);

	if (NULL != prog.comp)
	{
		VkPipelineShaderStageCreateInfo& stage = shaderStages[shaderStageCount];
		stage.sType		= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage.stage		= VK_SHADER_STAGE_COMPUTE_BIT;
		stage.module	= prog.comp;
		stage.pName		= "main";
		++shaderStageCount;
	}
	assert(shaderStageCount < shaderStagesCapacity);

	return shaderStageCount;
}

svkPipeline svkCreatePipelineEx(
	svkDevice&				device, 
	VkDescriptorSetLayout	pipelineLayout, 
	VkRenderPass			renderPass, 
	VkPrimitiveTopology		topology, 
	VkPipelineVertexInputStateCreateInfo& viCreateInfo, 
	svkShaderProgram&		shaderProgram, 
	VkPipelineCache*		oldPipelineCache)
{
	svkPipeline pipeline;
	VkResult err;

	VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
	VkPipelineDynamicStateCreateInfo dynamicState;
	memclr(dynamicStateEnables);
	memclr(dynamicState);
	dynamicState.sType			= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates	= dynamicStateEnables;

	VkPipelineInputAssemblyStateCreateInfo iaCreateInfo;
	memclr(iaCreateInfo);
	iaCreateInfo.sType			= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	iaCreateInfo.topology		= topology;

	VkPipelineRasterizationStateCreateInfo rs;
	memclr(rs);
	rs.sType					= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rs.polygonMode				= VK_POLYGON_MODE_FILL;
	rs.cullMode					= VK_CULL_MODE_BACK_BIT;
	rs.frontFace				= VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rs.depthClampEnable			= VK_FALSE;
	rs.rasterizerDiscardEnable	= VK_FALSE;
	rs.depthBiasEnable			= VK_FALSE;
	rs.lineWidth				= 1.0f;

	VkPipelineColorBlendStateCreateInfo cb;
	memclr(cb);
	cb.sType					= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	VkPipelineColorBlendAttachmentState attState[1];
	memclr(attState);
	attState[0].colorWriteMask	= 0xf;
	attState[0].blendEnable		= VK_FALSE;
	cb.attachmentCount			= 1;
	cb.pAttachments				= attState;

	VkPipelineViewportStateCreateInfo vp;
	memclr(vp);
	vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.viewportCount			= 1;
	dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	vp.scissorCount				= 1;
	dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

	VkPipelineDepthStencilStateCreateInfo ds;
	memclr(ds);
	ds.sType					= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable			= VK_TRUE;
	ds.depthWriteEnable			= VK_TRUE;
	ds.depthCompareOp			= VK_COMPARE_OP_LESS_OR_EQUAL;
	ds.depthBoundsTestEnable	= VK_FALSE;
	ds.back.failOp				= VK_STENCIL_OP_KEEP;
	ds.back.passOp				= VK_STENCIL_OP_KEEP;
	ds.back.compareOp			= VK_COMPARE_OP_ALWAYS;
	ds.stencilTestEnable		= VK_FALSE;
	ds.front = ds.back;

	VkPipelineMultisampleStateCreateInfo ms;
	memclr(ms);
	ms.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.pSampleMask				= NULL;
	ms.rasterizationSamples		= VK_SAMPLE_COUNT_1_BIT;


	VkPipelineShaderStageCreateInfo shaderStages[64];
	memset(shaderStages, 0, sizeof(shaderStages));
	int shaderStageCount = _buildShaderStageCreateInfo(shaderStages, 64, shaderProgram);

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	memclr(pipelineLayoutCreateInfo);
	pipelineLayoutCreateInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext					= NULL;
	pipelineLayoutCreateInfo.flags					= 0;
	pipelineLayoutCreateInfo.setLayoutCount			= 1;
	pipelineLayoutCreateInfo.pSetLayouts			= &pipelineLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount	= 0;
	pipelineLayoutCreateInfo.pPushConstantRanges	= NULL;

	err = vkCreatePipelineLayout(device.device, &pipelineLayoutCreateInfo, NULL, &pipeline.layout);
	assert(!err);

	VkPipelineCache pipelineCache = NULL;
	if (NULL != oldPipelineCache)
	{
		if (*oldPipelineCache == NULL)
		{
			VkPipelineCacheCreateInfo pipelineCacheCreateInfo;
			memclr(pipelineCacheCreateInfo);
			pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
			err = vkCreatePipelineCache(device.device, &pipelineCacheCreateInfo, NULL, &pipelineCache);
			assert(!err);
		}
		else
			pipelineCache = *oldPipelineCache;
	}
	pipeline.cache = pipelineCache;

	VkGraphicsPipelineCreateInfo pipelineCreateInfo;
	memclr(pipelineCreateInfo);
	pipelineCreateInfo.sType				= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.layout				= pipeline.layout;
	pipelineCreateInfo.pVertexInputState	= &viCreateInfo;
	pipelineCreateInfo.pInputAssemblyState	= &iaCreateInfo;
	pipelineCreateInfo.pRasterizationState	= &rs;
	pipelineCreateInfo.pColorBlendState		= &cb;
	pipelineCreateInfo.pMultisampleState	= &ms;
	pipelineCreateInfo.pViewportState		= &vp;
	pipelineCreateInfo.pDepthStencilState	= &ds;
	pipelineCreateInfo.stageCount			= shaderStageCount;
	pipelineCreateInfo.pStages				= shaderStages;
	pipelineCreateInfo.renderPass			= renderPass;
	pipelineCreateInfo.pDynamicState		= &dynamicState;

	err = vkCreateGraphicsPipelines(device.device, pipelineCache, 1, &pipelineCreateInfo, NULL, &pipeline.pipeline);
	assert(!err);

	return pipeline;
}

const svkDescriptorData* _findDescriptorDataWithBinding(const svkDescriptorData* datas, const int dataCount, int binding)
{
	const svkDescriptorData* pdata	= NULL;
	for (int i = 0; i < dataCount; ++i)
	{
		if (datas[i].binding == binding)
			return &datas[i];
	}
	return NULL;
}

void svkUpdateDescriptorSet(svkDevice& device, VkDescriptorSet descriptorSet, const VkDescriptorSetLayoutBinding* binds, const int bindCount, const svkDescriptorData* datas, const int dataCount)
{
	VkWriteDescriptorSet writes[64];
	memset(&writes, 0, sizeof(writes));
	for (int i = 0; i < bindCount; ++i)
	{
		const VkDescriptorSetLayoutBinding&	bind	= binds[i];
		VkWriteDescriptorSet&				write	= writes[i]; 

		write.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding		= bind.binding;
		write.descriptorType	= bind.descriptorType;
		write.dstSet			= descriptorSet;

		const svkDescriptorData* pData = _findDescriptorDataWithBinding(datas, dataCount, bind.binding);
		if (NULL == pData)
		{
			assert(false);
			continue;
		}

		// svkDescriptorData 数组中 data 的 buffer 或者 texture 的数量，和 VkDescriptorSetLayoutBinding 中的 descriptorCount 的数量不一定相等。
		// 这时为了防止写入无效数据，需要取这两个值的较小值作为实际写入的 descriptorCount
		uint32_t descriptorCount = min(bind.descriptorCount, static_cast<uint32_t>(pData->dataCount));
		write.descriptorCount	= descriptorCount;

		if (bind.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
			|| bind.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		{
			VkDescriptorBufferInfo* infos = new VkDescriptorBufferInfo[descriptorCount];
			memset(infos, 0, descriptorCount * sizeof(VkDescriptorBufferInfo));

			for (uint bufferIndex = 0; bufferIndex < descriptorCount; ++bufferIndex)
			{
				const svkDescriptorData::Info& dataInfo = pData->data[bufferIndex]; 
				if (NULL == dataInfo.buffer.buffer || 0 == dataInfo.buffer.bufferSize)
				{
					assert(false); // data 中的数据不完整！
					continue;
				}
				infos[bufferIndex].offset	= 0;
				infos[bufferIndex].range	= dataInfo.buffer.bufferSize;
				infos[bufferIndex].buffer	= dataInfo.buffer.buffer;
			}
			write.pBufferInfo = infos;
		}
		else if (bind.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		{
			VkDescriptorImageInfo* infos = new VkDescriptorImageInfo[descriptorCount];
			memset(infos, 0, descriptorCount * sizeof(VkDescriptorImageInfo));

			for (uint texIndex = 0; texIndex < descriptorCount; ++texIndex)
			{
				const svkDescriptorData::Info& dataInfo = pData->data[texIndex]; 
				infos[texIndex].sampler		= dataInfo.texture.sampler;
				infos[texIndex].imageView	= dataInfo.texture.imageView;
				infos[texIndex].imageLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}
			write.pImageInfo = infos;
		}
	}

	vkUpdateDescriptorSets(device.device, bindCount, writes, 0, NULL);

	for (int i = 0; i < bindCount; ++i)
	{
		VkWriteDescriptorSet& write = writes[i];
		if (write.pBufferInfo != NULL)
			delete[] write.pBufferInfo;
		if (write.pImageInfo != NULL)
			delete[] write.pImageInfo;
		if (write.pTexelBufferView != NULL)
			delete[] write.pTexelBufferView;
	}
}


