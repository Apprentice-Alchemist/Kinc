#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/shader.h>
#include <kinc/graphics4/vertexbuffer.h>
#include <kinc/libs/stb_sprintf.h>
#include <kinc/log.h>
#include <kinc/memory.h>
#include <kinc/string.h>

kinc_g4_pipeline_t *currentPipeline = NULL;

static D3D11_CULL_MODE convert_cull_mode(kinc_g4_cull_mode_t cullMode) {
	switch (cullMode) {
	case KINC_G4_CULL_CLOCKWISE:
		return D3D11_CULL_FRONT;
	case KINC_G4_CULL_COUNTER_CLOCKWISE:
		return D3D11_CULL_BACK;
	case KINC_G4_CULL_NOTHING:
		return D3D11_CULL_NONE;
	default:
		assert(false);
		return D3D11_CULL_NONE;
	}
}

static D3D11_BLEND convert_blend_operation(kinc_g4_blending_operation_t operation) {
	switch (operation) {
	case KINC_G4_BLEND_ONE:
		return D3D11_BLEND_ONE;
	case KINC_G4_BLEND_ZERO:
		return D3D11_BLEND_ZERO;
	case KINC_G4_BLEND_SOURCE_ALPHA:
		return D3D11_BLEND_SRC_ALPHA;
	case KINC_G4_BLEND_DEST_ALPHA:
		return D3D11_BLEND_DEST_ALPHA;
	case KINC_G4_BLEND_INV_SOURCE_ALPHA:
		return D3D11_BLEND_INV_SRC_ALPHA;
	case KINC_G4_BLEND_INV_DEST_ALPHA:
		return D3D11_BLEND_INV_DEST_ALPHA;
	case KINC_G4_BLEND_SOURCE_COLOR:
		return D3D11_BLEND_SRC_COLOR;
	case KINC_G4_BLEND_DEST_COLOR:
		return D3D11_BLEND_DEST_COLOR;
	case KINC_G4_BLEND_INV_SOURCE_COLOR:
		return D3D11_BLEND_INV_SRC_COLOR;
	case KINC_G4_BLEND_INV_DEST_COLOR:
		return D3D11_BLEND_INV_DEST_COLOR;
	default:
		//	throw Exception("Unknown blending operation.");
		return D3D11_BLEND_SRC_ALPHA;
	}
}

static D3D11_STENCIL_OP get_stencil_action(kinc_g4_stencil_action_t action) {
	switch (action) {
	default:
	case KINC_G4_STENCIL_KEEP:
		return D3D11_STENCIL_OP_KEEP;
	case KINC_G4_STENCIL_ZERO:
		return D3D11_STENCIL_OP_ZERO;
	case KINC_G4_STENCIL_REPLACE:
		return D3D11_STENCIL_OP_REPLACE;
	case KINC_G4_STENCIL_INCREMENT:
		return D3D11_STENCIL_OP_INCR;
	case KINC_G4_STENCIL_INCREMENT_WRAP:
		return D3D11_STENCIL_OP_INCR_SAT;
	case KINC_G4_STENCIL_DECREMENT:
		return D3D11_STENCIL_OP_DECR;
	case KINC_G4_STENCIL_DECREMENT_WRAP:
		return D3D11_STENCIL_OP_DECR_SAT;
	case KINC_G4_STENCIL_INVERT:
		return D3D11_STENCIL_OP_INVERT;
	}
}

void kinc_internal_set_constants(void) {
	if (currentPipeline->vertex_shader->impl.constantsSize > 0) {
		context->lpVtbl->UpdateSubresource(context, (ID3D11Resource *)currentPipeline->impl.vertexConstantBuffer, 0, NULL, vertexConstants, 0, 0);
		context->lpVtbl->VSSetConstantBuffers(context, 0, 1, &currentPipeline->impl.vertexConstantBuffer);
	}
	if (currentPipeline->fragment_shader->impl.constantsSize > 0) {
		context->lpVtbl->UpdateSubresource(context, (ID3D11Resource *)currentPipeline->impl.fragmentConstantBuffer, 0, NULL, fragmentConstants, 0, 0);
		context->lpVtbl->PSSetConstantBuffers(context, 0, 1, &currentPipeline->impl.fragmentConstantBuffer);
	}
	if (currentPipeline->geometry_shader != NULL && currentPipeline->geometry_shader->impl.constantsSize > 0) {
		context->lpVtbl->UpdateSubresource(context, (ID3D11Resource *)currentPipeline->impl.geometryConstantBuffer, 0, NULL, geometryConstants, 0, 0);
		context->lpVtbl->GSSetConstantBuffers(context, 0, 1, &currentPipeline->impl.geometryConstantBuffer);
	}
	if (currentPipeline->tessellation_control_shader != NULL && currentPipeline->tessellation_control_shader->impl.constantsSize > 0) {
		context->lpVtbl->UpdateSubresource(context, (ID3D11Resource *)currentPipeline->impl.tessControlConstantBuffer, 0, NULL, tessControlConstants, 0, 0);
		context->lpVtbl->HSSetConstantBuffers(context, 0, 1, &currentPipeline->impl.tessControlConstantBuffer);
	}
	if (currentPipeline->tessellation_evaluation_shader != NULL && currentPipeline->tessellation_evaluation_shader->impl.constantsSize > 0) {
		context->lpVtbl->UpdateSubresource(context, (ID3D11Resource *)currentPipeline->impl.tessEvalConstantBuffer, 0, NULL, tessEvalConstants, 0, 0);
		context->lpVtbl->DSSetConstantBuffers(context, 0, 1, &currentPipeline->impl.tessEvalConstantBuffer);
	}
}

void kinc_g4_pipeline_init(struct kinc_g4_pipeline *state) {
	kinc_memset(state, 0, sizeof(struct kinc_g4_pipeline));
	kinc_g4_internal_pipeline_set_defaults(state);
	state->impl.d3d11inputLayout = NULL;
	state->impl.fragmentConstantBuffer = NULL;
	state->impl.vertexConstantBuffer = NULL;
	state->impl.geometryConstantBuffer = NULL;
	state->impl.tessEvalConstantBuffer = NULL;
	state->impl.tessControlConstantBuffer = NULL;
	state->impl.depthStencilState = NULL;
	state->impl.rasterizerState = NULL;
	state->impl.rasterizerStateScissor = NULL;
	state->impl.blendState = NULL;
}

void kinc_g4_pipeline_destroy(struct kinc_g4_pipeline *state) {
	if (state->impl.d3d11inputLayout != NULL) {
		state->impl.d3d11inputLayout->lpVtbl->Release(state->impl.d3d11inputLayout);
		state->impl.d3d11inputLayout = NULL;
	}
	if (state->impl.fragmentConstantBuffer != NULL) {
		state->impl.fragmentConstantBuffer->lpVtbl->Release(state->impl.fragmentConstantBuffer);
		state->impl.fragmentConstantBuffer = NULL;
	}
	if (state->impl.vertexConstantBuffer != NULL) {
		state->impl.vertexConstantBuffer->lpVtbl->Release(state->impl.vertexConstantBuffer);
		state->impl.vertexConstantBuffer = NULL;
	}
	if (state->impl.geometryConstantBuffer != NULL) {
		state->impl.geometryConstantBuffer->lpVtbl->Release(state->impl.geometryConstantBuffer);
		state->impl.geometryConstantBuffer = NULL;
	}
	if (state->impl.tessEvalConstantBuffer != NULL) {
		state->impl.tessEvalConstantBuffer->lpVtbl->Release(state->impl.tessEvalConstantBuffer);
		state->impl.tessEvalConstantBuffer = NULL;
	}
	if (state->impl.tessControlConstantBuffer != NULL) {
		state->impl.tessControlConstantBuffer->lpVtbl->Release(state->impl.tessControlConstantBuffer);
		state->impl.tessControlConstantBuffer = NULL;
	}
	if (state->impl.depthStencilState != NULL) {
		state->impl.depthStencilState->lpVtbl->Release(state->impl.depthStencilState);
		state->impl.depthStencilState = NULL;
	}
	if (state->impl.rasterizerState != NULL) {
		state->impl.rasterizerState->lpVtbl->Release(state->impl.rasterizerState);
		state->impl.rasterizerState = NULL;
	}
	if (state->impl.rasterizerStateScissor != NULL) {
		state->impl.rasterizerStateScissor->lpVtbl->Release(state->impl.rasterizerStateScissor);
		state->impl.rasterizerStateScissor = NULL;
	}
	if (state->impl.blendState != NULL) {
		state->impl.blendState->lpVtbl->Release(state->impl.blendState);
		state->impl.blendState = NULL;
	}
}

void kinc_internal_set_rasterizer_state(struct kinc_g4_pipeline *pipeline, bool scissoring) {
	if (scissoring && pipeline->impl.rasterizerStateScissor != NULL)
		context->lpVtbl->RSSetState(context, pipeline->impl.rasterizerStateScissor);
	else if (pipeline->impl.rasterizerState != NULL)
		context->lpVtbl->RSSetState(context, pipeline->impl.rasterizerState);
}

void kinc_internal_set_pipeline(struct kinc_g4_pipeline *pipeline, bool scissoring) {
	currentPipeline = pipeline;

	context->lpVtbl->OMSetDepthStencilState(context, pipeline->impl.depthStencilState, pipeline->stencil_reference_value);
	float blendFactor[] = {0, 0, 0, 0};
	UINT sampleMask = 0xffffffff;
	context->lpVtbl->OMSetBlendState(context, pipeline->impl.blendState, blendFactor, sampleMask);
	kinc_internal_set_rasterizer_state(pipeline, scissoring);

	context->lpVtbl->VSSetShader(context, (ID3D11VertexShader *)pipeline->vertex_shader->impl.shader, NULL, 0);
	context->lpVtbl->PSSetShader(context, (ID3D11PixelShader *)pipeline->fragment_shader->impl.shader, NULL, 0);

	context->lpVtbl->GSSetShader(context, pipeline->geometry_shader != NULL ? (ID3D11GeometryShader *)pipeline->geometry_shader->impl.shader : NULL, NULL, 0);
	context->lpVtbl->HSSetShader(
	    context, pipeline->tessellation_control_shader != NULL ? (ID3D11HullShader *)pipeline->tessellation_control_shader->impl.shader : NULL, NULL, 0);
	context->lpVtbl->DSSetShader(
	    context, pipeline->tessellation_evaluation_shader != NULL ? (ID3D11DomainShader *)pipeline->tessellation_evaluation_shader->impl.shader : NULL, NULL,
	    0);

	context->lpVtbl->IASetInputLayout(context, pipeline->impl.d3d11inputLayout);
}

static kinc_internal_shader_constant_t *findConstant(kinc_internal_shader_constant_t *constants, uint32_t hash) {
	for (int i = 0; i < 64; ++i) {
		if (constants[i].hash == hash) {
			return &constants[i];
		}
	}
	return NULL;
}

static kinc_internal_hash_index_t *findTextureUnit(kinc_internal_hash_index_t *units, uint32_t hash) {
	for (int i = 0; i < 64; ++i) {
		if (units[i].hash == hash) {
			return &units[i];
		}
	}
	return NULL;
}

kinc_g4_constant_location_t kinc_g4_pipeline_get_constant_location(struct kinc_g4_pipeline *state, const char *name) {
	kinc_g4_constant_location_t location;

	uint32_t hash = kinc_internal_hash_name((unsigned char *)name);

	kinc_internal_shader_constant_t *constant = findConstant(state->vertex_shader->impl.constants, hash);
	if (constant == NULL) {
		location.impl.vertexOffset = 0;
		location.impl.vertexSize = 0;
		location.impl.vertexColumns = 0;
		location.impl.vertexRows = 0;
	}
	else {
		location.impl.vertexOffset = constant->offset;
		location.impl.vertexSize = constant->size;
		location.impl.vertexColumns = constant->columns;
		location.impl.vertexRows = constant->rows;
	}

	constant = findConstant(state->fragment_shader->impl.constants, hash);
	if (constant == NULL) {
		location.impl.fragmentOffset = 0;
		location.impl.fragmentSize = 0;
		location.impl.fragmentColumns = 0;
		location.impl.fragmentRows = 0;
	}
	else {
		location.impl.fragmentOffset = constant->offset;
		location.impl.fragmentSize = constant->size;
		location.impl.fragmentColumns = constant->columns;
		location.impl.fragmentRows = constant->rows;
	}

	constant = state->geometry_shader == NULL ? NULL : findConstant(state->geometry_shader->impl.constants, hash);
	if (constant == NULL) {
		location.impl.geometryOffset = 0;
		location.impl.geometrySize = 0;
		location.impl.geometryColumns = 0;
		location.impl.geometryRows = 0;
	}
	else {
		location.impl.geometryOffset = constant->offset;
		location.impl.geometrySize = constant->size;
		location.impl.geometryColumns = constant->columns;
		location.impl.geometryRows = constant->rows;
	}

	constant = state->tessellation_control_shader == NULL ? NULL : findConstant(state->tessellation_control_shader->impl.constants, hash);
	if (constant == NULL) {
		location.impl.tessControlOffset = 0;
		location.impl.tessControlSize = 0;
		location.impl.tessControlColumns = 0;
		location.impl.tessControlRows = 0;
	}
	else {
		location.impl.tessControlOffset = constant->offset;
		location.impl.tessControlSize = constant->size;
		location.impl.tessControlColumns = constant->columns;
		location.impl.tessControlRows = constant->rows;
	}

	constant = state->tessellation_evaluation_shader == NULL ? NULL : findConstant(state->tessellation_evaluation_shader->impl.constants, hash);
	if (constant == NULL) {
		location.impl.tessEvalOffset = 0;
		location.impl.tessEvalSize = 0;
		location.impl.tessEvalColumns = 0;
		location.impl.tessEvalRows = 0;
	}
	else {
		location.impl.tessEvalOffset = constant->offset;
		location.impl.tessEvalSize = constant->size;
		location.impl.tessEvalColumns = constant->columns;
		location.impl.tessEvalRows = constant->rows;
	}

	if (location.impl.vertexSize == 0 && location.impl.fragmentSize == 0 && location.impl.geometrySize == 0 && location.impl.tessControlSize &&
	    location.impl.tessEvalSize == 0) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Uniform %s not found.", name);
	}

	return location;
}

kinc_g4_texture_unit_t kinc_g4_pipeline_get_texture_unit(struct kinc_g4_pipeline *state, const char *name) {
	char unitName[64];
	int unitOffset = 0;
	size_t len = kinc_string_length(name);
	if (len > 63) len = 63;
	kinc_string_copy_limited(unitName, name, len + 1);
	if (unitName[len - 1] == ']') {                  // Check for array - mySampler[2]
		unitOffset = (int)(unitName[len - 2] - '0'); // Array index is unit offset
		unitName[len - 3] = 0;                       // Strip array from name
	}

	uint32_t hash = kinc_internal_hash_name((unsigned char *)unitName);

	kinc_g4_texture_unit_t unit;
	kinc_internal_hash_index_t *vertexUnit = findTextureUnit(state->vertex_shader->impl.textures, hash);
	if (vertexUnit == NULL) {
		kinc_internal_hash_index_t *fragmentUnit = findTextureUnit(state->fragment_shader->impl.textures, hash);
		if (fragmentUnit == NULL) {
			unit.impl.unit = -1;
#ifndef NDEBUG
			static int notFoundCount = 0;
			if (notFoundCount < 10) {
				kinc_log(KINC_LOG_LEVEL_WARNING, "Sampler %s not found.", unitName);
				++notFoundCount;
			}
			else if (notFoundCount == 10) {
				kinc_log(KINC_LOG_LEVEL_WARNING, "Giving up on sampler not found messages.", unitName);
				++notFoundCount;
			}
#endif
		}
		else {
			unit.impl.unit = fragmentUnit->index + unitOffset;
			unit.impl.vertex = false;
		}
	}
	else {
		unit.impl.unit = vertexUnit->index + unitOffset;
		unit.impl.vertex = true;
	}
	return unit;
}

static char stringCache[1024];
static int stringCacheIndex = 0;

static int getMultipleOf16(int value) {
	int ret = 16;
	while (ret < value) ret += 16;
	return ret;
}

static void setVertexDesc(D3D11_INPUT_ELEMENT_DESC *vertexDesc, int attributeIndex, int index, int stream, bool instanced, int subindex) {
	if (subindex < 0) {
		vertexDesc->SemanticName = "TEXCOORD";
		vertexDesc->SemanticIndex = attributeIndex;
	}
	else {
		// SPIRV_CROSS uses TEXCOORD_0_0,... for split up matrices
		int stringStart = stringCacheIndex;
		kinc_string_copy(&stringCache[stringCacheIndex], "TEXCOORD");
		stringCacheIndex += (int)kinc_string_length("TEXCOORD");
		stbsp_sprintf(&stringCache[stringCacheIndex], "%i", attributeIndex);
		stringCacheIndex += (int)kinc_string_length(&stringCache[stringCacheIndex]);
		kinc_string_copy(&stringCache[stringCacheIndex], "_");
		stringCacheIndex += 2;
		vertexDesc->SemanticName = &stringCache[stringStart];
		vertexDesc->SemanticIndex = subindex;
	}
	vertexDesc->InputSlot = stream;
	vertexDesc->AlignedByteOffset = (index == 0) ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc->InputSlotClass = instanced ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
	vertexDesc->InstanceDataStepRate = instanced ? 1 : 0;
}

#define usedCount 32

static int getAttributeLocation(kinc_internal_hash_index_t *attributes, const char *name, bool *used) {
	uint32_t hash = kinc_internal_hash_name((unsigned char *)name);

	for (int i = 0; i < 64; ++i) {
		if (attributes[i].hash == hash) {
			return attributes[i].index;
		}
	}

	for (int i = 0; i < usedCount; ++i) {
		if (!used[i]) {
			used[i] = true;
			return i;
		}
	}

	return 0;
}

static void createRenderTargetBlendDesc(struct kinc_g4_pipeline *pipe, D3D11_RENDER_TARGET_BLEND_DESC *rtbd, int targetNum) {
	rtbd->BlendEnable = pipe->blend_source != KINC_G4_BLEND_ONE || pipe->blend_destination != KINC_G4_BLEND_ZERO ||
	                    pipe->alpha_blend_source != KINC_G4_BLEND_ONE || pipe->alpha_blend_destination != KINC_G4_BLEND_ZERO;
	rtbd->SrcBlend = convert_blend_operation(pipe->blend_source);
	rtbd->DestBlend = convert_blend_operation(pipe->blend_destination);
	rtbd->BlendOp = D3D11_BLEND_OP_ADD;
	rtbd->SrcBlendAlpha = convert_blend_operation(pipe->alpha_blend_source);
	rtbd->DestBlendAlpha = convert_blend_operation(pipe->alpha_blend_destination);
	rtbd->BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd->RenderTargetWriteMask = (((pipe->color_write_mask_red[targetNum] ? D3D11_COLOR_WRITE_ENABLE_RED : 0) |
	                                (pipe->color_write_mask_green[targetNum] ? D3D11_COLOR_WRITE_ENABLE_GREEN : 0)) |
	                               (pipe->color_write_mask_blue[targetNum] ? D3D11_COLOR_WRITE_ENABLE_BLUE : 0)) |
	                              (pipe->color_write_mask_alpha[targetNum] ? D3D11_COLOR_WRITE_ENABLE_ALPHA : 0);
}

void kinc_g4_pipeline_compile(struct kinc_g4_pipeline *state) {
	if (state->vertex_shader->impl.constantsSize > 0) {
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = getMultipleOf16(state->vertex_shader->impl.constantsSize);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		kinc_microsoft_affirm(device->lpVtbl->CreateBuffer(device, &desc, NULL, &state->impl.vertexConstantBuffer));
	}
	if (state->fragment_shader->impl.constantsSize > 0) {
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = getMultipleOf16(state->fragment_shader->impl.constantsSize);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		kinc_microsoft_affirm(device->lpVtbl->CreateBuffer(device, &desc, NULL, &state->impl.fragmentConstantBuffer));
	}
	if (state->geometry_shader != NULL && state->geometry_shader->impl.constantsSize > 0) {
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = getMultipleOf16(state->geometry_shader->impl.constantsSize);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		kinc_microsoft_affirm(device->lpVtbl->CreateBuffer(device, &desc, NULL, &state->impl.geometryConstantBuffer));
	}
	if (state->tessellation_control_shader != NULL && state->tessellation_control_shader->impl.constantsSize > 0) {
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = getMultipleOf16(state->tessellation_control_shader->impl.constantsSize);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		kinc_microsoft_affirm(device->lpVtbl->CreateBuffer(device, &desc, NULL, &state->impl.tessControlConstantBuffer));
	}
	if (state->tessellation_evaluation_shader != NULL && state->tessellation_evaluation_shader->impl.constantsSize > 0) {
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = getMultipleOf16(state->tessellation_evaluation_shader->impl.constantsSize);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		kinc_microsoft_affirm(device->lpVtbl->CreateBuffer(device, &desc, NULL, &state->impl.tessEvalConstantBuffer));
	}

	int all = 0;
	for (int stream = 0; state->input_layout[stream] != NULL; ++stream) {
		for (int index = 0; index < state->input_layout[stream]->size; ++index) {
			if (state->input_layout[stream]->elements[index].data == KINC_G4_VERTEX_DATA_FLOAT4X4) {
				all += 4;
			}
			else {
				all += 1;
			}
		}
	}

	bool used[usedCount];
	for (int i = 0; i < usedCount; ++i) used[i] = false;
	for (int i = 0; i < 64; ++i) {
		used[state->vertex_shader->impl.attributes[i].index] = true;
	}
	stringCacheIndex = 0;
	D3D11_INPUT_ELEMENT_DESC *vertexDesc = (D3D11_INPUT_ELEMENT_DESC *)alloca(sizeof(D3D11_INPUT_ELEMENT_DESC) * all);
	int i = 0;
	for (int stream = 0; state->input_layout[stream] != NULL; ++stream) {
		for (int index = 0; index < state->input_layout[stream]->size; ++index) {
			switch (state->input_layout[stream]->elements[index].data) {
			case KINC_G4_VERTEX_DATA_FLOAT1:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R32_FLOAT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_FLOAT2:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32_FLOAT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_FLOAT3:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_FLOAT4:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_SHORT2_NORM:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R16G16_SNORM;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_SHORT4_NORM:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R16G16B16A16_SNORM;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_COLOR:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_FLOAT4X4: {
				char name[101];
				kinc_string_copy(name, state->input_layout[stream]->elements[index].name);
				kinc_string_append(name, "_");
				size_t length = kinc_string_length(name);
				stbsp_sprintf(&name[length], "%i", 0);
				name[length + 1] = 0;
				int attributeLocation = getAttributeLocation(state->vertex_shader->impl.attributes, name, used);

				for (int i2 = 0; i2 < 4; ++i2) {
					setVertexDesc(&vertexDesc[i], attributeLocation, index + i2, stream, state->input_layout[stream]->instanced, i2);
					vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					++i;
				}
				break;
			}
			}
		}
	}

	kinc_microsoft_affirm(device->lpVtbl->CreateInputLayout(device, vertexDesc, all, state->vertex_shader->impl.data, state->vertex_shader->impl.length,
	                                                        &state->impl.d3d11inputLayout));

	{
		D3D11_DEPTH_STENCIL_DESC desc;
		kinc_memset(&desc, 0, sizeof(desc));
		desc.DepthEnable = state->depth_mode != KINC_G4_COMPARE_ALWAYS;
		desc.DepthWriteMask = state->depth_write ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc = get_comparison(state->depth_mode);

		desc.StencilEnable = state->stencil_mode != KINC_G4_COMPARE_ALWAYS;
		desc.StencilReadMask = state->stencil_read_mask;
		desc.StencilWriteMask = state->stencil_write_mask;
		desc.FrontFace.StencilFunc = desc.BackFace.StencilFunc = get_comparison(state->stencil_mode);
		desc.FrontFace.StencilDepthFailOp = desc.BackFace.StencilDepthFailOp = get_stencil_action(state->stencil_depth_fail);
		desc.FrontFace.StencilPassOp = desc.BackFace.StencilPassOp = get_stencil_action(state->stencil_both_pass);
		desc.FrontFace.StencilFailOp = desc.BackFace.StencilFailOp = get_stencil_action(state->stencil_fail);

		device->lpVtbl->CreateDepthStencilState(device, &desc, &state->impl.depthStencilState);
	}

	{
		D3D11_RASTERIZER_DESC rasterDesc;
		rasterDesc.CullMode = convert_cull_mode(state->cull_mode);
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = FALSE;
		rasterDesc.DepthBias = 0;
		rasterDesc.SlopeScaledDepthBias = 0.0f;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = TRUE;
		rasterDesc.ScissorEnable = FALSE;
		rasterDesc.MultisampleEnable = FALSE;
		rasterDesc.AntialiasedLineEnable = FALSE;

		device->lpVtbl->CreateRasterizerState(device, &rasterDesc, &state->impl.rasterizerState);
		rasterDesc.ScissorEnable = TRUE;
		device->lpVtbl->CreateRasterizerState(device, &rasterDesc, &state->impl.rasterizerStateScissor);

		// We need d3d11_3 for conservative raster
		// D3D11_RASTERIZER_DESC2 rasterDesc;
		// rasterDesc.ConservativeRaster = conservativeRasterization ? D3D11_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D11_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		// device->CreateRasterizerState2(&rasterDesc, &rasterizerState);
		// rasterDesc.ScissorEnable = TRUE;
		// device->CreateRasterizerState2(&rasterDesc, &rasterizerStateScissor);
	}

	{
		bool independentBlend = false;
		for (int i = 1; i < 8; ++i) {
			if (state->color_write_mask_red[0] != state->color_write_mask_red[i] || state->color_write_mask_green[0] != state->color_write_mask_green[i] ||
			    state->color_write_mask_blue[0] != state->color_write_mask_blue[i] || state->color_write_mask_alpha[0] != state->color_write_mask_alpha[i]) {
				independentBlend = true;
				break;
			}
		}

		D3D11_BLEND_DESC blendDesc;
		kinc_memset(&blendDesc, 0, sizeof(blendDesc));
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = independentBlend;

		D3D11_RENDER_TARGET_BLEND_DESC rtbd[8];
		kinc_memset(&rtbd, 0, sizeof(rtbd));
		createRenderTargetBlendDesc(state, &rtbd[0], 0);
		blendDesc.RenderTarget[0] = rtbd[0];
		if (independentBlend) {
			for (int i = 1; i < 8; ++i) {
				createRenderTargetBlendDesc(state, &rtbd[i], i);
				blendDesc.RenderTarget[i] = rtbd[i];
			}
		}

		device->lpVtbl->CreateBlendState(device, &blendDesc, &state->impl.blendState);
	}
}
