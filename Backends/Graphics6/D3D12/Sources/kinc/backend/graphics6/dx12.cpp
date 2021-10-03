#include "dx12.h"
#include <kinc/graphics6/graphics.h>

void kinc_g6_init() {
	D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void **)&context.device);
}

void kinc_g6_destroy() {
	context.device->Release();
}

void kinc_g6_swapchain_init(kinc_g6_swapchain_t *swapchain, int window, int width, int height) {}
void kinc_g6_swapchain_resize(kinc_g6_swapchain_t *swapchain, int width, int height) {}
void kinc_g6_swapchain_destroy(kinc_g6_swapchain_t *swapchain) {}

struct kinc_g6_texture *kinc_g6_swapchain_next_texture(kinc_g6_swapchain_t *swapchain) {}

void kinc_g6_submit(kinc_g6_swapchain_t *swapchain, struct kinc_g6_command_buffer **buffers, int count) {}
void kinc_g6_present(kinc_g6_swapchain_t *swapchain) {}