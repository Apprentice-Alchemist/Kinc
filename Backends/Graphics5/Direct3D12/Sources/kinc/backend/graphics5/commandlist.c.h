#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/constantbuffer.h>
#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/vertexbuffer.h>
#include <kinc/window.h>

extern ID3D12CommandQueue *commandQueue;
extern kinc_g5_texture_t *currentTextures[textureCount];
extern kinc_g5_render_target_t *currentRenderTargets[textureCount];

/*const int constantBufferMultiply = 1024;
int currentConstantBuffer = 0;
ID3D12Resource* vertexConstantBuffer;
ID3D12Resource* fragmentConstantBuffer;
bool created = false;

void createConstantBuffer() {
    if (created) return;
    created = true;

    device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertexConstants) * constantBufferMultiply),
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_GRAPHICS_PPV_ARGS(&vertexConstantBuffer));

    void* p;
    vertexConstantBuffer->Map(0, nullptr, &p);
    ZeroMemory(p, sizeof(vertexConstants) * constantBufferMultiply);
    vertexConstantBuffer->Unmap(0, nullptr);

    device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(sizeof(fragmentConstants) * constantBufferMultiply),
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_GRAPHICS_PPV_ARGS(&fragmentConstantBuffer));

    fragmentConstantBuffer->Map(0, nullptr, &p);
    ZeroMemory(p, sizeof(fragmentConstants) * constantBufferMultiply);
    fragmentConstantBuffer->Unmap(0, nullptr);
}*/

static UINT64 renderFenceValue = 0;
static ID3D12Fence *renderFence;
static HANDLE renderFenceEvent;

static kinc_g5_render_target_t *currentRenderTarget = NULL;
static int currentRenderTargetCount = 0;
static D3D12_CPU_DESCRIPTOR_HANDLE targetDescriptors[16];

static void init() {
	static bool initialized = false;
	if (!initialized) {
		initialized = true;
		renderFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		device->lpVtbl->CreateFence(device, 0, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, &renderFence);
	}
}

/*void waitForFence(ID3D12Fence *fence, UINT64 completionValue, HANDLE waitEvent) {
    if (fence->GetCompletedValue() < completionValue) {
        fence->SetEventOnCompletion(completionValue, waitEvent);
        WaitForSingleObject(waitEvent, INFINITE);
    }
}*/

static void graphicsFlush(struct kinc_g5_command_list *list, ID3D12CommandAllocator *commandAllocator) {
	list->impl._commandList->lpVtbl->Close(list->impl._commandList);
	list->impl.closed = true;

	ID3D12CommandList *commandLists[] = {(ID3D12CommandList *)list->impl._commandList};
	commandQueue->lpVtbl->ExecuteCommandLists(commandQueue, 1, commandLists);

	commandQueue->lpVtbl->Signal(commandQueue, renderFence, ++renderFenceValue);
}

static void graphicsWait(struct kinc_g5_command_list *list, ID3D12CommandAllocator *commandAllocator) {
	waitForFence(renderFence, renderFenceValue, renderFenceEvent);
	commandAllocator->lpVtbl->Reset(commandAllocator);
	list->impl._commandList->lpVtbl->Reset(list->impl._commandList, commandAllocator, NULL);
	if (currentRenderTarget != NULL) {
		if (currentRenderTarget->impl.depthStencilDescriptorHeap != NULL) {
			D3D12_CPU_DESCRIPTOR_HANDLE heapStart = GetCPUDescriptorHandle(currentRenderTarget->impl.depthStencilDescriptorHeap);
			list->impl._commandList->lpVtbl->OMSetRenderTargets(list->impl._commandList, currentRenderTargetCount, &targetDescriptors[0], false, &heapStart);
		}
		else {
			list->impl._commandList->lpVtbl->OMSetRenderTargets(list->impl._commandList, currentRenderTargetCount, &targetDescriptors[0], false, NULL);
		}
		list->impl._commandList->lpVtbl->RSSetViewports(list->impl._commandList, 1, (D3D12_VIEWPORT *)&currentRenderTarget->impl.viewport);
		list->impl._commandList->lpVtbl->RSSetScissorRects(list->impl._commandList, 1, (D3D12_RECT *)&currentRenderTarget->impl.scissor);
	}
}

static void graphicsFlushAndWait(struct kinc_g5_command_list *list, ID3D12CommandAllocator *commandAllocator) {
	graphicsFlush(list, commandAllocator);
	graphicsWait(list, commandAllocator);
}

static int formatSize(DXGI_FORMAT format) {
	switch (format) {
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		return 16;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		return 8;
	case DXGI_FORMAT_R16_FLOAT:
		return 2;
	case DXGI_FORMAT_R8_UNORM:
		return 1;
	default:
		return 4;
	}
}

void kinc_g5_command_list_init(struct kinc_g5_command_list *list) {
	init();
	list->impl.closed = false;
	device->lpVtbl->CreateCommandAllocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, &list->impl._commandAllocator);
	device->lpVtbl->CreateCommandList(device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, list->impl._commandAllocator, NULL, &IID_ID3D12GraphicsCommandList,
	                                  &list->impl._commandList);
	//_commandList->Close();
	// createConstantBuffer();
	list->impl._indexCount = 0;
}

void kinc_g5_command_list_destroy(struct kinc_g5_command_list *list) {}

void kinc_g5_command_list_begin(struct kinc_g5_command_list *list) {
	if (list->impl.closed) {
		list->impl.closed = false;
		waitForFence(renderFence, list->impl.current_fence_value, renderFenceEvent);
		list->impl._commandAllocator->lpVtbl->Reset(list->impl._commandAllocator);
		list->impl._commandList->lpVtbl->Reset(list->impl._commandList, list->impl._commandAllocator, NULL);
	}
}

void kinc_g5_command_list_end(struct kinc_g5_command_list *list) {
	graphicsFlush(list, list->impl._commandAllocator);

	list->impl.current_fence_value = ++renderFenceValue;
	commandQueue->lpVtbl->Signal(commandQueue, renderFence, list->impl.current_fence_value);
}

void kinc_g5_command_list_clear(struct kinc_g5_command_list *list, kinc_g5_render_target_t *renderTarget, unsigned flags, unsigned color, float depth,
                                int stencil) {
	if (flags & KINC_G5_CLEAR_COLOR) {
		float clearColor[] = {((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f,
		                      ((color & 0xff000000) >> 24) / 255.0f};
		list->impl._commandList->lpVtbl->ClearRenderTargetView(list->impl._commandList, GetCPUDescriptorHandle(renderTarget->impl.renderTargetDescriptorHeap),
		                                                       clearColor, 0, NULL);
	}
	if ((flags & KINC_G5_CLEAR_DEPTH) || (flags & KINC_G5_CLEAR_STENCIL)) {
		D3D12_CLEAR_FLAGS d3dflags = (flags & KINC_G5_CLEAR_DEPTH) && (flags & KINC_G5_CLEAR_STENCIL) ? D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL
		                             : (flags & KINC_G5_CLEAR_DEPTH)                                  ? D3D12_CLEAR_FLAG_DEPTH
		                                                                                              : D3D12_CLEAR_FLAG_STENCIL;
		list->impl._commandList->lpVtbl->ClearDepthStencilView(list->impl._commandList, GetCPUDescriptorHandle(renderTarget->impl.depthStencilDescriptorHeap),
		                                                       d3dflags, depth, stencil, 0, NULL);
	}
}

void kinc_g5_command_list_render_target_to_framebuffer_barrier(struct kinc_g5_command_list *list, kinc_g5_render_target_t *renderTarget) {
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = renderTarget->impl.renderTarget;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &barrier);
}

void kinc_g5_command_list_framebuffer_to_render_target_barrier(struct kinc_g5_command_list *list, kinc_g5_render_target_t *renderTarget) {
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = renderTarget->impl.renderTarget;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &barrier);
}

void kinc_g5_command_list_texture_to_render_target_barrier(struct kinc_g5_command_list *list, kinc_g5_render_target_t *renderTarget) {
	if (renderTarget->impl.resourceState != RenderTargetResourceStateRenderTarget) {
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = renderTarget->impl.renderTarget;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &barrier);
		renderTarget->impl.resourceState = RenderTargetResourceStateRenderTarget;
	}
}

void kinc_g5_command_list_render_target_to_texture_barrier(struct kinc_g5_command_list *list, kinc_g5_render_target_t *renderTarget) {
	if (renderTarget->impl.resourceState != RenderTargetResourceStateTexture) {
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = renderTarget->impl.renderTarget;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &barrier);
		renderTarget->impl.resourceState = RenderTargetResourceStateTexture;
	}
}

void kinc_g5_command_list_set_pipeline_layout(struct kinc_g5_command_list *list) {
	kinc_g5_internal_setConstants(list->impl._commandList, list->impl._currentPipeline);
}

void kinc_g5_command_list_set_vertex_constant_buffer(struct kinc_g5_command_list *list, kinc_g5_constant_buffer_t *buffer, int offset, size_t size) {
#ifdef KORE_DXC
	if (list->impl._currentPipeline->impl.vertexConstantsSize > 0) {
		if (list->impl._currentPipeline->impl.textures > 0) {
			list->impl._commandList->lpVtbl->SetGraphicsRootConstantBufferView(
			    list->impl._commandList, 2, buffer->impl.constant_buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.constant_buffer) + offset);
		}
		else {
			list->impl._commandList->lpVtbl->SetGraphicsRootConstantBufferView(
			    list->impl._commandList, 0, buffer->impl.constant_buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.constant_buffer) + offset);
		}
	}
#else
	list->impl._commandList->lpVtbl->SetGraphicsRootConstantBufferView(
	    list->impl._commandList, 2, buffer->impl.constant_buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.constant_buffer) + offset);
#endif
}

void kinc_g5_command_list_set_fragment_constant_buffer(struct kinc_g5_command_list *list, kinc_g5_constant_buffer_t *buffer, int offset, size_t size) {
#ifdef KORE_DXC
	if (list->impl._currentPipeline->impl.fragmentConstantsSize > 0) {
		// list->impl._commandList->SetGraphicsRootConstantBufferView(3, buffer->impl.constant_buffer->GetGPUVirtualAddress() + offset);
	}
#else
	list->impl._commandList->lpVtbl->SetGraphicsRootConstantBufferView(
	    list->impl._commandList, 3, buffer->impl.constant_buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.constant_buffer) + offset);
#endif
}

void kinc_g5_command_list_draw_indexed_vertices(struct kinc_g5_command_list *list) {
	kinc_g5_command_list_draw_indexed_vertices_from_to(list, 0, list->impl._indexCount);
}

void kinc_g5_command_list_draw_indexed_vertices_from_to(struct kinc_g5_command_list *list, int start, int count) {
	list->impl._commandList->lpVtbl->IASetPrimitiveTopology(list->impl._commandList, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	/*u8* data;
	D3D12_RANGE range;
	range.Begin = currentConstantBuffer * sizeof(vertexConstants);
	range.End = range.Begin + sizeof(vertexConstants);
	vertexConstantBuffer->Map(0, &range, (void**)&data);
	memcpy(data + currentConstantBuffer * sizeof(vertexConstants), vertexConstants, sizeof(vertexConstants));
	vertexConstantBuffer->Unmap(0, &range);

	range.Begin = currentConstantBuffer * sizeof(fragmentConstants);
	range.End = range.Begin + sizeof(fragmentConstants);
	fragmentConstantBuffer->Map(0, &range, (void**)&data);
	memcpy(data + currentConstantBuffer * sizeof(fragmentConstants), fragmentConstants, sizeof(fragmentConstants));
	fragmentConstantBuffer->Unmap(0, &range);

	_commandList->SetGraphicsRootConstantBufferView(1, vertexConstantBuffer->GetGPUVirtualAddress() + currentConstantBuffer * sizeof(vertexConstants));
	_commandList->SetGraphicsRootConstantBufferView(2, fragmentConstantBuffer->GetGPUVirtualAddress() + currentConstantBuffer * sizeof(fragmentConstants));

	++currentConstantBuffer;
	if (currentConstantBuffer >= constantBufferMultiply) {
	    currentConstantBuffer = 0;
	}*/

	list->impl._commandList->lpVtbl->DrawIndexedInstanced(list->impl._commandList, count, 1, start, 0, 0);
}

void kinc_g5_command_list_draw_indexed_vertices_from_to_from(struct kinc_g5_command_list *list, int start, int count, int vertex_offset) {
	list->impl._commandList->lpVtbl->IASetPrimitiveTopology(list->impl._commandList, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->impl._commandList->lpVtbl->DrawIndexedInstanced(list->impl._commandList, count, 1, start, vertex_offset, 0);
}

void kinc_g5_command_list_draw_indexed_vertices_instanced(kinc_g5_command_list_t *list, int instanceCount) {
	kinc_g5_command_list_draw_indexed_vertices_instanced_from_to(list, instanceCount, 0, list->impl._indexCount);
}
void kinc_g5_command_list_draw_indexed_vertices_instanced_from_to(kinc_g5_command_list_t *list, int instanceCount, int start, int count) {
	list->impl._commandList->lpVtbl->IASetPrimitiveTopology(list->impl._commandList, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->impl._commandList->lpVtbl->DrawIndexedInstanced(list->impl._commandList, count, instanceCount, start, 0, 0);
}

void kinc_g5_command_list_execute_and_wait(struct kinc_g5_command_list *list) {
	graphicsFlushAndWait(list, list->impl._commandAllocator);
}

void kinc_g5_command_list_execute(struct kinc_g5_command_list *list) {
	graphicsFlush(list, list->impl._commandAllocator);
}

bool kinc_g5_non_pow2_textures_qupported(void) {
	return true;
}

void kinc_g5_command_list_viewport(struct kinc_g5_command_list *list, int x, int y, int width, int height) {
	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = (float)x;
	viewport.TopLeftY = (float)y;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	list->impl._commandList->lpVtbl->RSSetViewports(list->impl._commandList, 1, &viewport);
}

void kinc_g5_command_list_scissor(struct kinc_g5_command_list *list, int x, int y, int width, int height) {
	D3D12_RECT scissor;
	scissor.left = x;
	scissor.top = y;
	scissor.right = x + width;
	scissor.bottom = y + height;
	list->impl._commandList->lpVtbl->RSSetScissorRects(list->impl._commandList, 1, &scissor);
}

void kinc_g5_command_list_disable_scissor(struct kinc_g5_command_list *list) {
	if (currentRenderTarget != NULL) {
		list->impl._commandList->lpVtbl->RSSetScissorRects(list->impl._commandList, 1, (D3D12_RECT *)&currentRenderTarget->impl.scissor);
	}
	else {
		D3D12_RECT scissor;
		scissor.left = 0;
		scissor.top = 0;
		scissor.right = kinc_window_width(0);
		scissor.bottom = kinc_window_height(0);
		list->impl._commandList->lpVtbl->RSSetScissorRects(list->impl._commandList, 1, &scissor);
	}
}

void kinc_g5_command_list_set_pipeline(struct kinc_g5_command_list *list, kinc_g5_pipeline_t *pipeline) {
	list->impl._currentPipeline = pipeline;
	list->impl._commandList->lpVtbl->SetPipelineState(list->impl._commandList, pipeline->impl.pso);
	for (int i = 0; i < textureCount; ++i) {
		currentRenderTargets[i] = NULL;
		currentTextures[i] = NULL;
	}
}

void kinc_g5_command_list_set_vertex_buffers(struct kinc_g5_command_list *list, kinc_g5_vertex_buffer_t **buffers, int *offsets, int count) {
	D3D12_VERTEX_BUFFER_VIEW *views = (D3D12_VERTEX_BUFFER_VIEW *)alloca(sizeof(D3D12_VERTEX_BUFFER_VIEW) * count);
	ZeroMemory(views, sizeof(D3D12_VERTEX_BUFFER_VIEW) * count);
	for (int i = 0; i < count; ++i) {
		views[i].BufferLocation =
		    buffers[i]->impl.uploadBuffer->lpVtbl->GetGPUVirtualAddress(buffers[i]->impl.uploadBuffer) + offsets[i] * kinc_g5_vertex_buffer_stride(buffers[i]);
		views[i].SizeInBytes = (kinc_g5_vertex_buffer_count(buffers[i]) - offsets[i]) * kinc_g5_vertex_buffer_stride(buffers[i]);
		views[i].StrideInBytes = kinc_g5_vertex_buffer_stride(buffers[i]); // * kinc_g5_vertex_buffer_count(buffers[i]);
	}
	list->impl._commandList->lpVtbl->IASetVertexBuffers(list->impl._commandList, 0, count, views);
}

void kinc_g5_command_list_set_index_buffer(struct kinc_g5_command_list *list, kinc_g5_index_buffer_t *buffer) {
	list->impl._indexCount = kinc_g5_index_buffer_count(buffer);
	list->impl._commandList->lpVtbl->IASetIndexBuffer(list->impl._commandList, (D3D12_INDEX_BUFFER_VIEW *)&buffer->impl.index_buffer_view);
}

void kinc_g5_command_list_set_render_targets(struct kinc_g5_command_list *list, kinc_g5_render_target_t **targets, int count) {
	currentRenderTarget = targets[0];
	currentRenderTargetCount = count;
	for (int i = 0; i < count; ++i) {
		targetDescriptors[i] = GetCPUDescriptorHandle(targets[i]->impl.renderTargetDescriptorHeap);
	}
	graphicsFlushAndWait(list, list->impl._commandAllocator);
}

void kinc_g5_command_list_upload_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer) {}

void kinc_g5_command_list_upload_index_buffer(kinc_g5_command_list_t *list, kinc_g5_index_buffer_t *buffer) {
	kinc_g5_internal_index_buffer_upload(buffer, list->impl._commandList);
}

void kinc_g5_command_list_upload_texture(kinc_g5_command_list_t *list, kinc_g5_texture_t *texture) {
	D3D12_RESOURCE_DESC Desc = D3D12ResourceGetDesc(texture->impl.image);
	ID3D12Device *device;
	texture->impl.image->lpVtbl->GetDevice(texture->impl.image, &IID_ID3D12Device, &device);
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	device->lpVtbl->GetCopyableFootprints(device, &Desc, 0, 1, 0, &footprint, NULL, NULL, NULL);
	device->lpVtbl->Release(device);

	D3D12_TEXTURE_COPY_LOCATION source = {0};
	source.pResource = texture->impl.uploadImage;
	source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	source.PlacedFootprint = footprint;

	D3D12_TEXTURE_COPY_LOCATION destination = {0};
	destination.pResource = texture->impl.image;
	destination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	destination.SubresourceIndex = 0;

	list->impl._commandList->lpVtbl->CopyTextureRegion(list->impl._commandList, &destination, 0, 0, 0, &source, NULL);

	D3D12_RESOURCE_BARRIER transition = {0};
	transition.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	transition.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	transition.Transition.pResource = texture->impl.image;
	transition.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	transition.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	transition.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &transition);
}

#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
static int d3d12_textureAlignment() {
	return D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
}
#else
int d3d12_textureAlignment();
#endif

void kinc_g5_command_list_get_render_target_pixels(kinc_g5_command_list_t *list, kinc_g5_render_target_t *render_target, uint8_t *data) {
	DXGI_FORMAT dxgiFormat = D3D12ResourceGetDesc(render_target->impl.renderTarget).Format;
	int formatByteSize = formatSize(dxgiFormat);
	int rowPitch = render_target->texWidth * formatByteSize;
	int align = rowPitch % d3d12_textureAlignment();
	if (align != 0) rowPitch = rowPitch + (d3d12_textureAlignment() - align);

	// Create readback buffer
	if (render_target->impl.renderTargetReadback == NULL) {
		D3D12_HEAP_PROPERTIES heapProperties;
		heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1;
		heapProperties.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC resourceDesc;
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = rowPitch * render_target->texHeight;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		device->lpVtbl->CreateCommittedResource(device, &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, NULL,
		                                        &IID_ID3D12Resource, &render_target->impl.renderTargetReadback);
	}

	// Copy render target to readback buffer
	D3D12_RESOURCE_STATES sourceState = render_target->impl.resourceState == RenderTargetResourceStateRenderTarget ? D3D12_RESOURCE_STATE_RENDER_TARGET
	                                                                                                               : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = render_target->impl.renderTarget;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = sourceState;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &barrier);
	}

	D3D12_TEXTURE_COPY_LOCATION source;
	source.pResource = render_target->impl.renderTarget;
	source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	source.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION dest;
	dest.pResource = render_target->impl.renderTargetReadback;
	dest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	dest.PlacedFootprint.Offset = 0;
	dest.PlacedFootprint.Footprint.Format = dxgiFormat;
	dest.PlacedFootprint.Footprint.Width = render_target->texWidth;
	dest.PlacedFootprint.Footprint.Height = render_target->texHeight;
	dest.PlacedFootprint.Footprint.Depth = 1;
	dest.PlacedFootprint.Footprint.RowPitch = rowPitch;

	list->impl._commandList->lpVtbl->CopyTextureRegion(list->impl._commandList, &dest, 0, 0, 0, &source, NULL);

	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = render_target->impl.renderTarget;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.StateAfter = sourceState;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &barrier);
	}

	graphicsFlushAndWait(list, list->impl._commandAllocator);

	// Read buffer
	void *p;
	render_target->impl.renderTargetReadback->lpVtbl->Map(render_target->impl.renderTargetReadback, 0, NULL, &p);
	memcpy(data, p, render_target->texWidth * render_target->texHeight * formatByteSize);
	render_target->impl.renderTargetReadback->lpVtbl->Unmap(render_target->impl.renderTargetReadback, 0, NULL);
}

void kinc_g5_command_list_compute(kinc_g5_command_list_t *list, int x, int y, int z) {
	list->impl._commandList->lpVtbl->Dispatch(list->impl._commandList, x, y, z);
}

void kinc_g5_command_list_set_texture_addressing(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_direction_t dir,
                                                 kinc_g5_texture_addressing_t addressing) {}

void kinc_g5_command_list_set_texture_magnification_filter(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t texunit, kinc_g5_texture_filter_t filter) {
	bilinearFiltering = filter != KINC_G5_TEXTURE_FILTER_POINT;
}

void kinc_g5_command_list_set_texture_minification_filter(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t texunit, kinc_g5_texture_filter_t filter) {
	bilinearFiltering = filter != KINC_G5_TEXTURE_FILTER_POINT;
}

void kinc_g5_command_list_set_texture_mipmap_filter(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t texunit, kinc_g5_mipmap_filter_t filter) {}

void kinc_g5_command_list_set_render_target_face(kinc_g5_command_list_t *list, kinc_g5_render_target_t *texture, int face) {}

/*
void Graphics5::setVertexBuffers(VertexBuffer** buffers, int count) {
    buffers[0]->_set(0);
}

void Graphics5::setIndexBuffer(IndexBuffer& buffer) {
    buffer._set();
}
*/

void kinc_g5_command_list_set_texture(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {
	kinc_g5_internal_texture_set(texture, unit.impl.unit);
}

bool kinc_g5_command_list_init_occlusion_query(kinc_g5_command_list_t *list, unsigned *occlusionQuery) {
	return false;
}

void kinc_g5_command_list_set_image_texture(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {}

void kinc_g5_command_list_delete_occlusion_query(kinc_g5_command_list_t *list, unsigned occlusionQuery) {}

void kinc_g5_command_list_render_occlusion_query(kinc_g5_command_list_t *list, unsigned occlusionQuery, int triangles) {}

bool kinc_g5_command_list_are_query_results_available(kinc_g5_command_list_t *list, unsigned occlusionQuery) {
	return false;
}

void kinc_g5_command_list_get_query_result(kinc_g5_command_list_t *list, unsigned occlusionQuery, unsigned *pixelCount) {}

/*void Graphics5::setPipeline(PipelineState* pipeline) {
    pipeline->set(pipeline);
}*/
