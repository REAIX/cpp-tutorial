# 什么是着色器Shader
> 📖 相关章节：[图形编程概述](../../11-图形与GPU计算/00-图形编程概述.md)

> **着色器就是运行在GPU上的小程序。** 它们不是"着色"那么简单——顶点着色器管变形，片段着色器管上色，几何着色器管生成，计算着色器管一切——着色器是GPU编程的基本单元。

***

### 1. 要义概览

**着色器（Shader）** 是运行在GPU上的小型程序，用专门的着色器语言编写，在渲染管线的特定阶段对每个顶点或每个像素执行。现代图形编程的核心就是编写各种着色器来控制GPU如何处理图形数据。

***

### 2. 生活类比

| 类比 | 说明 |
|------|------|
| 着色器 = 流水线工人 | 每个工人（着色器）负责一道工序，数据从上游传到下游 |
| 顶点着色器 = 裁缝 | 把布料（顶点）裁剪成正确的形状和位置 |
| 片段着色器 = 画师 | 给每个像素涂上正确的颜色 |
| 几何着色器 = 装饰工 | 可以在已有形状上添加新的装饰 |
| 计算着色器 = 自由工人 | 不在流水线上，可以做任何计算任务 |

***

### 3. 着色器语言对比

| 特性 | GLSL | HLSL | MSL |
|------|------|------|-----|
| 全称 | OpenGL Shading Language | High-Level Shading Language | Metal Shading Language |
| 所属API | OpenGL / Vulkan | DirectX | Metal |
| 平台 | 跨平台 | Windows / Xbox | macOS / iOS |
| 语法风格 | C风格 | C++风格 | C++风格 |
| 编译方式 | 运行时编译 | 离线编译(DXC) | 离线编译 |
| 版本 | #version 460 | Shader Model 6.0 | Metal 3.0 |

#### 3.1 GLSL示例（OpenGL/Vulkan）

```glsl
// GLSL顶点着色器
#version 460 core

// 输入：从顶点缓冲区读取
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

// 输出：传递给下一个阶段
out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vFragPos;

// Uniform：从CPU端传入的全局变量
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    vFragPos = vec3(uModel * vec4(aPosition, 1.0));
    vTexCoord = aTexCoord;
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
}
```

#### 3.2 HLSL示例（DirectX）

```hlsl
// HLSL顶点着色器
struct VSInput {
    float3 position : POSITION;    // 语义绑定
    float2 texCoord : TEXCOORD;
    float3 normal   : NORMAL;
};

struct VSOutput {
    float4 position : SV_POSITION; // 系统值语义
    float2 texCoord : TEXCOORD;
    float3 normal   : NORMAL;
    float3 fragPos  : TEXCOORD1;
};

cbuffer Constants : register(b0) {   // 常量缓冲区
    float4x4 Model;
    float4x4 View;
    float4x4 Projection;
};

VSOutput main(VSInput input) {
    VSOutput output;
    output.fragPos = mul(Model, float4(input.position, 1.0)).xyz;
    output.texCoord = input.texCoord;
    output.normal = mul((float3x3)Model, input.normal);
    output.position = mul(Projection, mul(View, mul(Model, float4(input.position, 1.0))));
    return output;
}
```

#### 3.3 MSL示例（Metal）

```metal
// MSL顶点着色器
#include <metal_stdlib>
using namespace metal;

struct VertexIn {
    float3 position [[attribute(0)]];  // 属性绑定
    float2 texCoord [[attribute(1)]];
    float3 normal   [[attribute(2)]];
};

struct VertexOut {
    float4 position [[position]];      // 内建属性
    float2 texCoord;
    float3 normal;
    float3 fragPos;
};

vertex VertexOut vertex_main(
    VertexIn in [[stage_in]],
    constant float4x4& model [[buffer(1)]],
    constant float4x4& view [[buffer(2)]],
    constant float4x4& projection [[buffer(3)]]
) {
    VertexOut out;
    out.fragPos = (model * float4(in.position, 1.0)).xyz;
    out.texCoord = in.texCoord;
    out.normal = (model * float4(in.normal, 0.0)).xyz;
    out.position = projection * view * model * float4(in.position, 1.0);
    return out;
}
```

***

### 4. 着色器类型详解

#### 4.1 顶点着色器（Vertex Shader）

**执行频率**：每个顶点一次

```glsl
// 顶点着色器：MVP变换 + 骨骼动画
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 3) in ivec4 aBoneIDs;   // 骨骼ID
layout(location = 4) in vec4 aWeights;     // 骨骼权重

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 boneTransforms[100];           // 最多100根骨骼

void main() {
    // 骨骼蒙皮：加权混合多根骨骼的变换
    mat4 skinMatrix =
        aWeights.x * boneTransforms[aBoneIDs.x] +
        aWeights.y * boneTransforms[aBoneIDs.y] +
        aWeights.z * boneTransforms[aBoneIDs.z] +
        aWeights.w * boneTransforms[aBoneIDs.w];

    vec4 skinnedPos = skinMatrix * vec4(aPos, 1.0);
    gl_Position = projection * view * model * skinnedPos;
}
```

#### 4.2 片段着色器（Fragment Shader / Pixel Shader）

**执行频率**：每个片段（潜在像素）一次

```glsl
// 片段着色器：PBR（基于物理的渲染）
#version 460 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vWorldPos;

out vec4 FragColor;

uniform sampler2D albedoMap;      // 基础颜色贴图
uniform sampler2D normalMap;      // 法线贴图
uniform sampler2D metallicMap;    // 金属度贴图
uniform sampler2D roughnessMap;   // 粗糙度贴图
uniform sampler2D aoMap;          // 环境遮蔽贴图

uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
uniform vec3 camPos;

// 菲涅尔方程（Fresnel）
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 albedo    = texture(albedoMap, vTexCoord).rgb;
    float metallic = texture(metallicMap, vTexCoord).r;
    float roughness= texture(roughnessMap, vTexCoord).r;
    float ao       = texture(aoMap, vTexCoord).r;

    vec3 N = normalize(vNormal);
    vec3 V = normalize(camPos - vWorldPos);

    vec3 F0 = vec3(0.04);  // 非金属的基础反射率
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < 4; i++) {
        vec3 L = normalize(lightPositions[i] - vWorldPos);
        vec3 H = normalize(V + L);

        float distance = length(lightPositions[i] - vWorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = /* 法线分布函数 */;
        float G   = /* 几何遮蔽函数 */;
        vec3 F     = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / 3.14159 + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;
    FragColor = vec4(color, 1.0);
}
```

#### 4.3 几何着色器（Geometry Shader）

**执行频率**：每个图元一次，可生成新图元

```glsl
// 几何着色器：法线可视化工具
#version 460 core
layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

in vec3 vNormal[];

uniform mat4 projection;
uniform mat4 view;
uniform float normalLength;

void GenerateLine(int index) {
    gl_Position = projection * view * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = projection * view * (gl_in[index].gl_Position +
                  vec4(vNormal[index], 0.0) * normalLength);
    EmitVertex();
    EndPrimitive();
}

void main() {
    GenerateLine(0);  // 顶点0的法线
    GenerateLine(1);  // 顶点1的法线
    GenerateLine(2);  // 顶点2的法线
}
```

#### 4.4 计算着色器（Compute Shader）

**执行频率**：每个工作组调用一次，脱离图形管线

```glsl
// 计算着色器：高斯模糊
#version 460 core
layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0) uniform sampler2D srcImage;
layout(binding = 0, rgba32f) uniform writeonly image2D dstImage;

uniform int direction;  // 0=水平, 1=垂直

const float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
    vec2 texSize = vec2(textureSize(srcImage, 0));
    vec2 texCoord = (vec2(coord) + 0.5) / texSize;

    vec3 result = texture(srcImage, texCoord).rgb * weights[0];

    if (direction == 0) {
        // 水平模糊
        for (int i = 1; i < 5; i++) {
            vec2 offset = vec2(float(i), 0.0) / texSize;
            result += texture(srcImage, texCoord + offset).rgb * weights[i];
            result += texture(srcImage, texCoord - offset).rgb * weights[i];
        }
    } else {
        // 垂直模糊
        for (int i = 1; i < 5; i++) {
            vec2 offset = vec2(0.0, float(i)) / texSize;
            result += texture(srcImage, texCoord + offset).rgb * weights[i];
            result += texture(srcImage, texCoord - offset).rgb * weights[i];
        }
    }

    imageStore(dstImage, coord, vec4(result, 1.0));
}
```

***

### 5. 着色器编译流程

```
源代码(.glsl/.hlsl/.metal)
        │
        ↓
┌──────────────────┐
│   词法/语法分析    │  解析源代码为AST
└──────────────────┘
        │
        ↓
┌──────────────────┐
│   语义分析        │  类型检查、变量解析
└──────────────────┘
        │
        ↓
┌──────────────────┐
│   中间表示(IR)    │  生成与硬件无关的中间代码
└──────────────────┘
        │
        ↓
┌──────────────────┐
│   优化            │  常量折叠、死代码消除、循环优化
└──────────────────┘
        │
        ↓
┌──────────────────┐
│   目标代码生成    │  生成特定GPU架构的机器码
└──────────────────┘
        │
        ↓
GPU二进制指令
```

#### 5.1 不同API的编译方式

```cpp
// OpenGL：运行时编译
GLuint shader = glCreateShader(GL_VERTEX_SHADER);
glShaderSource(shader, 1, &source, NULL);
glCompileShader(shader);

// 检查编译错误
GLint success;
glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    // 处理编译错误
}

// Vulkan：离线编译（推荐）
// 使用glslangValidator或shaderc离线编译为SPIR-V
// 命令行：glslangValidator -V shader.vert -o shader.vert.spv
// 然后在运行时加载SPIR-V二进制

// DirectX：离线编译
// 使用fxc或dxc编译器
// 命令行：dxc -T vs_6_0 -E main shader.hlsl -Fo shader.cso

// Metal：离线编译
// 使用xcrun metal编译
// 命令行：xcrun -sdk macosx metal -c shader.metal -o shader.air
```

#### 5.2 SPIR-V：跨平台中间格式

```
GLSL/HLSL源码
      │
      ↓ (glslangValidator / shaderc)
SPIR-V二进制
      │
      ├──→ Vulkan驱动 (直接使用)
      ├──→ OpenGL驱动 (通过glSpecializeShader)
      └──→ Metal转换 (通过SPIRV-Cross)
           └──→ MSL源码 → Metal编译器
```

***

### 6. 着色器中的数据传递

#### 6.1 数据传递方式

| 方式 | 方向 | 作用域 | 速度 |
|------|------|--------|------|
| Vertex Attribute | CPU→顶点着色器 | 每个顶点 | 快 |
| Uniform | CPU→任意着色器 | 全局（一次绘制） | 快 |
| Varying/In-Out | 着色器间传递 | 管线阶段间 | 快 |
| Texture/Sampler | CPU→任意着色器 | 全局 | 中等 |
| Storage Buffer | CPU↔着色器 | 全局 | 中等 |
| Shared Memory | 计算着色器内 | 工作组内 | 极快 |

```glsl
// Uniform：全局常量，一次绘制中不变
uniform mat4 uModelMatrix;
uniform vec3 uLightPos;
uniform float uTime;

// Storage Buffer：可读写的大块数据
layout(std430, binding = 0) buffer DataBuffer {
    float data[];
};

// Shared Memory：计算着色器中工作组内共享
shared float sharedData[256];
```

#### 6.2 顶点着色器到片段着色器的数据传递

```glsl
// 顶点着色器
out vec3 vColor;       // 输出
out vec2 vTexCoord;

void main() {
    vColor = aColor;   // 设置输出值
    vTexCoord = aTexCoord;
    // 光栅化阶段会自动对vColor和vTexCoord进行插值
}

// 片段着色器
in vec3 vColor;        // 接收插值后的值
in vec2 vTexCoord;

void main() {
    FragColor = texture(diffuseMap, vTexCoord) * vec4(vColor, 1.0);
}
```

***

### 7. 着色器性能优化

#### 7.1 常见优化策略

```glsl
// 优化1：减少纹理采样次数
// 差：多次采样同一纹理
vec4 c1 = texture(tex, uv);
vec4 c2 = texture(tex, uv + offset);
// 好：如果可能，合并为一次采样

// 优化2：避免分支分歧
// 差：warp内分支
if (useTexture) {
    color = texture(tex, uv);
} else {
    color = uniformColor;
}
// 好：用step/mix替代分支
color = mix(uniformColor, texture(tex, uv), step(0.5, float(useTexture)));

// 优化3：利用向量运算
// 差：标量运算
float r = a.x * b.x;
float g = a.y * b.y;
float b = a.z * b.z;
// 好：向量运算
vec3 result = a * b;

// 优化4：减少依赖纹理读取
// 差：前一次采样的结果决定下一次采样的坐标
vec4 c1 = texture(tex1, uv);
vec4 c2 = texture(tex2, c1.rg);  // 依赖c1的结果
// 好：尽量使用独立的纹理坐标
```

#### 7.2 着色器复杂度指标

| 指标 | 含义 | 优化目标 |
|------|------|---------|
| 指令数 | 着色器执行的ALU指令数 | 减少指令数 |
| 纹理采样次数 | 每个片段的纹理读取次数 | 减少采样次数 |
| 寄存器压力 | 需要的临时寄存器数量 | 减少寄存器使用 |
| 占用率 | SM上活跃warp的比例 | 提高占用率 |

***

### 8. 常见误区

| 误区 | 事实 |
|------|------|
| 着色器只能做图形渲染 | 计算着色器可以做任何并行计算 |
| GLSL/HLSL完全不同 | 核心概念相同，语法差异不大 |
| 着色器编译很慢 | 离线编译+缓存可解决 |
| 着色器不能调试 | RenderDoc/NSight等工具支持着色器调试 |
| 片段着色器比顶点着色器重要 | 两者缺一不可，瓶颈可能在任一个 |

***

### 9. 总结

| 着色器类型 | 执行频率 | 核心任务 | 必需性 |
|-----------|---------|---------|--------|
| 顶点着色器 | 每顶点 | 坐标变换 | 必需 |
| 曲面细分着色器 | 每面片 | 细分曲面 | 可选 |
| 几何着色器 | 每图元 | 生成/修改图元 | 可选 |
| 片段着色器 | 每片段 | 计算颜色 | 必需 |
| 计算着色器 | 每工作组 | 通用计算 | 独立使用 |

**核心记忆**：着色器 = GPU上的小程序。顶点着色器管位置，片段着色器管颜色，计算着色器管一切。用GLSL/HLSL/MSL编写，编译后运行在GPU上。