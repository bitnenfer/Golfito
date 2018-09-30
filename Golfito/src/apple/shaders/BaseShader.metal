
#include <metal_math>
#include <metal_stdlib>
using namespace metal;

struct VertexUniform {
    float2 resolution;
};

struct TextureVertexIn {
    float2 position[[attribute(0)]];
    float2 texCoord[[attribute(1)]];
    uchar4 color[[attribute(2)]];
};

struct TextureVertexOut {
    float4 position[[position]];
    float2 texCoord;
    float4 color;
};

vertex TextureVertexOut textureColorVS (
                               uint vertexID[[vertex_id]],
                               device TextureVertexIn* vertices[[buffer(0)]],
                               constant VertexUniform& uniform[[buffer(1)]]) {
    
    TextureVertexOut out;
    TextureVertexIn vert = vertices[vertexID];
    out.position = float4(((vert.position / uniform.resolution) * 2.0 - 1.0) * float2(1.0, -1.0), 0.0, 1.0);
    out.texCoord = vert.texCoord;
    out.color = float4(vert.color.abgr) / float4(255.0);
    
    return out;
}

fragment float4 textureColorFS (
                                TextureVertexOut in[[stage_in]],
                                texture2d<float> colorTexture[[texture(0)]]) {
    
    constexpr sampler nearestSampler(mag_filter::nearest, min_filter::nearest);
    const float4 color = colorTexture.sample(nearestSampler, in.texCoord) * in.color;
    
    return color;
}

// Line Pipeline

struct LineVertexIn {
    float2 position[[attribute(0)]];
    uchar4 color[[attribute(0)]];
};

struct LineVertexOut {
    float4 position[[position]];
    float4 color;
};

vertex LineVertexOut lineColorVS (
                                        uint vertexID[[vertex_id]],
                                        device LineVertexIn* vertices[[buffer(0)]],
                                        constant VertexUniform& uniform[[buffer(1)]]) {
    
    LineVertexOut out;
    LineVertexIn vert = vertices[vertexID];
    out.position = float4(((vert.position / uniform.resolution) * 2.0 - 1.0) * float2(1.0, -1.0), 0.0, 1.0);
    out.color = float4(vert.color.abgr) / float4(255.0);
    
    return out;
}

fragment float4 lineColorFS (LineVertexOut in[[stage_in]]) {
    return in.color;
}
