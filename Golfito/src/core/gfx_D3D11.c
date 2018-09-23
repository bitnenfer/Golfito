#include <Windows.h>
#include <windowsx.h>
#include <dxgi.h>
#include <d3d11.h>

#include "gfx.h"
#include "../config/config_gfx.h"
#include "math.h"
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "assert.h"
#include "../win32/shaders/TextureColor_PS.h"
#include "../win32/shaders/TextureColor_VS.h"
#include "../win32/shaders/LineColor_PS.h"
#include "../win32/shaders/LineColor_VS.h"

#define MAX_MATRICES 100
#define MAX_PIPELINES 2
#define MAX_QUADS 10000
#define BATCH_COUNT 1000
#define VERTEX_COUNT MAX_QUADS * 6
#define TEXTURE_COUNT 1000
#define MAX_POINTS 10000

typedef struct {
	ID3D11Texture2D* pTexture;
	ID3D11ShaderResourceView* pView;
	vec2_t size;
} Texture2D;

typedef struct {
	ID3D11VertexShader* pVertexShader;
	ID3D11PixelShader* pPixelShader;
	ID3D11InputLayout* pInputLayout;
} RenderPipeline;

typedef struct {
	mat2d_t matrices[MAX_MATRICES];
	mat2d_t matrix;
	uint32_t index;
} MatrixStack;

typedef struct {
	vec2_t position;
	vec2_t texCoord;
	uint32_t color;
} TextureColorVertex;

typedef struct {
	vec2_t position;
	uint32_t color;
} PointVertex;

typedef struct {
	mat4_t projectionMatrix;
} BaseShaderUniform;

typedef struct {
	TextureID texture;
	uint32_t vertexCount;
	uint32_t offset;
} DrawBatch;

typedef struct {
	DrawBatch* pBuffer;
	uint32_t count;
} DrawBatchBuffer;

typedef struct {
	TextureColorVertex* pBuffer;
	uint32_t count;
} TextureColorVertexBuffer;

typedef struct {
	Texture2D *pBuffer;
	uint32_t count;
} TextureBuffer;

typedef struct {
	PointVertex* pBuffer;
	uint32_t count;
} PointBuffer;

typedef struct {
	MatrixStack matrixStack;
	BaseShaderUniform uniformData;
	struct { float32_t r, g, b, a; } clearColor;
	vec2_t viewportSize;
	DrawBatchBuffer batchBuffer;
	TextureBuffer textures;
	TextureColorVertexBuffer vertices;
	PointBuffer points;
	RenderPipeline pipelines[MAX_PIPELINES];
	IDXGISwapChain* pSwapChain;
	ID3D11Device* pDevice;
	ID3D11DeviceContext* pDeviceContext;
	ID3D11RenderTargetView* pBackBufferView;
	ID3D11Buffer* pVertexBuffer;
	ID3D11Buffer* pPointVertexBuffer;
	ID3D11Buffer* pUniformBuffer;
	ID3D11SamplerState* pNeareastSampler;
	ID3D11RasterizerState* pRasterizerState;
	ID3D11BlendState* pBlendState;
	DrawBatch* pCurrentBatch;
	RenderPipeline* pCurrentPipeline;
	int32_t currentTexture;
	uint32_t pipelineID;
	float32_t pixelScale;
	HWND windowHandle;
} GfxState;

static GfxState _gfxState = { 0 };

#define C_D3D(prop, funcName, ...) _gfxState.##prop->lpVtbl->##funcName(_gfxState.##prop, ##__VA_ARGS__)

void _gfx_d3d11_swap_buffers(void) {
	C_D3D(pSwapChain, Present, (UINT)1, 0);
}

void _gfx_d3d11_initialize(HWND windowHandle) {
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	IDXGISwapChain* pSwapChain = NULL;
	ID3D11Device* pDevice = NULL;
	ID3D11DeviceContext* pDeviceContext = NULL;
	ID3D11Texture2D* pBackBufferTexture = NULL;
	ID3D11RenderTargetView* pBackBufferView = NULL;
	HRESULT result;

	_gfxState.viewportSize.x = (float32_t)GFX_DISPLAY_WIDTH;
	_gfxState.viewportSize.y = (float32_t)GFX_DISPLAY_HEIGHT;

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = windowHandle;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = 1;

#if defined(_DEBUG)
	result = D3D11CreateDeviceAndSwapChain(
		NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL, 0, D3D11_SDK_VERSION,
		&swapChainDesc, &pSwapChain,
		&pDevice, NULL, &pDeviceContext
	);
#else
	result = D3D11CreateDeviceAndSwapChain(
		NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION,
		&swapChainDesc, &pSwapChain,
		&pDevice, NULL, &pDeviceContext
	);
#endif

	DBG_ASSERT(result == S_OK, "Failed to create device and swap chain");
	result = pSwapChain->lpVtbl->GetBuffer(pSwapChain, 0, &IID_ID3D11Texture2D, (void**)&pBackBufferTexture);
	DBG_ASSERT(result == S_OK, "Failed to get buffer from swap chain.");
	result = pDevice->lpVtbl->CreateRenderTargetView(pDevice, (ID3D11Resource*)pBackBufferTexture, NULL, &pBackBufferView);
	DBG_ASSERT(result == S_OK, "Failed to create render target for back buffer.");
	pBackBufferTexture->lpVtbl->Release(pBackBufferTexture);

	// Create texture-color render pipeline
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[3] = { 0 };
		ID3D11PixelShader* pPixelShader = NULL;
		ID3D11VertexShader* pVertexShader = NULL;
		ID3D11InputLayout* pInputLayout = NULL;
		HRESULT result;

		result = pDevice->lpVtbl->CreateVertexShader(pDevice, TextureColor_VS, sizeof(TextureColor_VS), NULL, &pVertexShader);
		DBG_ASSERT(result == S_OK, "Failed to create texture-color vertex shader");

		result = pDevice->lpVtbl->CreatePixelShader(pDevice, TextureColor_PS, sizeof(TextureColor_PS), NULL, &pPixelShader);
		DBG_ASSERT(result == S_OK, "Failed to create texture-color pixel shader");

		inputElements[0].SemanticName = "POSITION";
		inputElements[0].Format = DXGI_FORMAT_R32G32_FLOAT;
		inputElements[0].AlignedByteOffset = offsetof(TextureColorVertex, position);
		inputElements[0].InputSlot = 0;
		inputElements[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputElements[0].InstanceDataStepRate = 0;
		inputElements[0].SemanticIndex = 0;

		inputElements[1].SemanticName = "TEXCOORD";
		inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		inputElements[1].AlignedByteOffset = offsetof(TextureColorVertex, texCoord);
		inputElements[1].InputSlot = 0;
		inputElements[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputElements[1].InstanceDataStepRate = 0;
		inputElements[1].SemanticIndex = 0;

		inputElements[2].SemanticName = "COLOR";
		inputElements[2].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		inputElements[2].AlignedByteOffset = offsetof(TextureColorVertex, color);
		inputElements[2].InputSlot = 0;
		inputElements[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputElements[2].InstanceDataStepRate = 0;
		inputElements[2].SemanticIndex = 0;

		result = pDevice->lpVtbl->CreateInputLayout(pDevice, inputElements, 3, TextureColor_VS, sizeof(TextureColor_VS), &pInputLayout);
		DBG_ASSERT(result == S_OK, "Failed to create input layout for texture-color pipeline");

		_gfxState.pipelines[0].pInputLayout = pInputLayout;
		_gfxState.pipelines[0].pVertexShader = pVertexShader;
		_gfxState.pipelines[0].pPixelShader = pPixelShader;
	}

	// Create line-color render pipeline
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = { 0 };
		ID3D11PixelShader* pPixelShader = NULL;
		ID3D11VertexShader* pVertexShader = NULL;
		ID3D11InputLayout* pInputLayout = NULL;
		HRESULT result;

		result = pDevice->lpVtbl->CreateVertexShader(pDevice, LineColor_VS, sizeof(LineColor_VS), NULL, &pVertexShader);
		DBG_ASSERT(result == S_OK, "Failed to create line-color vertex shader");

		result = pDevice->lpVtbl->CreatePixelShader(pDevice, LineColor_PS, sizeof(LineColor_PS), NULL, &pPixelShader);
		DBG_ASSERT(result == S_OK, "Failed to create line-color pixel shader");

		inputElements[0].SemanticName = "POSITION";
		inputElements[0].Format = DXGI_FORMAT_R32G32_FLOAT;
		inputElements[0].AlignedByteOffset = offsetof(PointVertex, position);
		inputElements[0].InputSlot = 0;
		inputElements[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputElements[0].InstanceDataStepRate = 0;
		inputElements[0].SemanticIndex = 0;

		inputElements[1].SemanticName = "COLOR";
		inputElements[1].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		inputElements[1].AlignedByteOffset = offsetof(PointVertex, color);
		inputElements[1].InputSlot = 0;
		inputElements[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputElements[1].InstanceDataStepRate = 0;
		inputElements[1].SemanticIndex = 0;

		result = pDevice->lpVtbl->CreateInputLayout(pDevice, inputElements, 2, LineColor_VS, sizeof(LineColor_VS), &pInputLayout);
		DBG_ASSERT(result == S_OK, "Failed to create input layout for line-color pipeline");

		_gfxState.pipelines[1].pInputLayout = pInputLayout;
		_gfxState.pipelines[1].pVertexShader = pVertexShader;
		_gfxState.pipelines[1].pPixelShader = pPixelShader;
	}

	// Nearest Sampler
	{
		D3D11_SAMPLER_DESC samplerDesc = { 0 };
		ID3D11SamplerState* pSampler = NULL;
		HRESULT result;

		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.MaxLOD = 0.0f;
		samplerDesc.MinLOD = 0.0f;

		result = pDevice->lpVtbl->CreateSamplerState(pDevice, &samplerDesc, &pSampler);
		DBG_ASSERT(result == S_OK, "Failed to create nearest sampler");

		_gfxState.pNeareastSampler = pSampler;
	}

	// Rasterizer State 
	{
		D3D11_RASTERIZER_DESC rasterizerStateDesc = { 0 };
		ID3D11RasterizerState* pRasterizerState = NULL;
		HRESULT result;

		rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerStateDesc.CullMode = D3D11_CULL_NONE;
		rasterizerStateDesc.ScissorEnable = FALSE;
		rasterizerStateDesc.AntialiasedLineEnable = TRUE;
		rasterizerStateDesc.DepthClipEnable = FALSE;
		rasterizerStateDesc.MultisampleEnable = FALSE;
		rasterizerStateDesc.DepthBias = 0;
		rasterizerStateDesc.DepthBiasClamp = 0.0f;
		rasterizerStateDesc.FrontCounterClockwise = FALSE;
		rasterizerStateDesc.SlopeScaledDepthBias = 0;

		result = pDevice->lpVtbl->CreateRasterizerState(pDevice, &rasterizerStateDesc, &pRasterizerState);
		DBG_ASSERT(result == S_OK, "Failed to create rasterizer state");

		_gfxState.pRasterizerState = pRasterizerState;
	}

	// Blend State
	{
		D3D11_BLEND_DESC blendStateDesc = { 0 };
		ID3D11BlendState* pBlendState = NULL;
		HRESULT result;

		blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = 0xF;

		result = pDevice->lpVtbl->CreateBlendState(pDevice, &blendStateDesc, &pBlendState);
		DBG_ASSERT(result == S_OK, "Failed to create blend state");

		_gfxState.pBlendState = pBlendState;
	}

	_gfxState.windowHandle = windowHandle;
	_gfxState.pDevice = pDevice;
	_gfxState.pDeviceContext = pDeviceContext;
	_gfxState.pBackBufferView = pBackBufferView;
	_gfxState.pSwapChain = pSwapChain;

}

HRESULT _gfx_create_buffer_with_length(UINT size, UINT stride, D3D11_BIND_FLAG bindFlag, D3D11_USAGE usage, UINT accessFlag, ID3D11Buffer** ppOutBuffer) {
	HRESULT result;
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	ID3D11Buffer* pBuffer = NULL;

	DBG_ASSERT(ppOutBuffer != NULL, "Can't save to NULL pointer to out buffer");

	bufferDesc.ByteWidth = size;
	bufferDesc.CPUAccessFlags = accessFlag;
	bufferDesc.BindFlags = bindFlag;
	bufferDesc.Usage = usage;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = stride;
	result = C_D3D(pDevice, CreateBuffer, &bufferDesc, NULL, &pBuffer);
	*ppOutBuffer = pBuffer;

	return result;
}

HRESULT _gfx_create_buffer_with_data(const void* pData, UINT size, UINT stride, D3D11_BIND_FLAG bindFlag, D3D11_USAGE usage, UINT accessFlag, ID3D11Buffer** ppOutBuffer) {
	HRESULT result;
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	D3D11_SUBRESOURCE_DATA dataDesc = { 0 };
	ID3D11Buffer* pBuffer = NULL;

	DBG_ASSERT(ppOutBuffer != NULL, "Can't save to NULL pointer to out buffer");
	DBG_ASSERT(pData != NULL, "Can't use pData since it's NULL");

	bufferDesc.ByteWidth = size;
	bufferDesc.CPUAccessFlags = accessFlag;
	bufferDesc.BindFlags = bindFlag;
	bufferDesc.Usage = usage;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = stride;

	dataDesc.pSysMem = pData;
	dataDesc.SysMemPitch = 0;
	dataDesc.SysMemSlicePitch = 0;

	result = C_D3D(pDevice, CreateBuffer, &bufferDesc, &dataDesc, &pBuffer);
	*ppOutBuffer = pBuffer;

	return result;
}

void gfx_initialize(void) {
	_gfxState.points.pBuffer = (PointVertex*)malloc(sizeof(PointVertex) * MAX_POINTS);
	_gfxState.points.count = 0;
	_gfxState.batchBuffer.pBuffer = (DrawBatch*)malloc(sizeof(DrawBatch) * BATCH_COUNT);
	_gfxState.batchBuffer.count = 0;
	_gfxState.vertices.pBuffer = (TextureColorVertex*)malloc(sizeof(TextureColorVertex) * VERTEX_COUNT);
	_gfxState.vertices.count = 0;
	_gfxState.textures.pBuffer = (Texture2D*)malloc(sizeof(ID3D11Texture2D) * TEXTURE_COUNT);
	_gfxState.textures.count = 0;
	_gfxState.currentTexture = INVALID_TEXTURE_ID;
	_gfxState.pCurrentBatch = NULL;
	DBG_ASSERT(
		_gfx_create_buffer_with_length(sizeof(PointVertex) * MAX_POINTS, sizeof(PointVertex), D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, &_gfxState.pPointVertexBuffer) == S_OK,
		"Failed to create vertex buffer for point/line rendering"
		);
	DBG_ASSERT(
		_gfx_create_buffer_with_length(sizeof(TextureColorVertex) * VERTEX_COUNT, sizeof(TextureColorVertex), D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, &_gfxState.pVertexBuffer) == S_OK,
		"Failed to create vertex buffer for texture-color rendering"
	);
	DBG_ASSERT(
		_gfx_create_buffer_with_length(sizeof(BaseShaderUniform), sizeof(BaseShaderUniform), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, &_gfxState.pUniformBuffer) == S_OK,
		"Failed to create uniform buffer"
	);
	_gfxState.matrixStack.index = 0;
	mat2dIdent(&_gfxState.matrixStack.matrix);
	_gfxState.pCurrentPipeline = NULL;
	_gfxState.pipelineID = (uint32_t)-1;
	gfx_set_pipeline(PIPELINE_TEXTURE);
}
void gfx_shutdown(void) {
	// TODO: clear resources
}
void gfx_begin(void) {
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = _gfxState.viewportSize.x;
	viewport.Height = _gfxState.viewportSize.y;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	_gfxState.pDeviceContext->lpVtbl->RSSetViewports(_gfxState.pDeviceContext, 1, &viewport);
	_gfxState.pDeviceContext->lpVtbl->RSSetState(_gfxState.pDeviceContext, _gfxState.pRasterizerState);
	_gfxState.pDeviceContext->lpVtbl->OMSetBlendState(_gfxState.pDeviceContext, _gfxState.pBlendState, NULL, 0xFFFFFFFF);
	_gfxState.pDeviceContext->lpVtbl->OMSetRenderTargets(_gfxState.pDeviceContext, 1, &_gfxState.pBackBufferView, NULL);
	_gfxState.pDeviceContext->lpVtbl->ClearRenderTargetView(_gfxState.pDeviceContext, _gfxState.pBackBufferView, (float32_t*)&_gfxState.clearColor);
	_gfxState.pDeviceContext->lpVtbl->PSSetSamplers(_gfxState.pDeviceContext, 0, 1, &_gfxState.pNeareastSampler);
	mat4Orthographic(&_gfxState.uniformData.projectionMatrix, 0.0f, _gfxState.viewportSize.x, _gfxState.viewportSize.y, 0.0f, -100.0f, 100.0f);
	D3D11_MAPPED_SUBRESOURCE resource = { 0 };
	HRESULT result = _gfxState.pDeviceContext->lpVtbl->Map(_gfxState.pDeviceContext, (ID3D11Resource*)_gfxState.pUniformBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	DBG_ASSERT(result == S_OK, "Failed to map Uniform Buffer");
	memcpy(resource.pData, (const void*)&_gfxState.uniformData, sizeof(_gfxState.uniformData));
	_gfxState.pDeviceContext->lpVtbl->Unmap(_gfxState.pDeviceContext, (ID3D11Resource*)_gfxState.pUniformBuffer, 0);
	_gfxState.pDeviceContext->lpVtbl->VSSetConstantBuffers(_gfxState.pDeviceContext, 0, 1, &_gfxState.pUniformBuffer);
}
void gfx_end(void) {
	gfx_flush();
	_gfx_d3d11_swap_buffers();
}
void gfx_flush(void) {
	uint32_t count = _gfxState.batchBuffer.count;
	DrawBatch* pBatches = _gfxState.batchBuffer.pBuffer;

	if (_gfxState.pipelineID == PIPELINE_TEXTURE) {
		if (count > 0 && _gfxState.vertices.count > 0) {
			size_t size = _gfxState.vertices.count * sizeof(TextureColorVertex);
			UINT stride = sizeof(TextureColorVertex);
			UINT offset = 0;

			D3D11_MAPPED_SUBRESOURCE resource = { 0 };
			HRESULT result = _gfxState.pDeviceContext->lpVtbl->Map(_gfxState.pDeviceContext, (ID3D11Resource*)_gfxState.pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
			DBG_ASSERT(result == S_OK, "Failed to map Vertex Buffer");
			size_t dataSize = _gfxState.vertices.count * sizeof(TextureColorVertex);
			memcpy(resource.pData, (const void*)_gfxState.vertices.pBuffer, dataSize);
			_gfxState.pDeviceContext->lpVtbl->Unmap(_gfxState.pDeviceContext, (ID3D11Resource*)_gfxState.pVertexBuffer, 0);

			_gfxState.pDeviceContext->lpVtbl->VSSetShader(_gfxState.pDeviceContext, _gfxState.pipelines[0].pVertexShader, NULL, 0);
			_gfxState.pDeviceContext->lpVtbl->PSSetShader(_gfxState.pDeviceContext, _gfxState.pipelines[0].pPixelShader, NULL, 0);
			_gfxState.pDeviceContext->lpVtbl->IASetInputLayout(_gfxState.pDeviceContext, _gfxState.pipelines[0].pInputLayout);
			_gfxState.pDeviceContext->lpVtbl->IASetVertexBuffers(_gfxState.pDeviceContext, 0, 1, &_gfxState.pVertexBuffer, &stride, &offset);
			_gfxState.pDeviceContext->lpVtbl->IASetPrimitiveTopology(_gfxState.pDeviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			for (uint32_t index = 0; index < count; ++index) {
				DrawBatch* pBatch = &pBatches[index];
				ID3D11ShaderResourceView* pTextureView = _gfxState.textures.pBuffer[pBatch->texture].pView;
				_gfxState.pDeviceContext->lpVtbl->PSSetShaderResources(_gfxState.pDeviceContext, 0, 1, &pTextureView);
				_gfxState.pDeviceContext->lpVtbl->Draw(_gfxState.pDeviceContext, pBatch->vertexCount, pBatch->offset);
			}
		}
	}
	else if (_gfxState.pipelineID == PIPELINE_LINE) {
		if (_gfxState.points.count > 0) {
			size_t size = _gfxState.points.count * sizeof(PointVertex);
			UINT stride = sizeof(PointVertex);
			UINT offset = 0;
			D3D11_MAPPED_SUBRESOURCE resource = { 0 };
			HRESULT result = _gfxState.pDeviceContext->lpVtbl->Map(_gfxState.pDeviceContext, (ID3D11Resource*)_gfxState.pPointVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
			DBG_ASSERT(result == S_OK, "Failed to map Point Vertex Buffer");
			size_t dataSize = _gfxState.points.count * sizeof(PointVertex);
			memcpy(resource.pData, (const void*)_gfxState.points.pBuffer, dataSize);
			_gfxState.pDeviceContext->lpVtbl->Unmap(_gfxState.pDeviceContext, (ID3D11Resource*)_gfxState.pPointVertexBuffer, 0);

			_gfxState.pDeviceContext->lpVtbl->VSSetShader(_gfxState.pDeviceContext, _gfxState.pipelines[1].pVertexShader, NULL, 0);
			_gfxState.pDeviceContext->lpVtbl->PSSetShader(_gfxState.pDeviceContext, _gfxState.pipelines[1].pPixelShader, NULL, 0);
			_gfxState.pDeviceContext->lpVtbl->IASetInputLayout(_gfxState.pDeviceContext, _gfxState.pipelines[1].pInputLayout);
			_gfxState.pDeviceContext->lpVtbl->IASetVertexBuffers(_gfxState.pDeviceContext, 0, 1, &_gfxState.pPointVertexBuffer, &stride, &offset);
			_gfxState.pDeviceContext->lpVtbl->IASetPrimitiveTopology(_gfxState.pDeviceContext, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			_gfxState.pDeviceContext->lpVtbl->Draw(_gfxState.pDeviceContext, _gfxState.points.count, 0);
		}
	}

	_gfxState.currentTexture = INVALID_TEXTURE_ID;
	_gfxState.pCurrentBatch = NULL;
	_gfxState.batchBuffer.count = 0;
	_gfxState.vertices.count = 0;
	_gfxState.points.count = 0;
}
void gfx_resize(float32_t width, float32_t height) {
	_gfxState.viewportSize.x = width;
	_gfxState.viewportSize.y = height;
}
void gfx_set_clear_color(float32_t r, float32_t g, float32_t b, float32_t a) {
	_gfxState.clearColor.r = r;
	_gfxState.clearColor.g = g;
	_gfxState.clearColor.b = b;
	_gfxState.clearColor.a = a;
}
TextureID gfx_create_texture(uint32_t width, uint32_t height, const void* pPixels) {
	D3D11_TEXTURE2D_DESC textureDesc = { 0 };
	ID3D11Texture2D* pTexture = NULL;
	ID3D11ShaderResourceView* pTextureView = NULL;
	TextureID texId = INVALID_TEXTURE_ID;
	HRESULT result;

	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.ArraySize = 1;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA resourceDesc = { 0 };
	resourceDesc.pSysMem = pPixels;
	resourceDesc.SysMemPitch = width * 4;
	resourceDesc.SysMemSlicePitch = width * height * 4;
	result = _gfxState.pDevice->lpVtbl->CreateTexture2D(_gfxState.pDevice, &textureDesc, &resourceDesc, &pTexture);
	DBG_ASSERT(result == S_OK, "Failed to create Texture2D");

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = { 0 };
	viewDesc.Format = textureDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	viewDesc.Texture2D.MostDetailedMip = 0;
	result = _gfxState.pDevice->lpVtbl->CreateShaderResourceView(_gfxState.pDevice, (ID3D11Resource*)pTexture, &viewDesc, &pTextureView);
	DBG_ASSERT(result == S_OK, "Failed to create Texture View");

	Texture2D tex2D;
	tex2D.pTexture = pTexture;
	tex2D.pView = pTextureView;
	tex2D.size.x = (float32_t)width;
	tex2D.size.y = (float32_t)height;

	_gfxState.textures.pBuffer[_gfxState.textures.count] = tex2D;
	texId = _gfxState.textures.count++;

	return texId;
}
TextureID gfx_load_texture(const char* pTexturePath) {
	int x, y, c;
	uint8_t* pPixels = stbi_load(pTexturePath, &x, &y, &c, 4);
	DBG_ASSERT(pPixels != NULL, "Failed to load image %s", pTexturePath);
	return gfx_create_texture(x, y, pPixels);
}
static __forceinline TextureColorVertex _push_vertex(float32_t x, float32_t y, float32_t u, float32_t v, uint32_t color) {
	vec2_t output = { 0.0f, 0.0f };
	vec2_t input = { x, y };
	mat2DVec2Mul(&output, &_gfxState.matrixStack.matrix, &input);
	TextureColorVertex vertex = { { output.x, output.y }, { u, v }, color };
	return vertex;
}

static __forceinline void _push_quad(float32_t x, float32_t y, float32_t w, float32_t h, float32_t u0, float32_t v0, float32_t u1, float32_t v1, uint32_t color) {
	if (_gfxState.vertices.count >= VERTEX_COUNT) return;
	TextureColorVertex vert0 = _push_vertex(x, y, u0, v0, color);
	TextureColorVertex vert1 = _push_vertex(x, y + h, u0, v1, color);
	TextureColorVertex vert2 = _push_vertex(x + w, y + h, u1, v1, color);
	TextureColorVertex vert3 = _push_vertex(x + w, y, u1, v0, color);
	TextureColorVertex* pVertices = &_gfxState.vertices.pBuffer[_gfxState.vertices.count];
	pVertices[0] = vert0;
	pVertices[1] = vert1;
	pVertices[2] = vert2;
	pVertices[3] = vert0;
	pVertices[4] = vert2;
	pVertices[5] = vert3;
	_gfxState.vertices.count += 6;
	_gfxState.pCurrentBatch->vertexCount += 6;
}

static void _create_batch(TextureID texture, uint32_t vertexCount, uint32_t offset) {
	DrawBatch batch = { .texture = texture,.vertexCount = vertexCount,.offset = offset };
	DrawBatch* pDst = &_gfxState.batchBuffer.pBuffer[_gfxState.batchBuffer.count];
	*pDst = batch;
	_gfxState.pCurrentBatch = &_gfxState.batchBuffer.pBuffer[_gfxState.batchBuffer.count++];
}

static Texture2D _check_tex_batch(TextureID texId) {
	if (texId != _gfxState.currentTexture) {
		_create_batch(texId, 0, _gfxState.vertices.count);
		_gfxState.currentTexture = texId;
	}
	return _gfxState.textures.pBuffer[texId];
}

void gfx_draw_texture(TextureID texture, float32_t x, float32_t y) {
	gfx_draw_texture_with_color(texture, x, y, 0xFFFFFFFF);
}

void gfx_draw_texture_with_color(TextureID texture, float32_t x, float32_t y, uint32_t color) {

	DBG_ASSERT(_gfxState.pipelineID == PIPELINE_TEXTURE, "Need to set pipeline to PIPELINE_TEXTURE to draw textures.");
	Texture2D textureData = _check_tex_batch(texture);
	float32_t width = textureData.size.x;
	float32_t height = textureData.size.y;
	_push_quad(x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f, color);
}
void gfx_draw_texture_frame(TextureID texture, float32_t x, float32_t y, float32_t fx, float32_t fy, float32_t fw, float32_t fh) {
	gfx_draw_texture_frame_with_color(texture, x, y, fx, fy, fw, fh, 0xFFFFFFFF);
}
void gfx_draw_texture_frame_with_color(TextureID texture, float32_t x, float32_t y, float32_t fx, float32_t fy, float32_t fw, float32_t fh, uint32_t color) {
	DBG_ASSERT(_gfxState.pipelineID == PIPELINE_TEXTURE, "Need to set pipeline to PIPELINE_TEXTURE to draw textures.");
	Texture2D textureData = _check_tex_batch(texture);
	float32_t width = textureData.size.x;
	float32_t height = textureData.size.y;
	float32_t u0 = fx / width;
	float32_t v0 = fy / height;
	float32_t u1 = (fx + fw) / width;
	float32_t v1 = (fy + fh) / height;
	_push_quad(x, y, fw, fh, u0, v0, u1, v1, color);
}
vec2_t gfx_get_texture_size(TextureID texture) {
	Texture2D textureData = _gfxState.textures.pBuffer[texture];
	return textureData.size;
}
vec2_t gfx_get_view_size(void) {
	return _gfxState.viewportSize;
}

void gfx_push_matrix(void) {
	if (_gfxState.matrixStack.index < MAX_MATRICES) {
		_gfxState.matrixStack.matrices[_gfxState.matrixStack.index++] = _gfxState.matrixStack.matrix;
	}
}
void gfx_pop_matrix(void) {
	if (_gfxState.matrixStack.index > 0) {
		_gfxState.matrixStack.matrix = _gfxState.matrixStack.matrices[--_gfxState.matrixStack.index];
	}
}
void gfx_translate(float32_t x, float32_t y) {
	mat2d_t result = _gfxState.matrixStack.matrix;
	mat2DTranslate(&result, &_gfxState.matrixStack.matrix, x, y);
	_gfxState.matrixStack.matrix = result;
}
void gfx_scale(float32_t x, float32_t y) {
	mat2d_t result = _gfxState.matrixStack.matrix;
	mat2DScale(&result, &_gfxState.matrixStack.matrix, x, y);
	_gfxState.matrixStack.matrix = result;
}
void gfx_rotate(float32_t r) {
	mat2d_t result = _gfxState.matrixStack.matrix;
	mat2DRotate(&result, &_gfxState.matrixStack.matrix, r);
	_gfxState.matrixStack.matrix = result;
}
void gfx_load_identity(void) {
	mat2dIdent(&_gfxState.matrixStack.matrix);
}
void gfx_vertex2(float32_t x, float32_t y, uint32_t color) {
	DBG_ASSERT(_gfxState.pipelineID == PIPELINE_LINE, "Need to set pipeline to PIPELINE_LINE to draw lines.");
	if (_gfxState.points.count >= MAX_POINTS) return;
	vec2_t output = { 0.0f, 0.0f };
	vec2_t input = { x, y };
	mat2DVec2Mul(&output, &_gfxState.matrixStack.matrix, &input);
	PointVertex vertex = { { output.x, output.y }, color };
	_gfxState.points.pBuffer[_gfxState.points.count++] = vertex;
}
void gfx_line2(float32_t x0, float32_t y0, float32_t x1, float32_t y1, uint32_t color0, uint32_t color1) {
	gfx_vertex2(x0, y0, color0);
	gfx_vertex2(x1, y1, color1);
}
void gfx_line(float32_t x0, float32_t y0, float32_t x1, float32_t y1, uint32_t color) {
	gfx_line2(x0, y0, x1, y1, color, color);
}
bool32_t gfx_set_pipeline(uint32_t pipeline) {
	if (pipeline >= 0 && pipeline < MAX_PIPELINES && _gfxState.pipelineID != pipeline) {
		if (_gfxState.pCurrentPipeline) {
			gfx_flush();
		}
		_gfxState.pipelineID = pipeline;
		_gfxState.pCurrentPipeline = &_gfxState.pipelines[pipeline];
		return GF_TRUE;
	}
	return GF_FALSE;
}

