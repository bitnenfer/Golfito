#import <MetalKit/MetalKit.h>
#import <Foundation/Foundation.h>
#include "math.h"
#include "gfx.h"
#include "utils.h"
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "assert.h"
#include "memory.h"
#import <GLKit/GLKMath.h>


static const uint32_t kMaxMatrices = 100;
static const uint32_t kMaxPipelines = 2;
static const uint32_t kMaxFrames = 3;
static const uint32_t kMaxQuads = 16000;
static const uint32_t kMaxBatches = 1000;
static const uint32_t kMaxVertices = kMaxQuads * 6;
static const uint32_t kMaxPoints = 10000;

typedef struct {
    mat2d_t matrices[kMaxMatrices];
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
    TextureColorVertex* pBuffer[kMaxFrames];
    uint32_t count;
    uint32_t idx;
} TextureColorVertexBuffer;

typedef struct {
    PointVertex* pBuffer[kMaxFrames];
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
    id<MTLRenderPipelineState> pipelines[kMaxPipelines];
    id<MTLRenderPipelineState> currentPipeline;
    id<MTLBuffer> vertexBuffer[kMaxFrames];
    id<MTLBuffer> pointVertexBuffer[kMaxFrames];
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

static GfxStateMetal gGfxState = { 0 };
NSBundle* gAssetBundle = NULL;

void gfx_initialize (void) {
    gAssetBundle = [NSBundle mainBundle];
    for (uint32_t index = 0; index < kMaxFrames; ++index) {
        gGfxState.points.pBuffer[index] = (PointVertex*)mem_flist_alloc(sizeof(PointVertex) * kMaxPoints, 4);
        gGfxState.vertices.pBuffer[index] = (TextureColorVertex*)mem_flist_alloc((sizeof(TextureColorVertex) * kMaxVertices), 4);
    }
    gGfxState.vertices.count = 0;
    gGfxState.vertices.idx = 0;
    gGfxState.points.count = 0;
    gGfxState.points.idx = 0;
    gGfxState.batchBuffer.pBuffer = (DrawBatch*)mem_flist_alloc(sizeof(DrawBatch) * kMaxBatches, 4);
    gGfxState.batchBuffer.count = 0;
    gGfxState.currentTexture = ((void*)0xDEADBEEF);
    gGfxState.pCurrentBatch = NULL;
    for (uint32_t index = 0; index < kMaxFrames; ++index) {
        gGfxState.pointVertexBuffer[index] = [gGfxState.device newBufferWithLength:sizeof(PointVertex)*kMaxPoints options:MTLResourceStorageModeShared];
        gGfxState.vertexBuffer[index] = [gGfxState.device newBufferWithLength:sizeof(TextureColorVertex)*kMaxVertices options:MTLResourceStorageModeShared];
    }
    gGfxState.frameIdx = 0;
    gGfxState.matrixStack.index = 0;
    mat2dIdent(&gGfxState.matrixStack.matrix);
    gGfxState.currentPipeline = NULL;
    gGfxState.pipelineID = (uint32_t)-1;
    gfx_set_pipeline(PIPELINE_TEXTURE);
    gGfxState.framebufferLoadAction = MTLLoadActionClear;
}
void gfx_shutdown(void) {

}
void gfx_begin (void) {
    gGfxState.frameIdx = (gGfxState.frameIdx + 1) % kMaxFrames;

    dispatch_semaphore_wait(gGfxState.frameSemaphore, DISPATCH_TIME_FOREVER);
    MTLRenderPassDescriptor* pCurrentRenderPassDesc = gGfxState.metalKitView.currentRenderPassDescriptor;
    pCurrentRenderPassDesc.colorAttachments[0].clearColor = gGfxState.clearColor;
    pCurrentRenderPassDesc.colorAttachments[0].loadAction = gGfxState.framebufferLoadAction;
    id<MTLCommandBuffer> cmdBuffer = [gGfxState.cmdQueue commandBuffer];
    id<MTLRenderCommandEncoder> renderEncoder = [cmdBuffer renderCommandEncoderWithDescriptor:pCurrentRenderPassDesc];
    gGfxState.cmdBuffer = cmdBuffer;
    gGfxState.renderCmdEncoder = renderEncoder;
#if defined(TARGET_IOS) || defined(TARGET_TVOS)
    UIScreen* mainScreen = [UIScreen mainScreen];
    CGSize viewSize = [mainScreen nativeBounds].size;
    gGfxState.viewportSize.x = viewSize.width;
    gGfxState.viewportSize.y = viewSize.height;
    gGfxState.pixelScale = mainScreen.nativeScale;
#else
    NSScreen* mainScreen = [NSScreen mainScreen];
    gGfxState.viewportSize.x = mainScreen.frame.size.width;
    gGfxState.viewportSize.y = mainScreen.frame.size.height;
    gGfxState.pixelScale = mainScreen.backingScaleFactor;
#endif
//    mat4Orthographic(&gGfxState.uniformData.projectionMatrix, 0.0f, gGfxState.viewportSize.x, gGfxState.viewportSize.y, 0.0f, -100.0f, 100.0f);
    gGfxState.uniformData.resolution.x = gGfxState.viewportSize.x;
    gGfxState.uniformData.resolution.y = gGfxState.viewportSize.y;
    MTLViewport viewport = { 0.0, 0.0, gGfxState.viewportSize.x * gGfxState.pixelScale, gGfxState.viewportSize.y * gGfxState.pixelScale, -10.0, 10.0 };
    gGfxState.viewport = viewport;
}
void _gfx_force_end_no_present (void) {
    gfx_flush();
    [gGfxState.renderCmdEncoder endEncoding];
    __weak dispatch_semaphore_t semaphore = gGfxState.frameSemaphore;
    [gGfxState.cmdBuffer addCompletedHandler:^(id<MTLCommandBuffer> commandBuffer) {
        // GPU work is complete
        // Signal the semaphore to start the CPU work
        dispatch_semaphore_signal(semaphore);
    }];
    
    [gGfxState.cmdBuffer commit];
    gGfxState.cmdBuffer = NULL;
    gGfxState.renderCmdEncoder = NULL;
    gGfxState.frameIdx = (gGfxState.frameIdx + 1) % kMaxFrames;
}
void gfx_end (void) {
    gfx_flush();
    [gGfxState.renderCmdEncoder endEncoding];
    [gGfxState.cmdBuffer presentDrawable:gGfxState.metalKitView.currentDrawable];
    __weak dispatch_semaphore_t semaphore = gGfxState.frameSemaphore;
    [gGfxState.cmdBuffer addCompletedHandler:^(id<MTLCommandBuffer> commandBuffer) {
        // GPU work is complete
        // Signal the semaphore to start the CPU work
        dispatch_semaphore_signal(semaphore);
    }];
    
    [gGfxState.cmdBuffer commit];
    gGfxState.cmdBuffer = NULL;
    gGfxState.renderCmdEncoder = NULL;
}

static void _gfx_flush_no_clear (void) {
    uint32_t count = gGfxState.batchBuffer.count;
    DrawBatch* pBatches = gGfxState.batchBuffer.pBuffer;
    id<MTLRenderCommandEncoder> renderEncoder = gGfxState.renderCmdEncoder;
    
    [renderEncoder setViewport:gGfxState.viewport];
    [renderEncoder setCullMode:MTLCullModeNone];
    
    if (gGfxState.pipelineID == PIPELINE_TEXTURE) {
        if (count > 0 && gGfxState.vertices.count > 0) {
            size_t size = gGfxState.vertices.count * sizeof(TextureColorVertex);
            memcpy(gGfxState.vertexBuffer[gGfxState.frameIdx].contents, (void*)gGfxState.vertices.pBuffer[gGfxState.vertices.idx], size);
            [renderEncoder setRenderPipelineState:gGfxState.pipelines[PIPELINE_TEXTURE]];
            [renderEncoder setVertexBuffer:gGfxState.vertexBuffer[gGfxState.frameIdx] offset:0 atIndex:0];
            [renderEncoder setVertexBytes:&gGfxState.uniformData length:sizeof(BaseShaderUniform) atIndex:1];
            for (uint32_t index = 0; index < count; ++index) {
                DrawBatch* pBatch = &pBatches[index];
                id<MTLTexture> mtlTexture = ((__bridge id<MTLTexture>)pBatch->texture);
                [renderEncoder setFragmentTexture:mtlTexture atIndex:0];
                [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:pBatch->offset vertexCount:pBatch->vertexCount];
            }
            gGfxState.vertices.idx = (gGfxState.vertices.idx + 1) % kMaxFrames;
        }
    } else if (gGfxState.pipelineID == PIPELINE_LINE) {
        if (gGfxState.points.count > 0) {
            size_t size = gGfxState.points.count * sizeof(PointVertex);
            memcpy(gGfxState.pointVertexBuffer[gGfxState.frameIdx].contents, (void*)gGfxState.points.pBuffer[gGfxState.points.idx], size);
            [renderEncoder setRenderPipelineState:gGfxState.pipelines[PIPELINE_LINE]];
            [renderEncoder setVertexBuffer:gGfxState.pointVertexBuffer[gGfxState.frameIdx] offset:0 atIndex:0];
            [renderEncoder setVertexBytes:&gGfxState.uniformData length:sizeof(BaseShaderUniform) atIndex:1];
            [renderEncoder drawPrimitives:MTLPrimitiveTypeLine vertexStart:0 vertexCount:gGfxState.points.count];
            gGfxState.points.idx = (gGfxState.points.idx + 1) % kMaxFrames;
        }
    }
}
void gfx_flush (void) {
    _gfx_flush_no_clear();
    gGfxState.currentTexture = (void*)0xDEADBEEF;
    gGfxState.pCurrentBatch = NULL;
    gGfxState.batchBuffer.count = 0;
    gGfxState.vertices.count = 0;
    gGfxState.points.count = 0;
}

void gfx_resize (float32_t width, float32_t height) {
    gGfxState.viewportSize.x = width;
    gGfxState.viewportSize.y = height;
}

void _gfx_init_state (MTKView* pView, float32_t width, float32_t height) {
    gGfxState.metalKitView = pView;
    gGfxState.device = pView.device;
    gGfxState.cmdQueue = [gGfxState.device newCommandQueue];
    gGfxState.defaultLibrary = [gGfxState.device newDefaultLibrary];
    gGfxState.viewportSize.x = width;
    gGfxState.viewportSize.y = height;

    // Texture Color Pipeline
    {
        id<MTLFunction> textureColorVS = [gGfxState.defaultLibrary newFunctionWithName:@"textureColorVS"];
        id<MTLFunction> textureColorFS = [gGfxState.defaultLibrary newFunctionWithName:@"textureColorFS"];
        
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
        gGfxState.pipelines[0] = [gGfxState.device newRenderPipelineStateWithDescriptor:pRenderPipelineDesc error:&pError];
        if (pError) {
            NSLog(@"Failed to create Texture Color Pipeline:\n%@", pError);
            exit(1);
            return;
        }
    }
    
    // Line Color Pipeline
    {
        id<MTLFunction> lineColorVS = [gGfxState.defaultLibrary newFunctionWithName:@"lineColorVS"];
        id<MTLFunction> lineColorFS = [gGfxState.defaultLibrary newFunctionWithName:@"lineColorFS"];
        
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
        gGfxState.pipelines[1] = [gGfxState.device newRenderPipelineStateWithDescriptor:pRenderPipelineDesc error:&pError];
        if (pError) {
            NSLog(@"Failed to create Line Color Pipeline:\n%@", pError);
            exit(1);
            return;
        }

    }
    
    MTLClearColor clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    pView.clearColor = clearColor;
    
    gGfxState.frameSemaphore = dispatch_semaphore_create(kMaxFrames);
    
    // if we want to add depth buffer do it via the MTKView's setDepthStencilPixelFormat and setClearDepth
//    mat4Orthographic(&gGfxState.uniformData.projectionMatrix, 0.0f, width, height, 0.0f, -100.0f, 100.0f);
    gGfxState.uniformData.resolution.x = width;
    gGfxState.uniformData.resolution.y = height;
}

void gfx_set_clear_color(float32_t r, float32_t g, float32_t b, float32_t a) {
    gGfxState.clearColor.red = r;
    gGfxState.clearColor.green = g;
    gGfxState.clearColor.blue = b;
    gGfxState.clearColor.alpha = a;
}

TextureID gfx_create_texture(uint32_t width, uint32_t height, const void* pPixels) {
    MTLPixelFormat format;
#if defined(TARGET_IOS) || defined(TARGET_TVOS)
    format = MTLPixelFormatBGRA8Unorm;
#else
    format = MTLPixelFormatRGBA8Unorm;
#endif
    MTLTextureDescriptor* pTextureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format width:width height:height mipmapped:NO];
    id<MTLTexture> mtlTexture = [gGfxState.device newTextureWithDescriptor:pTextureDesc];
    [mtlTexture replaceRegion:MTLRegionMake2D(0, 0, width, height) mipmapLevel:0 withBytes:pPixels bytesPerRow:4 * width];
    void* pOpaque = ((__bridge_retained void*)mtlTexture);
    return pOpaque;
}

TextureID gfx_load_texture(const char* pTexturePath) {
    int x,y,c;
    NSString* str = [[NSString alloc] initWithCString:pTexturePath encoding:NSASCIIStringEncoding];
    NSString* path = [gAssetBundle pathForResource:str ofType:NULL];
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
    mat2DVec2Mul(&output, &gGfxState.matrixStack.matrix, &input);
    TextureColorVertex vertex = { { output.x, output.y }, { u, v }, color };
    return vertex;
}

static __attribute__((always_inline)) inline void _push_quad(float32_t x, float32_t y, float32_t w, float32_t h, float32_t u0, float32_t v0, float32_t u1, float32_t v1, uint32_t color) {
    if (gGfxState.vertices.count >= kMaxVertices) return;
    TextureColorVertex vert0 = _push_vertex(x, y, u0, v0, color);
    TextureColorVertex vert1 = _push_vertex(x, y + h, u0, v1, color);
    TextureColorVertex vert2 = _push_vertex(x + w, y + h, u1, v1, color);
    TextureColorVertex vert3 = _push_vertex(x + w, y, u1, v0, color);
    TextureColorVertex* pVertices = &gGfxState.vertices.pBuffer[gGfxState.vertices.idx][gGfxState.vertices.count];
    pVertices[0] = vert0;
    pVertices[1] = vert1;
    pVertices[2] = vert2;
    pVertices[3] = vert0;
    pVertices[4] = vert2;
    pVertices[5] = vert3;
    gGfxState.vertices.count += 6;
    gGfxState.pCurrentBatch->vertexCount += 6;
}

static void _create_batch(TextureID texture, uint32_t vertexCount, uint32_t offset) {
    DrawBatch batch = { .texture = texture, .vertexCount = vertexCount, .offset = offset };
    DrawBatch* pDst = &gGfxState.batchBuffer.pBuffer[gGfxState.batchBuffer.count];
    *pDst = batch;
    gGfxState.pCurrentBatch = &gGfxState.batchBuffer.pBuffer[gGfxState.batchBuffer.count++];
}

static id<MTLTexture> _check_tex_batch(TextureID texId) {
    if (texId != gGfxState.currentTexture) {
        _create_batch(texId, 0, gGfxState.vertices.count);
        gGfxState.currentTexture = texId;
    }
    id<MTLTexture> mtlTexture = ((__bridge id<MTLTexture>)texId);
    return mtlTexture;
}

void gfx_draw_texture(TextureID texture, float32_t x, float32_t y) {
    gfx_draw_texture_with_color(texture, x, y, 0xFFFFFFFF);
}

void gfx_draw_texture_with_color(TextureID texture, float32_t x, float32_t y, uint32_t color) {
    DBG_ASSERT(gGfxState.pipelineID == PIPELINE_TEXTURE, "Need to set pipeline to PIPELINE_TEXTURE to draw textures.");
    id<MTLTexture> mtlTexture = _check_tex_batch(texture);
    float32_t width = (float32_t)mtlTexture.width;
    float32_t height = (float32_t)mtlTexture.height;
    _push_quad(x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f, color);
}
void gfx_draw_texture_frame(TextureID texture, float32_t x, float32_t y, float32_t fx, float32_t fy, float32_t fw, float32_t fh) {
    gfx_draw_texture_frame_with_color(texture, x, y, fx, fy, fw, fh, 0xFFFFFFFF);
}
void gfx_draw_texture_frame_with_color(TextureID texture, float32_t x, float32_t y, float32_t fx, float32_t fy, float32_t fw, float32_t fh, uint32_t color) {
    DBG_ASSERT(gGfxState.pipelineID == PIPELINE_TEXTURE, "Need to set pipeline to PIPELINE_TEXTURE to draw textures.");
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
    return gGfxState.viewportSize;
}

void gfx_push_matrix (void) {
    if (gGfxState.matrixStack.index < kMaxMatrices) {
        gGfxState.matrixStack.matrices[gGfxState.matrixStack.index++] = gGfxState.matrixStack.matrix;
    }
}
void gfx_pop_matrix (void) {
    if (gGfxState.matrixStack.index > 0) {
        gGfxState.matrixStack.matrix = gGfxState.matrixStack.matrices[--gGfxState.matrixStack.index];
    }
}
void gfx_translate (float32_t x, float32_t y) {
    mat2d_t result = gGfxState.matrixStack.matrix;
    mat2DTranslate(&result, &gGfxState.matrixStack.matrix, x, y);
    gGfxState.matrixStack.matrix = result;
}
void gfx_scale (float32_t x, float32_t y) {
    mat2d_t result = gGfxState.matrixStack.matrix;
    mat2DScale(&result, &gGfxState.matrixStack.matrix, x, y);
    gGfxState.matrixStack.matrix = result;
}
void gfx_rotate (float32_t r) {
    mat2d_t result = gGfxState.matrixStack.matrix;
    mat2DRotate(&result, &gGfxState.matrixStack.matrix, r);
    gGfxState.matrixStack.matrix = result;
}
void gfx_load_identity (void) {
    mat2dIdent(&gGfxState.matrixStack.matrix);
}
void gfx_vertex2(float32_t x, float32_t y, uint32_t color) {
    DBG_ASSERT(gGfxState.pipelineID == PIPELINE_LINE, "Need to set pipeline to PIPELINE_LINE to draw lines.");
    if (gGfxState.points.count >= kMaxPoints) return;
    vec2_t output = { 0.0f, 0.0f };
    vec2_t input = { x, y };
    mat2DVec2Mul(&output, &gGfxState.matrixStack.matrix, &input);
    PointVertex vertex = { { output.x, output.y }, color };
    gGfxState.points.pBuffer[gGfxState.points.idx][gGfxState.points.count++] = vertex;
}
void gfx_line2(float32_t x0, float32_t y0, float32_t x1, float32_t y1, uint32_t color0, uint32_t color1) {
    gfx_vertex2(x0, y0, color0);
    gfx_vertex2(x1, y1, color1);
}
void gfx_line(float32_t x0, float32_t y0, float32_t x1, float32_t y1, uint32_t color) {
    gfx_line2(x0, y0, x1, y1, color, color);
}
bool32_t gfx_set_pipeline (uint32_t pipeline) {
    if (pipeline >= 0 && pipeline < kMaxPipelines && gGfxState.pipelineID != pipeline) {
        if (gGfxState.currentPipeline) {
            gGfxState.framebufferLoadAction = MTLLoadActionDontCare;
            gfx_flush();
            _gfx_force_end_no_present();
            gfx_begin();
            gGfxState.framebufferLoadAction = MTLLoadActionClear;
        }
        gGfxState.pipelineID = pipeline;
        gGfxState.currentPipeline = gGfxState.pipelines[pipeline];
        return UT_TRUE;
    }
    return UT_FALSE;
}
float32_t gfx_get_pixel_ratio (void) {
#if defined(TARGET_MACOS)
    return (float32_t)[NSScreen mainScreen].backingScaleFactor;
#elif defined(TARGET_IOS) || defined(TARGET_TVOS)
    return (float32_t)[UIScreen mainScreen].nativeScale;
#endif
}
