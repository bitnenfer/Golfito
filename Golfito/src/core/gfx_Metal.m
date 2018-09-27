#import <MetalKit/MetalKit.h>
#import <Foundation/Foundation.h>
#include "math.h"
#include "gfx.h"
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "assert.h"
#import <GLKit/GLKMath.h>

#define MAX_MATRICES 100
#define MAX_PIPELINES 2
#define MAX_FRAMES 3
static const uint32_t MAX_QUADS = 16000;
static const uint32_t BATCH_COUNT = 1000;
static const uint32_t VERTEX_COUNT = MAX_QUADS * 6;
static const uint32_t MAX_POINTS = 10000;

typedef struct {
    mat2d_t matrices[MAX_MATRICES];
    mat2d_t matrix;
    uint32_t index;
} MatrixStack;

typedef struct {
    GLKVector2 position;
    GLKVector2 texCoord;
    uint32_t color;
} TextureColorVertex;

typedef struct {
    GLKVector2 position;
    uint32_t color;
} PointVertex;

typedef struct {
//    mat4_t projectionMatrix;
    GLKVector2 resolution;
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
    TextureColorVertex* pBuffer[MAX_FRAMES];
    uint32_t count;
    uint32_t idx;
} TextureColorVertexBuffer;

typedef struct {
    PointVertex* pBuffer[MAX_FRAMES];
    uint32_t count;
    uint32_t idx;
} PointBuffer;

typedef struct {
    MatrixStack matrixStack;
    BaseShaderUniform uniformData;
    vec2_t viewportSize;
    DrawBatchBuffer batchBuffer;
    TextureColorVertexBuffer vertices;
    PointBuffer points;
    id<MTLDevice> device;
    id<MTLCommandQueue> cmdQueue;
    id<MTLRenderPipelineState> pipelines[MAX_PIPELINES];
    id<MTLRenderPipelineState> currentPipeline;
    id<MTLBuffer> vertexBuffer[MAX_FRAMES];
    id<MTLBuffer> pointVertexBuffer[MAX_FRAMES];
    id<MTLLibrary> defaultLibrary;
    id<MTLCommandBuffer> cmdBuffer;
    id<MTLRenderCommandEncoder> renderCmdEncoder;
    MTLLoadAction framebufferLoadAction;
    dispatch_semaphore_t frameSemaphore;
    MTKView* metalKitView;
    DrawBatch* pCurrentBatch;
    TextureID currentTexture;
    MTLClearColor clearColor;
    MTLViewport viewport;
    uint32_t pipelineID;
    float32_t pixelScale;
    uint32_t frameIdx;
} GfxStateMetal;

static GfxStateMetal _gfxState = { 0 };
NSBundle* _assetBundle = NULL;


void gfx_initialize (void) {
    _assetBundle = [NSBundle mainBundle];
    for (uint32_t index = 0; index < MAX_FRAMES; ++index) {
        _gfxState.points.pBuffer[index] = (PointVertex*)malloc(sizeof(PointVertex) * MAX_POINTS);
        _gfxState.vertices.pBuffer[index] = (TextureColorVertex*)malloc((sizeof(TextureColorVertex) * VERTEX_COUNT));

    }
    _gfxState.vertices.count = 0;
    _gfxState.vertices.idx = 0;
    _gfxState.points.count = 0;
    _gfxState.points.idx = 0;
    _gfxState.batchBuffer.pBuffer = (DrawBatch*)malloc(sizeof(DrawBatch) * BATCH_COUNT);
    _gfxState.batchBuffer.count = 0;
    _gfxState.currentTexture = ((void*)0xDEADBEEF);
    _gfxState.pCurrentBatch = NULL;
    for (uint32_t index = 0; index < MAX_FRAMES; ++index) {
        _gfxState.pointVertexBuffer[index] = [_gfxState.device newBufferWithLength:sizeof(PointVertex)*MAX_POINTS options:MTLResourceStorageModeShared];
        _gfxState.vertexBuffer[index] = [_gfxState.device newBufferWithLength:sizeof(TextureColorVertex)*VERTEX_COUNT options:MTLResourceStorageModeShared];
    }
    _gfxState.frameIdx = 0;
    _gfxState.matrixStack.index = 0;
    mat2dIdent(&_gfxState.matrixStack.matrix);
    _gfxState.currentPipeline = NULL;
    _gfxState.pipelineID = (uint32_t)-1;
    gfx_set_pipeline(PIPELINE_TEXTURE);
    _gfxState.framebufferLoadAction = MTLLoadActionClear;
}
void gfx_shutdown(void) {

}
void gfx_begin (void) {
    _gfxState.frameIdx = (_gfxState.frameIdx + 1) % MAX_FRAMES;

    dispatch_semaphore_wait(_gfxState.frameSemaphore, DISPATCH_TIME_FOREVER);
    MTLRenderPassDescriptor* pCurrentRenderPassDesc = _gfxState.metalKitView.currentRenderPassDescriptor;
    pCurrentRenderPassDesc.colorAttachments[0].clearColor = _gfxState.clearColor;
    pCurrentRenderPassDesc.colorAttachments[0].loadAction = _gfxState.framebufferLoadAction;
    id<MTLCommandBuffer> cmdBuffer = [_gfxState.cmdQueue commandBuffer];
    id<MTLRenderCommandEncoder> renderEncoder = [cmdBuffer renderCommandEncoderWithDescriptor:pCurrentRenderPassDesc];
    _gfxState.cmdBuffer = cmdBuffer;
    _gfxState.renderCmdEncoder = renderEncoder;
#if defined(TARGET_IOS) || defined(TARGET_TVOS)
    UIScreen* mainScreen = [UIScreen mainScreen];
    CGSize viewSize = [mainScreen nativeBounds].size;
    _gfxState.viewportSize.x = viewSize.width;
    _gfxState.viewportSize.y = viewSize.height;
    _gfxState.pixelScale = mainScreen.nativeScale;
#else
    NSScreen* mainScreen = [NSScreen mainScreen];
    _gfxState.viewportSize.x = mainScreen.frame.size.width;
    _gfxState.viewportSize.y = mainScreen.frame.size.height;
    _gfxState.pixelScale = mainScreen.backingScaleFactor;
#endif
//    mat4Orthographic(&_gfxState.uniformData.projectionMatrix, 0.0f, _gfxState.viewportSize.x, _gfxState.viewportSize.y, 0.0f, -100.0f, 100.0f);
    _gfxState.uniformData.resolution.x = _gfxState.viewportSize.x;
    _gfxState.uniformData.resolution.y = _gfxState.viewportSize.y;
    MTLViewport viewport = { 0.0, 0.0, _gfxState.viewportSize.x * _gfxState.pixelScale, _gfxState.viewportSize.y * _gfxState.pixelScale, -10.0, 10.0 };
    _gfxState.viewport = viewport;
}
void _gfx_force_end_no_present (void) {
    gfx_flush();
    [_gfxState.renderCmdEncoder endEncoding];
    __weak dispatch_semaphore_t semaphore = _gfxState.frameSemaphore;
    [_gfxState.cmdBuffer addCompletedHandler:^(id<MTLCommandBuffer> commandBuffer) {
        // GPU work is complete
        // Signal the semaphore to start the CPU work
        dispatch_semaphore_signal(semaphore);
    }];
    
    [_gfxState.cmdBuffer commit];
    _gfxState.cmdBuffer = NULL;
    _gfxState.renderCmdEncoder = NULL;
    _gfxState.frameIdx = (_gfxState.frameIdx + 1) % MAX_FRAMES;
}
void gfx_end (void) {
    gfx_flush();
    [_gfxState.renderCmdEncoder endEncoding];
    [_gfxState.cmdBuffer presentDrawable:_gfxState.metalKitView.currentDrawable];
    __weak dispatch_semaphore_t semaphore = _gfxState.frameSemaphore;
    [_gfxState.cmdBuffer addCompletedHandler:^(id<MTLCommandBuffer> commandBuffer) {
        // GPU work is complete
        // Signal the semaphore to start the CPU work
        dispatch_semaphore_signal(semaphore);
    }];
    
    [_gfxState.cmdBuffer commit];
    _gfxState.cmdBuffer = NULL;
    _gfxState.renderCmdEncoder = NULL;
}

static void _gfx_flush_no_clear (void) {
    uint32_t count = _gfxState.batchBuffer.count;
    DrawBatch* pBatches = _gfxState.batchBuffer.pBuffer;
    id<MTLRenderCommandEncoder> renderEncoder = _gfxState.renderCmdEncoder;
    
    [renderEncoder setViewport:_gfxState.viewport];
    [renderEncoder setCullMode:MTLCullModeNone];
    
    if (_gfxState.pipelineID == PIPELINE_TEXTURE) {
        if (count > 0 && _gfxState.vertices.count > 0) {
            size_t size = _gfxState.vertices.count * sizeof(TextureColorVertex);
            memcpy(_gfxState.vertexBuffer[_gfxState.frameIdx].contents, (void*)_gfxState.vertices.pBuffer[_gfxState.vertices.idx], size);
            [renderEncoder setRenderPipelineState:_gfxState.pipelines[PIPELINE_TEXTURE]];
            [renderEncoder setVertexBuffer:_gfxState.vertexBuffer[_gfxState.frameIdx] offset:0 atIndex:0];
            [renderEncoder setVertexBytes:&_gfxState.uniformData length:sizeof(BaseShaderUniform) atIndex:1];
            for (uint32_t index = 0; index < count; ++index) {
                DrawBatch* pBatch = &pBatches[index];
                id<MTLTexture> mtlTexture = ((__bridge id<MTLTexture>)pBatch->texture);
                [renderEncoder setFragmentTexture:mtlTexture atIndex:0];
                [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:pBatch->offset vertexCount:pBatch->vertexCount];
            }
            _gfxState.vertices.idx = (_gfxState.vertices.idx + 1) % MAX_FRAMES;
        }
    } else if (_gfxState.pipelineID == PIPELINE_LINE) {
        if (_gfxState.points.count > 0) {
            size_t size = _gfxState.points.count * sizeof(PointVertex);
            memcpy(_gfxState.pointVertexBuffer[_gfxState.frameIdx].contents, (void*)_gfxState.points.pBuffer[_gfxState.points.idx], size);
            [renderEncoder setRenderPipelineState:_gfxState.pipelines[PIPELINE_LINE]];
            [renderEncoder setVertexBuffer:_gfxState.pointVertexBuffer[_gfxState.frameIdx] offset:0 atIndex:0];
            [renderEncoder setVertexBytes:&_gfxState.uniformData length:sizeof(BaseShaderUniform) atIndex:1];
            [renderEncoder drawPrimitives:MTLPrimitiveTypeLine vertexStart:0 vertexCount:_gfxState.points.count];
            _gfxState.points.idx = (_gfxState.points.idx + 1) % MAX_FRAMES;
        }
    }
}
void gfx_flush (void) {
    _gfx_flush_no_clear();
    _gfxState.currentTexture = (void*)0xDEADBEEF;
    _gfxState.pCurrentBatch = NULL;
    _gfxState.batchBuffer.count = 0;
    _gfxState.vertices.count = 0;
    _gfxState.points.count = 0;
}

void gfx_resize (float32_t width, float32_t height) {
    _gfxState.viewportSize.x = width;
    _gfxState.viewportSize.y = height;
}

void _gfx_init_state (MTKView* pView, float32_t width, float32_t height) {
    _gfxState.metalKitView = pView;
    _gfxState.device = pView.device;
    _gfxState.cmdQueue = [_gfxState.device newCommandQueue];
    _gfxState.defaultLibrary = [_gfxState.device newDefaultLibrary];
    _gfxState.viewportSize.x = width;
    _gfxState.viewportSize.y = height;

    // Texture Color Pipeline
    {
        id<MTLFunction> textureColorVS = [_gfxState.defaultLibrary newFunctionWithName:@"textureColorVS"];
        id<MTLFunction> textureColorFS = [_gfxState.defaultLibrary newFunctionWithName:@"textureColorFS"];
        
        MTLVertexDescriptor* pVertexDescriptor = [[MTLVertexDescriptor alloc] init];
        pVertexDescriptor.attributes[0].offset = offsetof(TextureColorVertex, position);
        pVertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
        pVertexDescriptor.attributes[0].bufferIndex = 0;
        pVertexDescriptor.attributes[1].offset = offsetof(TextureColorVertex, texCoord);
        pVertexDescriptor.attributes[1].format = MTLVertexFormatFloat2;
        pVertexDescriptor.attributes[1].bufferIndex = 0;
        pVertexDescriptor.attributes[2].offset = offsetof(TextureColorVertex, color);
        pVertexDescriptor.attributes[2].format = MTLVertexFormatUChar4;
        pVertexDescriptor.attributes[2].bufferIndex = 0;
        pVertexDescriptor.layouts[0].stepRate = 1;
        pVertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        pVertexDescriptor.layouts[0].stride = sizeof(TextureColorVertex);
        
        MTLRenderPipelineDescriptor* pRenderPipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
        pRenderPipelineDesc.label = @"Texture Color Pipeline";
        pRenderPipelineDesc.vertexFunction = textureColorVS;
        pRenderPipelineDesc.fragmentFunction = textureColorFS;
        pRenderPipelineDesc.vertexDescriptor = pVertexDescriptor;
        pRenderPipelineDesc.colorAttachments[0].pixelFormat = pView.colorPixelFormat;
        pRenderPipelineDesc.colorAttachments[0].blendingEnabled = YES;
        pRenderPipelineDesc.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
        pRenderPipelineDesc.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
        pRenderPipelineDesc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
        pRenderPipelineDesc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
        pRenderPipelineDesc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        pRenderPipelineDesc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        
        NSError* pError = NULL;
        _gfxState.pipelines[0] = [_gfxState.device newRenderPipelineStateWithDescriptor:pRenderPipelineDesc error:&pError];
        if (pError) {
            NSLog(@"Failed to create Texture Color Pipeline:\n%@", pError);
            exit(1);
            return;
        }
    }
    
    // Line Color Pipeline
    {
        id<MTLFunction> lineColorVS = [_gfxState.defaultLibrary newFunctionWithName:@"lineColorVS"];
        id<MTLFunction> lineColorFS = [_gfxState.defaultLibrary newFunctionWithName:@"lineColorFS"];
        
        MTLVertexDescriptor* pVertexDescriptor = [[MTLVertexDescriptor alloc] init];
        pVertexDescriptor.attributes[0].offset = offsetof(PointVertex, position);
        pVertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
        pVertexDescriptor.attributes[0].bufferIndex = 0;
        pVertexDescriptor.attributes[1].offset = offsetof(PointVertex, color);
        pVertexDescriptor.attributes[1].format = MTLVertexFormatUChar4;
        pVertexDescriptor.attributes[1].bufferIndex = 0;
        pVertexDescriptor.layouts[0].stepRate = 1;
        pVertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        pVertexDescriptor.layouts[0].stride = sizeof(PointVertex);
        
        MTLRenderPipelineDescriptor* pRenderPipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
        pRenderPipelineDesc.label = @"Line Color Pipeline";
        pRenderPipelineDesc.vertexFunction = lineColorVS;
        pRenderPipelineDesc.fragmentFunction = lineColorFS;
        pRenderPipelineDesc.vertexDescriptor = pVertexDescriptor;
        pRenderPipelineDesc.colorAttachments[0].pixelFormat = pView.colorPixelFormat;
        pRenderPipelineDesc.colorAttachments[0].blendingEnabled = YES;
        pRenderPipelineDesc.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
        pRenderPipelineDesc.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
        pRenderPipelineDesc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
        pRenderPipelineDesc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
        pRenderPipelineDesc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        pRenderPipelineDesc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        
        NSError* pError = NULL;
        _gfxState.pipelines[1] = [_gfxState.device newRenderPipelineStateWithDescriptor:pRenderPipelineDesc error:&pError];
        if (pError) {
            NSLog(@"Failed to create Line Color Pipeline:\n%@", pError);
            exit(1);
            return;
        }

    }
    
    MTLClearColor clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    pView.clearColor = clearColor;
    
    _gfxState.frameSemaphore = dispatch_semaphore_create(MAX_FRAMES);
    
    // if we want to add depth buffer do it via the MTKView's setDepthStencilPixelFormat and setClearDepth
//    mat4Orthographic(&_gfxState.uniformData.projectionMatrix, 0.0f, width, height, 0.0f, -100.0f, 100.0f);
    _gfxState.uniformData.resolution.x = width;
    _gfxState.uniformData.resolution.y = height;
}

void gfx_set_clear_color(float32_t r, float32_t g, float32_t b, float32_t a) {
    _gfxState.clearColor.red = r;
    _gfxState.clearColor.green = g;
    _gfxState.clearColor.blue = b;
    _gfxState.clearColor.alpha = a;
}

TextureID gfx_create_texture(uint32_t width, uint32_t height, const void* pPixels) {
    MTLPixelFormat format;
#if defined(TARGET_IOS) || defined(TARGET_TVOS)
    format = MTLPixelFormatBGRA8Unorm;
#else
    format = MTLPixelFormatRGBA8Unorm;
#endif
    MTLTextureDescriptor* pTextureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format width:width height:height mipmapped:NO];
    id<MTLTexture> mtlTexture = [_gfxState.device newTextureWithDescriptor:pTextureDesc];
    [mtlTexture replaceRegion:MTLRegionMake2D(0, 0, width, height) mipmapLevel:0 withBytes:pPixels bytesPerRow:4 * width];
    void* pOpaque = ((__bridge_retained void*)mtlTexture);
    return pOpaque;
}

TextureID gfx_load_texture(const char* pTexturePath) {
    int x,y,c;
    NSString* str = [[NSString alloc] initWithCString:pTexturePath encoding:NSASCIIStringEncoding];
    NSString* path = [_assetBundle pathForResource:str ofType:NULL];
    if (!path) {
        NSLog(@"Failed to load image %@", str);
        return INVALID_TEXTURE_ID;
    }
    uint8_t* pPixels = stbi_load([path UTF8String], &x, &y, &c, 4);
    if (pPixels==NULL) {
        NSLog(@"Failed to load image %@", path);
        return INVALID_TEXTURE_ID;
    }
    return gfx_create_texture(x, y, pPixels);
}

vec2_t gfx_get_texture_size(TextureID texture) {
    id<MTLTexture> mtlTexture = ((__bridge id<MTLTexture>)texture);
    vec2_t size = { (float32_t)mtlTexture.width, (float32_t)mtlTexture.height };
    return size;
}

static __attribute__((always_inline)) inline TextureColorVertex _push_vertex (float32_t x, float32_t y, float32_t u ,float32_t v, uint32_t color) {
    vec2_t output = { 0.0f, 0.0f };
    vec2_t input = { x, y };
    mat2DVec2Mul(&output, &_gfxState.matrixStack.matrix, &input);
    TextureColorVertex vertex = { { output.x, output.y }, { u, v }, color };
    return vertex;
}

static __attribute__((always_inline)) inline void _push_quad(float32_t x, float32_t y, float32_t w, float32_t h, float32_t u0, float32_t v0, float32_t u1, float32_t v1, uint32_t color) {
    if (_gfxState.vertices.count >= VERTEX_COUNT) return;
    TextureColorVertex vert0 = _push_vertex(x, y, u0, v0, color);
    TextureColorVertex vert1 = _push_vertex(x, y + h, u0, v1, color);
    TextureColorVertex vert2 = _push_vertex(x + w, y + h, u1, v1, color);
    TextureColorVertex vert3 = _push_vertex(x + w, y, u1, v0, color);
    TextureColorVertex* pVertices = &_gfxState.vertices.pBuffer[_gfxState.vertices.idx][_gfxState.vertices.count];
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
    DrawBatch batch = { .texture = texture, .vertexCount = vertexCount, .offset = offset };
    DrawBatch* pDst = &_gfxState.batchBuffer.pBuffer[_gfxState.batchBuffer.count];
    *pDst = batch;
    _gfxState.pCurrentBatch = &_gfxState.batchBuffer.pBuffer[_gfxState.batchBuffer.count++];
}

static id<MTLTexture> _check_tex_batch(TextureID texId) {
    if (texId != _gfxState.currentTexture) {
        _create_batch(texId, 0, _gfxState.vertices.count);
        _gfxState.currentTexture = texId;
    }
    id<MTLTexture> mtlTexture = ((__bridge id<MTLTexture>)texId);
    return mtlTexture;
}

void gfx_draw_texture(TextureID texture, float32_t x, float32_t y) {
    gfx_draw_texture_with_color(texture, x, y, 0xFFFFFFFF);
}

void gfx_draw_texture_with_color(TextureID texture, float32_t x, float32_t y, uint32_t color) {
    DBG_ASSERT(_gfxState.pipelineID == PIPELINE_TEXTURE, "Need to set pipeline to PIPELINE_TEXTURE to draw textures.");
    id<MTLTexture> mtlTexture = _check_tex_batch(texture);
    float32_t width = (float32_t)mtlTexture.width;
    float32_t height = (float32_t)mtlTexture.height;
    _push_quad(x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f, color);
}
void gfx_draw_texture_frame(TextureID texture, float32_t x, float32_t y, float32_t fx, float32_t fy, float32_t fw, float32_t fh) {
    gfx_draw_texture_frame_with_color(texture, x, y, fx, fy, fw, fh, 0xFFFFFFFF);
}
void gfx_draw_texture_frame_with_color(TextureID texture, float32_t x, float32_t y, float32_t fx, float32_t fy, float32_t fw, float32_t fh, uint32_t color) {
    DBG_ASSERT(_gfxState.pipelineID == PIPELINE_TEXTURE, "Need to set pipeline to PIPELINE_TEXTURE to draw textures.");
    id<MTLTexture> mtlTexture = _check_tex_batch(texture);
    float32_t width = (float32_t)mtlTexture.width;
    float32_t height = (float32_t)mtlTexture.height;
    float32_t u0 = fx / width;
    float32_t v0 = fy / height;
    float32_t u1 = (fx + fw) / width;
    float32_t v1 = (fy + fh) / height;
    _push_quad(x, y, fw, fh, u0, v0, u1, v1, color);
}

vec2_t gfx_get_view_size (void) {
    return _gfxState.viewportSize;
}

void gfx_push_matrix (void) {
    if (_gfxState.matrixStack.index < MAX_MATRICES) {
        _gfxState.matrixStack.matrices[_gfxState.matrixStack.index++] = _gfxState.matrixStack.matrix;
    }
}
void gfx_pop_matrix (void) {
    if (_gfxState.matrixStack.index > 0) {
        _gfxState.matrixStack.matrix = _gfxState.matrixStack.matrices[--_gfxState.matrixStack.index];
    }
}
void gfx_translate (float32_t x, float32_t y) {
    mat2d_t result = _gfxState.matrixStack.matrix;
    mat2DTranslate(&result, &_gfxState.matrixStack.matrix, x, y);
    _gfxState.matrixStack.matrix = result;
}
void gfx_scale (float32_t x, float32_t y) {
    mat2d_t result = _gfxState.matrixStack.matrix;
    mat2DScale(&result, &_gfxState.matrixStack.matrix, x, y);
    _gfxState.matrixStack.matrix = result;
}
void gfx_rotate (float32_t r) {
    mat2d_t result = _gfxState.matrixStack.matrix;
    mat2DRotate(&result, &_gfxState.matrixStack.matrix, r);
    _gfxState.matrixStack.matrix = result;
}
void gfx_load_identity (void) {
    mat2dIdent(&_gfxState.matrixStack.matrix);
}
void gfx_vertex2(float32_t x, float32_t y, uint32_t color) {
    DBG_ASSERT(_gfxState.pipelineID == PIPELINE_LINE, "Need to set pipeline to PIPELINE_LINE to draw lines.");
    if (_gfxState.points.count >= MAX_POINTS) return;
    vec2_t output = { 0.0f, 0.0f };
    vec2_t input = { x, y };
    mat2DVec2Mul(&output, &_gfxState.matrixStack.matrix, &input);
    PointVertex vertex = { { output.x, output.y }, color };
    _gfxState.points.pBuffer[_gfxState.points.idx][_gfxState.points.count++] = vertex;
}
void gfx_line2(float32_t x0, float32_t y0, float32_t x1, float32_t y1, uint32_t color0, uint32_t color1) {
    gfx_vertex2(x0, y0, color0);
    gfx_vertex2(x1, y1, color1);
}
void gfx_line(float32_t x0, float32_t y0, float32_t x1, float32_t y1, uint32_t color) {
    gfx_line2(x0, y0, x1, y1, color, color);
}
bool32_t gfx_set_pipeline (uint32_t pipeline) {
    if (pipeline >= 0 && pipeline < MAX_PIPELINES && _gfxState.pipelineID != pipeline) {
        if (_gfxState.currentPipeline) {
            _gfxState.framebufferLoadAction = MTLLoadActionDontCare;
            gfx_flush();
            _gfx_force_end_no_present();
            gfx_begin();
            _gfxState.framebufferLoadAction = MTLLoadActionClear;
        }
        _gfxState.pipelineID = pipeline;
        _gfxState.currentPipeline = _gfxState.pipelines[pipeline];
        return GF_TRUE;
    }
    return GF_FALSE;
}
float32_t gfx_get_pixel_ratio (void) {
#if defined(TARGET_MACOS)
    return (float32_t)[NSScreen mainScreen].backingScaleFactor;
#elif defined(TARGET_IOS) || defined(TARGET_TVOS)
    return (float32_t)[UIScreen mainScreen].nativeScale;
#endif
}
