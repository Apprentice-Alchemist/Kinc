#include "texture.h"

#include <kinc/graphics5/rendertarget.h>
#include <kinc/graphics5/texture.h>
#include <kinc/math/core.h>

#include <kinc/backend/SystemMicrosoft.h>

// static const int textureCount = 16;

kinc_g5_render_target_t *currentRenderTargets[textureCount] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
struct kinc_g5_texture *currentTextures[textureCount] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

bool bilinearFiltering = false;

static ID3D12DescriptorHeap *samplerDescriptorHeapPoint;
static ID3D12DescriptorHeap *samplerDescriptorHeapBilinear;
static const int heapSize = 1024;
static int heapIndex = 0;
static ID3D12DescriptorHeap *srvHeap;
static ID3D12DescriptorHeap *samplerHeap;

#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
/*static int d3d12_textureAlignment() {
    return D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
}*/
#else
int d3d12_textureAlignment();
#endif

static inline UINT64 GetRequiredIntermediateSize(ID3D12Resource *destinationResource, UINT FirstSubresource, UINT NumSubresources) {
	D3D12_RESOURCE_DESC desc = D3D12ResourceGetDesc(destinationResource);
	UINT64 requiredSize = 0;
	device->lpVtbl->GetCopyableFootprints(device, &desc, FirstSubresource, NumSubresources, 0, NULL, NULL, NULL, &requiredSize);
	device->lpVtbl->Release(device);
	return requiredSize;
}

static DXGI_FORMAT convertImageFormat(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case KINC_IMAGE_FORMAT_RGBA64:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case KINC_IMAGE_FORMAT_RGB24:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case KINC_IMAGE_FORMAT_A32:
		return DXGI_FORMAT_R32_FLOAT;
	case KINC_IMAGE_FORMAT_A16:
		return DXGI_FORMAT_R16_FLOAT;
	case KINC_IMAGE_FORMAT_GREY8:
		return DXGI_FORMAT_R8_UNORM;
	case KINC_IMAGE_FORMAT_BGRA32:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case KINC_IMAGE_FORMAT_RGBA32:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	default:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
}

static int formatByteSize(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return 16;
	case KINC_IMAGE_FORMAT_RGBA64:
		return 8;
	case KINC_IMAGE_FORMAT_RGB24:
		return 4;
	case KINC_IMAGE_FORMAT_A32:
		return 4;
	case KINC_IMAGE_FORMAT_A16:
		return 2;
	case KINC_IMAGE_FORMAT_GREY8:
		return 1;
	case KINC_IMAGE_FORMAT_BGRA32:
	case KINC_IMAGE_FORMAT_RGBA32:
		return 4;
	default:
		return 4;
	}
}

void kinc_g5_internal_set_textures(ID3D12GraphicsCommandList *commandList) {
	if (currentRenderTargets[0] != NULL || currentTextures[0] != NULL) {
		int srvStep = device->lpVtbl->GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		int samplerStep = device->lpVtbl->GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		if (heapIndex + textureCount >= heapSize) {
			heapIndex = 0;
		}

		ID3D12DescriptorHeap *samplerDescriptorHeap = bilinearFiltering ? samplerDescriptorHeapBilinear : samplerDescriptorHeapPoint;
		D3D12_GPU_DESCRIPTOR_HANDLE srvGpu = GetGPUDescriptorHandle(srvHeap);
		D3D12_GPU_DESCRIPTOR_HANDLE samplerGpu = GetGPUDescriptorHandle(samplerHeap);
		srvGpu.ptr += heapIndex * srvStep;
		samplerGpu.ptr += heapIndex * samplerStep;

		for (int i = 0; i < textureCount; ++i) {
			if (currentRenderTargets[i] != NULL || currentTextures[i] != NULL) {

				D3D12_CPU_DESCRIPTOR_HANDLE srvCpu = GetCPUDescriptorHandle(srvHeap);
				D3D12_CPU_DESCRIPTOR_HANDLE samplerCpu = GetCPUDescriptorHandle(samplerHeap);
				srvCpu.ptr += heapIndex * srvStep;
				samplerCpu.ptr += heapIndex * samplerStep;
				++heapIndex;

				if (currentRenderTargets[i] != NULL) {
					bool is_depth = currentRenderTargets[i]->impl.stage_depth == i;
					D3D12_CPU_DESCRIPTOR_HANDLE sourceCpu = is_depth ? GetCPUDescriptorHandle(currentRenderTargets[i]->impl.srvDepthDescriptorHeap)
					                                                 : GetCPUDescriptorHandle(currentRenderTargets[i]->impl.srvDescriptorHeap);
					device->lpVtbl->CopyDescriptorsSimple(device, 1, srvCpu, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					device->lpVtbl->CopyDescriptorsSimple(device, 1, samplerCpu, GetCPUDescriptorHandle(samplerDescriptorHeap),
					                                      D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

					if (is_depth) {
						currentRenderTargets[i]->impl.stage_depth = -1;
						currentRenderTargets[i] = NULL;
					}
				}
				else {
					D3D12_CPU_DESCRIPTOR_HANDLE sourceCpu = GetCPUDescriptorHandle(currentTextures[i]->impl.srvDescriptorHeap);
					device->lpVtbl->CopyDescriptorsSimple(device, 1, srvCpu, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					device->lpVtbl->CopyDescriptorsSimple(device, 1, samplerCpu, GetCPUDescriptorHandle(samplerDescriptorHeap),
					                                      D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
				}
			}
		}

		ID3D12DescriptorHeap *heaps[2] = {srvHeap, samplerHeap};
		commandList->lpVtbl->SetDescriptorHeaps(commandList, 2, heaps);
		commandList->lpVtbl->SetGraphicsRootDescriptorTable(commandList, 0, srvGpu);
		commandList->lpVtbl->SetGraphicsRootDescriptorTable(commandList, 1, samplerGpu);
	}
}

static void createSampler(bool bilinear, D3D12_FILTER filter) {
	D3D12_DESCRIPTOR_HEAP_DESC descHeapSampler = {0};
	descHeapSampler.NumDescriptors = 2;
	descHeapSampler.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	descHeapSampler.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	device->lpVtbl->CreateDescriptorHeap(device, &descHeapSampler, &IID_ID3D12DescriptorHeap,
	                                     bilinear ? &samplerDescriptorHeapBilinear : &samplerDescriptorHeapPoint);

	D3D12_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D12_SAMPLER_DESC));
	samplerDesc.Filter = filter;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	device->lpVtbl->CreateSampler(device, &samplerDesc, GetCPUDescriptorHandle(bilinear ? samplerDescriptorHeapBilinear : samplerDescriptorHeapPoint));
}

void createSamplersAndHeaps() {
	createSampler(false, D3D12_FILTER_MIN_MAG_MIP_POINT);
	createSampler(true, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {0};
	heapDesc.NumDescriptors = heapSize;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	device->lpVtbl->CreateDescriptorHeap(device, &heapDesc, &IID_ID3D12DescriptorHeap, &srvHeap);

	D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {0};
	samplerHeapDesc.NumDescriptors = heapSize;
	samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	device->lpVtbl->CreateDescriptorHeap(device, &samplerHeapDesc, &IID_ID3D12DescriptorHeap, &samplerHeap);
}

void kinc_memory_emergency();

void kinc_g5_texture_init_from_image(kinc_g5_texture_t *texture, kinc_image_t *image) {
	memset(&texture->impl, 0, sizeof(texture->impl));
	texture->impl.stage = 0;
	texture->impl.mipmap = true;
	texture->texWidth = image->width;
	texture->texHeight = image->height;

	DXGI_FORMAT d3dformat = convertImageFormat(image->format);
	int formatSize = formatByteSize(image->format);

	D3D12_HEAP_PROPERTIES heapPropertiesDefault;
	heapPropertiesDefault.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapPropertiesDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapPropertiesDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapPropertiesDefault.CreationNodeMask = 1;
	heapPropertiesDefault.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDescTex;
	resourceDescTex.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDescTex.Alignment = 0;
	resourceDescTex.Width = texture->texWidth;
	resourceDescTex.Height = texture->texHeight;
	resourceDescTex.DepthOrArraySize = 1;
	resourceDescTex.MipLevels = 1;
	resourceDescTex.Format = d3dformat;
	resourceDescTex.SampleDesc.Count = 1;
	resourceDescTex.SampleDesc.Quality = 0;
	resourceDescTex.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDescTex.Flags = D3D12_RESOURCE_FLAG_NONE;

	HRESULT result = device->lpVtbl->CreateCommittedResource(device, &heapPropertiesDefault, D3D12_HEAP_FLAG_NONE, &resourceDescTex,
	                                                         D3D12_RESOURCE_STATE_COPY_DEST, NULL, &IID_ID3D12Resource, &texture->impl.image);
	if (result != S_OK) {
		for (int i = 0; i < 10; ++i) {
			kinc_memory_emergency();
			result = device->lpVtbl->CreateCommittedResource(device, &heapPropertiesDefault, D3D12_HEAP_FLAG_NONE, &resourceDescTex,
			                                                 D3D12_RESOURCE_STATE_COPY_DEST, NULL, &IID_ID3D12Resource, &texture->impl.image);
			if (result == S_OK) {
				break;
			}
		}
	}

	D3D12_HEAP_PROPERTIES heapPropertiesUpload;
	heapPropertiesUpload.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapPropertiesUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapPropertiesUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapPropertiesUpload.CreationNodeMask = 1;
	heapPropertiesUpload.VisibleNodeMask = 1;

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture->impl.image, 0, 1);
	D3D12_RESOURCE_DESC resourceDescBuffer;
	resourceDescBuffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDescBuffer.Alignment = 0;
	resourceDescBuffer.Width = uploadBufferSize;
	resourceDescBuffer.Height = 1;
	resourceDescBuffer.DepthOrArraySize = 1;
	resourceDescBuffer.MipLevels = 1;
	resourceDescBuffer.Format = DXGI_FORMAT_UNKNOWN;
	resourceDescBuffer.SampleDesc.Count = 1;
	resourceDescBuffer.SampleDesc.Quality = 0;
	resourceDescBuffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDescBuffer.Flags = D3D12_RESOURCE_FLAG_NONE;

	result = device->lpVtbl->CreateCommittedResource(device, &heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &resourceDescBuffer,
	                                                 D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, &texture->impl.uploadImage);
	if (result != S_OK) {
		for (int i = 0; i < 10; ++i) {
			kinc_memory_emergency();
			result = device->lpVtbl->CreateCommittedResource(device, &heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &resourceDescBuffer,
			                                                 D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, &texture->impl.uploadImage);
			if (result == S_OK) {
				break;
			}
		}
	}

	texture->impl.stride = (int)kinc_ceil(uploadBufferSize / (float)image->height);
	if (texture->impl.stride < d3d12_textureAlignment()) {
		texture->impl.stride = d3d12_textureAlignment();
	}

	BYTE *pixel;
	texture->impl.uploadImage->lpVtbl->Map(texture->impl.uploadImage, 0, NULL, (void **)&pixel);
	int pitch = kinc_g5_texture_stride(texture);
	for (int y = 0; y < texture->texHeight; ++y) {
		memcpy(&pixel[y * pitch], &((uint8_t *)image->data)[y * texture->texWidth * formatSize], texture->texWidth * formatSize);
	}
	texture->impl.uploadImage->lpVtbl->Unmap(texture->impl.uploadImage, 0, NULL);

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {0};
	descriptorHeapDesc.NumDescriptors = 1;

	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	device->lpVtbl->CreateDescriptorHeap(device, &descriptorHeapDesc, &IID_ID3D12DescriptorHeap, &texture->impl.srvDescriptorHeap);

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {0};
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Format = d3dformat;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	device->lpVtbl->CreateShaderResourceView(device, texture->impl.image, &shaderResourceViewDesc, GetCPUDescriptorHandle(texture->impl.srvDescriptorHeap));
}

void create_texture(struct kinc_g5_texture *texture, int width, int height, kinc_image_format_t format, D3D12_RESOURCE_FLAGS flags) {
	// kinc_image_init(&texture->image, width, height, format, readable);
	memset(&texture->impl, 0, sizeof(texture->impl));
	texture->impl.stage = 0;
	texture->impl.mipmap = true;
	texture->texWidth = width;
	texture->texHeight = height;

	DXGI_FORMAT d3dformat = convertImageFormat(format);

	D3D12_HEAP_PROPERTIES heapPropertiesDefault;
	heapPropertiesDefault.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapPropertiesDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapPropertiesDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapPropertiesDefault.CreationNodeMask = 1;
	heapPropertiesDefault.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDescTex;
	resourceDescTex.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDescTex.Alignment = 0;
	resourceDescTex.Width = texture->texWidth;
	resourceDescTex.Height = texture->texHeight;
	resourceDescTex.DepthOrArraySize = 1;
	resourceDescTex.MipLevels = 1;
	resourceDescTex.Format = d3dformat;
	resourceDescTex.SampleDesc.Count = 1;
	resourceDescTex.SampleDesc.Quality = 0;
	resourceDescTex.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDescTex.Flags = flags;

	HRESULT result = device->lpVtbl->CreateCommittedResource(device, &heapPropertiesDefault, D3D12_HEAP_FLAG_NONE, &resourceDescTex,
	                                                         D3D12_RESOURCE_STATE_COPY_DEST, NULL, &IID_ID3D12Resource, &texture->impl.image);
	if (result != S_OK) {
		for (int i = 0; i < 10; ++i) {
			kinc_memory_emergency();
			result = device->lpVtbl->CreateCommittedResource(device, &heapPropertiesDefault, D3D12_HEAP_FLAG_NONE, &resourceDescTex,
			                                                 D3D12_RESOURCE_STATE_COPY_DEST, NULL, &IID_ID3D12Resource, &texture->impl.image);
			if (result == S_OK) {
				break;
			}
		}
	}

	D3D12_HEAP_PROPERTIES heapPropertiesUpload;
	heapPropertiesUpload.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapPropertiesUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapPropertiesUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapPropertiesUpload.CreationNodeMask = 1;
	heapPropertiesUpload.VisibleNodeMask = 1;

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture->impl.image, 0, 1);
	D3D12_RESOURCE_DESC resourceDescBuffer;
	resourceDescBuffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDescBuffer.Alignment = 0;
	resourceDescBuffer.Width = uploadBufferSize;
	resourceDescBuffer.Height = 1;
	resourceDescBuffer.DepthOrArraySize = 1;
	resourceDescBuffer.MipLevels = 1;
	resourceDescBuffer.Format = DXGI_FORMAT_UNKNOWN;
	resourceDescBuffer.SampleDesc.Count = 1;
	resourceDescBuffer.SampleDesc.Quality = 0;
	resourceDescBuffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDescBuffer.Flags = D3D12_RESOURCE_FLAG_NONE;

	result = device->lpVtbl->CreateCommittedResource(device, &heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &resourceDescBuffer,
	                                                 D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, &texture->impl.uploadImage);
	if (result != S_OK) {
		for (int i = 0; i < 10; ++i) {
			kinc_memory_emergency();
			result = device->lpVtbl->CreateCommittedResource(device, &heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &resourceDescBuffer,
			                                                 D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, &texture->impl.uploadImage);
			if (result == S_OK) {
				break;
			}
		}
	}

	texture->impl.stride = (int)kinc_ceil(uploadBufferSize / (float)height);
	if (texture->impl.stride < d3d12_textureAlignment()) {
		texture->impl.stride = d3d12_textureAlignment();
	}

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {0};
	descriptorHeapDesc.NumDescriptors = 1;

	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	device->lpVtbl->CreateDescriptorHeap(device, &descriptorHeapDesc, &IID_ID3D12DescriptorHeap, &texture->impl.srvDescriptorHeap);

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {0};
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Format = d3dformat;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	device->lpVtbl->CreateShaderResourceView(device, texture->impl.image, &shaderResourceViewDesc, GetCPUDescriptorHandle(texture->impl.srvDescriptorHeap));
}

void kinc_g5_texture_init(struct kinc_g5_texture *texture, int width, int height, kinc_image_format_t format) {
	create_texture(texture, width, height, format, D3D12_RESOURCE_FLAG_NONE);
}

void kinc_g5_texture_init3d(kinc_g5_texture_t *texture, int width, int height, int depth, kinc_image_format_t format) {
	// kinc_image_init3d(&texture->image, width, height, depth, format, readable);
}

void kinc_g5_texture_init_non_sampled_access(struct kinc_g5_texture *texture, int width, int height, kinc_image_format_t format) {
	create_texture(texture, width, height, format, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
}

void kinc_g5_internal_texture_unset(struct kinc_g5_texture *texture);

void kinc_g5_texture_destroy(struct kinc_g5_texture *texture) {
	kinc_g5_internal_texture_unset(texture);
	texture->impl.image->lpVtbl->Release(texture->impl.image);
	texture->impl.uploadImage->lpVtbl->Release(texture->impl.uploadImage);
	texture->impl.srvDescriptorHeap->lpVtbl->Release(texture->impl.srvDescriptorHeap);
}

void kinc_g5_internal_texture_unmipmap(struct kinc_g5_texture *texture) {
	texture->impl.mipmap = false;
}

void kinc_g5_internal_texture_set(struct kinc_g5_texture *texture, int unit) {
	if (unit < 0) return;
	// context->PSSetShaderResources(unit.unit, 1, &view);
	texture->impl.stage = unit;
	currentTextures[texture->impl.stage] = texture;
	currentRenderTargets[texture->impl.stage] = NULL;
}

void kinc_g5_internal_texture_unset(struct kinc_g5_texture *texture) {
	if (currentTextures[texture->impl.stage] == texture) {

		currentTextures[texture->impl.stage] = NULL;
	}
}

uint8_t *kinc_g5_texture_lock(struct kinc_g5_texture *texture) {
	BYTE *pixel;
	texture->impl.uploadImage->lpVtbl->Map(texture->impl.uploadImage, 0, NULL, (void **)&pixel);
	return pixel;
}

void kinc_g5_texture_unlock(struct kinc_g5_texture *texture) {
	texture->impl.uploadImage->lpVtbl->Unmap(texture->impl.uploadImage, 0, NULL);
}

void kinc_g5_texture_clear(kinc_g5_texture_t *texture, int x, int y, int z, int width, int height, int depth, unsigned color) {}

int kinc_g5_texture_stride(struct kinc_g5_texture *texture) {
	/*int baseStride = texture->format == KINC_IMAGE_FORMAT_RGBA32 ? (texture->texWidth * 4) : texture->texWidth;
	if (texture->format == KINC_IMAGE_FORMAT_GREY8) return texture->texWidth; // please investigate further
	for (int i = 0;; ++i) {
	    if (d3d12_textureAlignment() * i >= baseStride) {
	        return d3d12_textureAlignment() * i;
	    }
	}*/
	return texture->impl.stride;
}

void kinc_g5_texture_generate_mipmaps(struct kinc_g5_texture *texture, int levels) {}

void kinc_g5_texture_set_mipmap(struct kinc_g5_texture *texture, kinc_image_t *mipmap, int level) {}
