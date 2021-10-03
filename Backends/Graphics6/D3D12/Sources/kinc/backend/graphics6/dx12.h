#include <d3d12.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct d3d12_context {
	ID3D12Device *device;
} d3d12_context_t;

extern d3d12_context_t context;

#ifdef __cplusplus
}
#endif
