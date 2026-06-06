# OpenGL与着色器编程

> 掌握OpenGL核心模式与GLSL着色器语言，实现纹理映射与光照模型

***

> **OpenGL is not a library; it's a specification.** — Unknown
> （OpenGL不是库，而是一份规范。）

> **Shaders are the heart of modern real-time graphics.** — Unknown
> （着色器是现代实时图形的核心。）

***

> **🎯 纸上得来终觉浅，绝知此事要躬行。**
>
> （图形编程需要大量实践，从着色器编写开始动手是最佳路径。）

> 💡 **通俗理解 - 什么是OpenGL？**

想象一下：

- **OpenGL** 就像一个"画画的工具箱"，里面有画笔、颜料、画布等各种工具
- **着色器** 就像"画画的技法"，告诉你怎么调色、怎么画光影、怎么画纹理
- **固定管线** 就像"数字涂色书"，只能按编号涂色，不能自由发挥
- **核心模式** 就像"空白画布"，你可以自由创作任何效果

现代OpenGL（核心模式）要求你自己写"着色器"来控制每个像素的颜色，虽然学习曲线更陡，但灵活性和性能远超旧式固定管线。

> 🔬 **抽象理解 - OpenGL核心模式**：
>
> - **核心模式（Core Profile）**：移除了所有固定功能，完全由着色器控制渲染
> - **VAO/VBO/EBO**：顶点数据的组织方式，将数据从CPU传到GPU
> - **着色器程序**：在GPU上运行的小程序，控制顶点变换和像素着色
> - **纹理与采样**：将2D图像数据映射到3D表面，增加细节

***

## 前置知识
- [图形编程概述](00-图形编程概述.md)
- C++基础
- 线性代数基础

## 后续内容
- [Vulkan基础](02-Vulkan基础.md)

***

## 目录

- [1. OpenGL核心模式](#1-opengl核心模式)
- [2. GLSL着色器语言](#2-glsl着色器语言)
- [3. 顶点与片段着色器](#3-顶点与片段着色器)
- [4. 纹理映射](#4-纹理映射)
- [5. 光照模型](#5-光照模型)
- [6. OpenGL与现代C++结合](#6-opengl与现代c结合)
- [7. 本章小结](#7-本章小结)

***

## 1. OpenGL核心模式

### 1.1 OpenGL上下文与窗口创建

```cpp
// 使用GLFW创建OpenGL窗口和上下文
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

class GLWindow {
public:
    bool init(int width, int height, const std::string& title) {
        // 初始化GLFW
        if (!glfwInit()) {
            std::cerr << "GLFW初始化失败" << std::endl;
            return false;
        }

        // 设置OpenGL版本和核心模式
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

        // 创建窗口
        window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!window_) {
            std::cerr << "窗口创建失败" << std::endl;
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window_);

        // 初始化GLEW
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cerr << "GLEW初始化失败" << std::endl;
            return false;
        }

        // 设置视口
        glViewport(0, 0, width, height);

        // 注册窗口大小变化回调
        glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);

        // 输出OpenGL信息
        std::cout << "OpenGL版本: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "GLSL版本: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "渲染器: " << glGetString(GL_RENDERER) << std::endl;

        return true;
    }

    bool shouldClose() const {
        return glfwWindowShouldClose(window_);
    }

    void swapBuffers() {
        glfwSwapBuffers(window_);
    }

    void pollEvents() {
        glfwPollEvents();
    }

    void cleanup() {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }

    GLFWwindow* getHandle() const { return window_; }

private:
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }

    GLFWwindow* window_ = nullptr;
};
```

### 1.2 VAO、VBO与EBO

```cpp
// 顶点数据组织：VAO（顶点数组对象）、VBO（顶点缓冲对象）、EBO（索引缓冲对象）

struct Vertex {
    float position[3];  // 位置
    float normal[3];    // 法线
    float texCoord[2];  // 纹理坐标
};

class Mesh {
public:
    void init(const std::vector<Vertex>& vertices,
              const std::vector<uint32_t>& indices) {
        vertexCount_ = vertices.size();
        indexCount_ = indices.size();

        // 创建VAO
        glGenVertexArrays(1, &vao_);
        glBindVertexArray(vao_);

        // 创建VBO
        glGenBuffers(1, &vbo_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER,
                     vertices.size() * sizeof(Vertex),
                     vertices.data(), GL_STATIC_DRAW);

        // 创建EBO
        glGenBuffers(1, &ebo_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     indices.size() * sizeof(uint32_t),
                     indices.data(), GL_STATIC_DRAW);

        // 设置顶点属性
        // 位置属性 (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);

        // 法线属性 (location = 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);

        // 纹理坐标属性 (location = 2)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)offsetof(Vertex, texCoord));
        glEnableVertexAttribArray(2);

        // 解绑VAO（保存状态）
        glBindVertexArray(0);
    }

    void draw() const {
        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void cleanup() {
        glDeleteVertexArrays(1, &vao_);
        glDeleteBuffers(1, &vbo_);
        glDeleteBuffers(1, &ebo_);
    }

private:
    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
    size_t vertexCount_ = 0, indexCount_ = 0;
};

// 创建一个立方体的网格数据
std::pair<std::vector<Vertex>, std::vector<uint32_t>> createCube() {
    std::vector<Vertex> vertices = {
        // 前面
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        // 后面
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
        // 其他面...
    };

    std::vector<uint32_t> indices = {
        0, 1, 2, 2, 3, 0,     // 前
        4, 5, 6, 6, 7, 4,     // 后
        // 其他面索引...
    };

    return {vertices, indices};
}
```

## 2. GLSL着色器语言

### 2.1 GLSL基础语法

```glsl
// GLSL着色器语言基础

// ===== 数据类型 =====
// 标量类型
float f = 1.0;          // 浮点数
int i = 42;             // 整数
uint u = 1u;            // 无符号整数
bool b = true;          // 布尔值

// 向量类型
vec2 v2 = vec2(1.0, 2.0);           // 二维向量
vec3 v3 = vec3(1.0, 2.0, 3.0);     // 三维向量
vec4 v4 = vec4(1.0, 2.0, 3.0, 4.0); // 四维向量
ivec3 iv3 = ivec3(1, 2, 3);        // 整数向量

// 向量分量访问（swizzle）
vec4 color = vec4(1.0, 0.5, 0.0, 1.0);
vec3 rgb = color.rgb;      // 取rgb分量
vec2 rg = color.rg;        // 取rg分量
float r = color.r;         // 取r分量

// 矩阵类型
mat2 m2 = mat2(1.0);                    // 2x2单位矩阵
mat3 m3 = mat3(1.0, 0.0, 0.0,          // 3x3矩阵（列优先）
               0.0, 1.0, 0.0,
               0.0, 0.0, 1.0);
mat4 m4 = mat4(1.0);                    // 4x4单位矩阵

// 采样器类型
sampler2D tex2D;           // 2D纹理采样器
samplerCube texCube;       // 立方体贴图采样器
sampler2DArray texArray;   // 2D纹理数组采样器

// ===== 限定符 =====
// 存储限定符
// const      - 编译期常量
// in         - 输入变量（顶点属性/片段输入）
// out        - 输出变量
// uniform    - 全局统一变量（CPU传入）
// buffer     - 着色器存储缓冲区

// 布局限定符
layout(location = 0) in vec3 aPosition;     // 指定位置
layout(binding = 0) uniform sampler2D tex;  // 指定绑定点
layout(std140) uniform Matrices {           // 指定内存布局
    mat4 view;
    mat4 projection;
};
```

### 2.2 GLSL内置函数

```glsl
// GLSL常用内置函数

// 数学函数
float a = abs(-1.5);           // 绝对值 → 1.5
float b = sign(-2.0);          // 符号 → -1.0
float c = floor(1.7);          // 向下取整 → 1.0
float d = ceil(1.3);           // 向上取整 → 2.0
float e = fract(1.7);          // 小数部分 → 0.7
float f = mod(5.0, 3.0);       // 取模 → 2.0
float g = min(1.0, 2.0);       // 最小值 → 1.0
float h = max(1.0, 2.0);       // 最大值 → 2.0
float i = clamp(1.5, 0.0, 1.0); // 钳制 → 1.0
float j = mix(0.0, 1.0, 0.3);  // 线性混合 → 0.3
float k = step(0.5, 0.7);      // 阶梯函数 → 1.0
float l = smoothstep(0.0, 1.0, 0.5); // 平滑阶梯 → 0.5

// 三角函数
float angle = radians(45.0);   // 角度转弧度
float deg = degrees(0.785);    // 弧度转角度
float s = sin(angle);          // 正弦
float co = cos(angle);         // 余弦

// 指数函数
float pw = pow(2.0, 3.0);     // 幂 → 8.0
float sq = sqrt(4.0);         // 平方根 → 2.0
float ex = exp(1.0);          // e^x
float lg = log(2.718);        // 自然对数

// 向量函数
vec3 v1 = vec3(1.0, 0.0, 0.0);
vec3 v2 = vec3(0.0, 1.0, 0.0);
float len = length(v1);              // 向量长度 → 1.0
float dist = distance(v1, v2);       // 两点距离 → 1.414
float dotp = dot(v1, v2);           // 点积 → 0.0
vec3 crossp = cross(v1, v2);        // 叉积 → (0,0,1)
vec3 n = normalize(vec3(3.0, 4.0, 0.0)); // 归一化 → (0.6, 0.8, 0)
vec3 ref = reflect(v1, n);           // 反射
vec3 refr = refract(v1, n, 1.5);    // 折射
```

## 3. 顶点与片段着色器

### 3.1 着色器编译与链接

```cpp
// 着色器编译和程序链接的C++封装
#include <GL/glew.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
    GLuint programId = 0;

    // 从源码字符串创建着色器程序
    bool createFromSource(const std::string& vertexSrc,
                          const std::string& fragmentSrc) {
        // 编译顶点着色器
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSrc);
        if (!vertexShader) return false;

        // 编译片段着色器
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);
        if (!fragmentShader) {
            glDeleteShader(vertexShader);
            return false;
        }

        // 链接着色器程序
        programId = glCreateProgram();
        glAttachShader(programId, vertexShader);
        glAttachShader(programId, fragmentShader);
        glLinkProgram(programId);

        // 检查链接错误
        GLint success;
        glGetProgramiv(programId, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(programId, 512, nullptr, infoLog);
            std::cerr << "着色器程序链接失败: " << infoLog << std::endl;
            glDeleteProgram(programId);
            programId = 0;
        }

        // 着色器对象已附加到程序，可以删除
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return programId != 0;
    }

    // 从文件创建着色器程序
    bool createFromFile(const std::string& vertexPath,
                        const std::string& fragmentPath) {
        std::string vertexSrc = readFile(vertexPath);
        std::string fragmentSrc = readFile(fragmentPath);
        return createFromSource(vertexSrc, fragmentSrc);
    }

    void use() const {
        glUseProgram(programId);
    }

    // 设置uniform变量
    void setBool(const std::string& name, bool value) const {
        glUniform1i(glGetUniformLocation(programId, name.c_str()), (int)value);
    }

    void setInt(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(programId, name.c_str()), value);
    }

    void setFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(programId, name.c_str()), value);
    }

    void setVec3(const std::string& name, const float* value) const {
        glUniform3fv(glGetUniformLocation(programId, name.c_str()), 1, value);
    }

    void setMat4(const std::string& name, const float* value) const {
        glUniformMatrix4fv(glGetUniformLocation(programId, name.c_str()),
                           1, GL_FALSE, value);
    }

    void cleanup() {
        if (programId) {
            glDeleteProgram(programId);
            programId = 0;
        }
    }

private:
    GLuint compileShader(GLenum type, const std::string& source) {
        GLuint shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        // 检查编译错误
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "着色器编译失败: " << infoLog << std::endl;
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    static std::string readFile(const std::string& path) {
        std::ifstream file(path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
};
```

### 3.2 完整的顶点与片段着色器

```glsl
// ===== 顶点着色器 =====
#version 450 core

layout(location = 0) in vec3 aPosition;   // 顶点位置
layout(location = 1) in vec3 aNormal;     // 顶点法线
layout(location = 2) in vec2 aTexCoord;   // 纹理坐标
layout(location = 3) in vec3 aTangent;    // 切线（用于法线贴图）

// Uniform缓冲区 - 变换矩阵
layout(std140, binding = 0) uniform TransformBlock {
    mat4 uModel;           // 模型矩阵
    mat4 uView;            // 视图矩阵
    mat4 uProjection;      // 投影矩阵
    mat4 uNormalMatrix;    // 法线矩阵（模型矩阵逆转置）
};

// 输出到片段着色器
out VS_OUT {
    vec3 worldPos;         // 世界空间位置
    vec3 normal;           // 世界空间法线
    vec2 texCoord;         // 纹理坐标
    vec3 tangent;          // 切线
    vec3 bitangent;        // 副切线
} vs_out;

void main() {
    // 计算世界空间位置
    vec4 worldPosition = uModel * vec4(aPosition, 1.0);
    vs_out.worldPos = worldPosition.xyz;

    // 变换法线到世界空间
    vs_out.normal = normalize(mat3(uNormalMatrix) * aNormal);

    // 传递纹理坐标
    vs_out.texCoord = aTexCoord;

    // 计算TBN矩阵所需的切线和副切线
    vs_out.tangent = normalize(mat3(uModel) * aTangent);
    vs_out.bitangent = cross(vs_out.normal, vs_out.tangent);

    // 计算裁剪空间位置
    gl_Position = uProjection * uView * worldPosition;
}
```

```glsl
// ===== 片段着色器 =====
#version 450 core

// 从顶点着色器接收
in VS_OUT {
    vec3 worldPos;
    vec3 normal;
    vec2 texCoord;
    vec3 tangent;
    vec3 bitangent;
} fs_in;

// 材质参数
layout(std140, binding = 1) uniform MaterialBlock {
    vec3 uAlbedo;            // 基础颜色
    float uMetallic;         // 金属度
    float uRoughness;        // 粗糙度
    float uAO;               // 环境遮蔽
    float uOpacity;          // 不透明度
};

// 纹理采样器
uniform sampler2D uAlbedoMap;     // 漫反射贴图
uniform sampler2D uNormalMap;     // 法线贴图
uniform sampler2D uMetallicMap;   // 金属度贴图
uniform sampler2D uRoughnessMap;  // 粗糙度贴图
uniform sampler2D uAOMap;         // 环境遮蔽贴图

// 光源数据
struct Light {
    vec3 position;
    vec3 color;
    float intensity;
    int type;  // 0=方向光, 1=点光源, 2=聚光灯
};

layout(std430, binding = 2) buffer LightBuffer {
    Light lights[];
};

uniform vec3 uCameraPos;
uniform int uUseTextures;  // 是否使用纹理

out vec4 FragColor;

// 从法线贴图计算切线空间法线
vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(uNormalMap, fs_in.texCoord).xyz * 2.0 - 1.0;

    vec3 N = normalize(fs_in.normal);
    vec3 T = normalize(fs_in.tangent);
    vec3 B = normalize(fs_in.bitangent);

    // TBN矩阵：切线空间 → 世界空间
    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangentNormal);
}

// Blinn-Phong光照计算
vec3 calculateBlinnPhong(vec3 N, vec3 V, const Light light) {
    vec3 L;
    float attenuation = 1.0;

    if (light.type == 0) {
        // 方向光
        L = normalize(-light.position);
    } else if (light.type == 1) {
        // 点光源
        L = normalize(light.position - fs_in.worldPos);
        float dist = length(light.position - fs_in.worldPos);
        attenuation = 1.0 / (1.0 + 0.09 * dist + 0.032 * dist * dist);
    }

    vec3 H = normalize(L + V);  // 半程向量

    // 获取材质参数
    vec3 albedo = uUseTextures > 0 ? texture(uAlbedoMap, fs_in.texCoord).rgb : uAlbedo;
    float metallic = uUseTextures > 0 ? texture(uMetallicMap, fs_in.texCoord).r : uMetallic;
    float roughness = uUseTextures > 0 ? texture(uRoughnessMap, fs_in.texCoord).r : uRoughness;

    // 环境光
    vec3 ambient = 0.05 * albedo;

    // 漫反射
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * light.color * albedo * light.intensity;

    // 镜面反射（Blinn-Phong）
    float spec = pow(max(dot(N, H), 0.0), mix(8.0, 256.0, 1.0 - roughness));
    vec3 specular = spec * light.color * mix(vec3(0.04), albedo, metallic) * light.intensity;

    return (ambient + (diffuse + specular) * attenuation);
}

void main() {
    // 获取法线
    vec3 N;
    if (uUseTextures > 0) {
        N = getNormalFromMap();
    } else {
        N = normalize(fs_in.normal);
    }

    vec3 V = normalize(uCameraPos - fs_in.worldPos);

    // 累加所有光源的贡献
    vec3 result = vec3(0.0);
    for (int i = 0; i < lights.length(); ++i) {
        result += calculateBlinnPhong(N, V, lights[i]);
    }

    // HDR色调映射
    result = result / (result + vec3(1.0));

    // Gamma校正
    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, uOpacity);
}
```

## 4. 纹理映射

### 4.1 纹理加载与管理

```cpp
// 纹理加载与管理类
#include <GL/glew.h>
#include <string>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class Texture {
public:
    enum class WrapMode {
        Repeat = GL_REPEAT,
        MirroredRepeat = GL_MIRRORED_REPEAT,
        ClampToEdge = GL_CLAMP_TO_EDGE,
        ClampToBorder = GL_CLAMP_TO_BORDER
    };

    enum class FilterMode {
        Nearest = GL_NEAREST,
        Linear = GL_LINEAR,
        NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
        LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR
    };

    bool loadFromFile(const std::string& path,
                      WrapMode wrapS = WrapMode::Repeat,
                      WrapMode wrapT = WrapMode::Repeat,
                      FilterMode minFilter = FilterMode::LinearMipmapLinear,
                      FilterMode magFilter = FilterMode::Linear) {
        // 翻转Y轴（OpenGL纹理原点在左下角）
        stbi_set_flip_vertically_on_load(true);

        int width, height, channels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        if (!data) {
            std::cerr << "纹理加载失败: " << path << std::endl;
            return false;
        }

        // 确定纹理格式
        GLenum format;
        if (channels == 1) format = GL_RED;
        else if (channels == 3) format = GL_RGB;
        else if (channels == 4) format = GL_RGBA;
        else {
            stbi_image_free(data);
            return false;
        }

        // 创建纹理
        glGenTextures(1, &textureId_);
        glBindTexture(GL_TEXTURE_2D, textureId_);

        // 上传纹理数据
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
                     format, GL_UNSIGNED_BYTE, data);

        // 生成Mipmap
        glGenerateMipmap(GL_TEXTURE_2D);

        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(wrapS));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(wrapT));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(minFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(magFilter));

        // 各向异性过滤
        if (GLEW_EXT_texture_filter_anisotropic) {
            float maxAniso;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);

        width_ = width;
        height_ = height;
        return true;
    }

    // 加载HDR纹理（用于环境贴图）
    bool loadHDR(const std::string& path) {
        stbi_set_flip_vertically_on_load(true);
        int width, height, channels;
        float* data = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
        if (!data) return false;

        glGenTextures(1, &textureId_);
        glBindTexture(GL_TEXTURE_2D, textureId_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0,
                     GL_RGB, GL_FLOAT, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        return true;
    }

    void bind(GLuint unit = 0) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, textureId_);
    }

    void unbind() const {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void cleanup() {
        if (textureId_) {
            glDeleteTextures(1, &textureId_);
            textureId_ = 0;
        }
    }

    GLuint getId() const { return textureId_; }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

private:
    GLuint textureId_ = 0;
    int width_ = 0, height_ = 0;
};
```

### 4.2 立方体贴图与天空盒

```cpp
// 立方体贴图（天空盒）
class Cubemap {
public:
    bool loadFromFiles(const std::vector<std::string>& faces) {
        // faces顺序：右、左、上、下、前、后
        glGenTextures(1, &textureId_);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureId_);

        int width, height, channels;
        for (GLuint i = 0; i < faces.size(); ++i) {
            unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &channels, 0);
            if (data) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             0, GL_RGB, width, height, 0,
                             GL_RGB, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            } else {
                std::cerr << "立方体贴图面加载失败: " << faces[i] << std::endl;
                stbi_image_free(data);
                return false;
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return true;
    }

    void bind(GLuint unit = 0) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureId_);
    }

    void cleanup() {
        if (textureId_) {
            glDeleteTextures(1, &textureId_);
            textureId_ = 0;
        }
    }

private:
    GLuint textureId_ = 0;
};
```

天空盒着色器：

```glsl
// 天空盒顶点着色器
#version 450 core
layout(location = 0) in vec3 aPosition;

uniform mat4 uView;        // 不含平移的视图矩阵
uniform mat4 uProjection;

out vec3 vWorldPos;

void main() {
    vWorldPos = aPosition;
    // 将深度设为最大值，确保天空盒在所有物体之后
    vec4 pos = uProjection * uView * vec4(aPosition, 1.0);
    gl_Position = pos.xyww;  // z = w，深度为1.0
}
```

```glsl
// 天空盒片段着色器
#version 450 core
in vec3 vWorldPos;
uniform samplerCube uSkybox;
out vec4 FragColor;

void main() {
    FragColor = texture(uSkybox, vWorldPos);
}
```

## 5. 光照模型

### 5.1 Phong与Blinn-Phong光照

```glsl
// Phong光照模型
vec3 calculatePhong(vec3 N, vec3 L, vec3 V, vec3 lightColor, vec3 albedo) {
    // 环境光
    vec3 ambient = 0.1 * albedo;

    // 漫反射
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor * albedo;

    // 镜面反射（Phong模型：使用反射向量R）
    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(R, V), 0.0), 32.0);
    vec3 specular = spec * lightColor * vec3(0.5);

    return ambient + diffuse + specular;
}

// Blinn-Phong光照模型（更高效，更物理正确）
vec3 calculateBlinnPhong(vec3 N, vec3 L, vec3 V, vec3 lightColor, vec3 albedo,
                         float shininess) {
    vec3 ambient = 0.1 * albedo;

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor * albedo;

    // Blinn-Phong：使用半程向量H代替反射向量R
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), shininess);
    vec3 specular = spec * lightColor * vec3(0.5);

    return ambient + diffuse + specular;
}
```

### 5.2 PBR（基于物理的渲染）

```glsl
// PBR核心函数
#version 450 core

const float PI = 3.14159265359;

// 法线分布函数（GGX/Trowbridge-Reitz）
float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom + 0.0001);
}

// 菲涅尔方程（Schlick近似）
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// 菲涅尔方程（带粗糙度的Schlick-GGX近似）
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) *
           pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// 几何遮蔽函数（Smith方法 + Schlick-GGX）
float geometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// PBR光照计算
vec3 calculatePBR(vec3 N, vec3 V, vec3 L, vec3 lightColor, vec3 albedo,
                  float metallic, float roughness) {
    vec3 H = normalize(V + L);

    // 基础反射率（电介质为0.04，金属使用albedo）
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Cook-Torrance BRDF
    float NDF = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    // 镜面反射项
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    // 能量守恒：ks + kd = 1
    vec3 kS = F;                         // 镜面反射比例
    vec3 kD = vec3(1.0) - kS;           // 漫反射比例
    kD *= (1.0 - metallic);              // 金属没有漫反射

    // 漫反射项（Lambertian）
    vec3 diffuse = kD * albedo / PI;

    // 最终颜色
    float NdotL = max(dot(N, L), 0.0);
    return (diffuse + specular) * lightColor * NdotL;
}
```

### 5.3 多光源PBR渲染

```glsl
// 完整的PBR片段着色器
#version 450 core

in VS_OUT {
    vec3 worldPos;
    vec3 normal;
    vec2 texCoord;
} fs_in;

uniform sampler2D uAlbedoMap;
uniform sampler2D uNormalMap;
uniform sampler2D uMetallicMap;
uniform sampler2D uRoughnessMap;
uniform sampler2D uAOMap;

uniform vec3 uLightPositions[4];
uniform vec3 uLightColors[4];
uniform vec3 uCameraPos;

out vec4 FragColor;

// ...（包含上面PBR函数的定义）

void main() {
    // 采样材质纹理
    vec3 albedo = pow(texture(uAlbedoMap, fs_in.texCoord).rgb, vec3(2.2)); // sRGB转线性
    float metallic = texture(uMetallicMap, fs_in.texCoord).r;
    float roughness = texture(uRoughnessMap, fs_in.texCoord).r;
    float ao = texture(uAOMap, fs_in.texCoord).r;

    vec3 N = normalize(fs_in.normal);
    vec3 V = normalize(uCameraPos - fs_in.worldPos);

    // 直接光照
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < 4; ++i) {
        vec3 L = normalize(uLightPositions[i] - fs_in.worldPos);
        float distance = length(uLightPositions[i] - fs_in.worldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = uLightColors[i] * attenuation;

        Lo += calculatePBR(N, V, L, radiance, albedo, metallic, roughness);
    }

    // 环境光照（简化版）
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 kD = (1.0 - fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness))
              * (1.0 - metallic);
    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo;

    // HDR色调映射（Reinhard）
    color = color / (color + vec3(1.0));

    // Gamma校正
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
```

## 6. OpenGL与现代C++结合

### 6.1 RAII资源管理

```cpp
// 使用RAII管理OpenGL资源
#include <GL/glew.h>
#include <memory>

// 自定义删除器，用于unique_ptr管理OpenGL对象
struct GLDeleter {
    void operator()(GLuint* ptr) const {
        if (ptr && *ptr != 0) {
            // 根据对象类型调用对应的删除函数
            // 这里简化处理，实际应区分类型
            glDeleteBuffers(1, ptr);
            delete ptr;
        }
    }
};

// 更完善的RAII封装
class GLBuffer {
public:
    GLBuffer() {
        glGenBuffers(1, &id_);
    }

    ~GLBuffer() {
        if (id_ != 0) {
            glDeleteBuffers(1, &id_);
        }
    }

    // 禁止拷贝
    GLBuffer(const GLBuffer&) = delete;
    GLBuffer& operator=(const GLBuffer&) = delete;

    // 支持移动
    GLBuffer(GLBuffer&& other) noexcept : id_(other.id_) {
        other.id_ = 0;
    }

    GLBuffer& operator=(GLBuffer&& other) noexcept {
        if (this != &other) {
            if (id_ != 0) glDeleteBuffers(1, &id_);
            id_ = other.id_;
            other.id_ = 0;
        }
        return *this;
    }

    void bind(GLenum target) const {
        glBindBuffer(target, id_);
    }

    void unbind(GLenum target) const {
        glBindBuffer(target, 0);
    }

    void uploadData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
        bind(target);
        glBufferData(target, size, data, usage);
    }

    GLuint getId() const { return id_; }

private:
    GLuint id_ = 0;
};

class GLVertexArray {
public:
    GLVertexArray() {
        glGenVertexArrays(1, &id_);
    }

    ~GLVertexArray() {
        if (id_ != 0) {
            glDeleteVertexArrays(1, &id_);
        }
    }

    GLVertexArray(const GLVertexArray&) = delete;
    GLVertexArray& operator=(const GLVertexArray&) = delete;

    GLVertexArray(GLVertexArray&& other) noexcept : id_(other.id_) {
        other.id_ = 0;
    }

    void bind() const { glBindVertexArray(id_); }
    void unbind() const { glBindVertexArray(0); }

    GLuint getId() const { return id_; }

private:
    GLuint id_ = 0;
};
```

### 6.2 渲染器类设计

```cpp
// 完整的渲染器类
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>

struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 color;
    float intensity;
};

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

class Renderer {
public:
    bool init(int width, int height) {
        // 启用深度测试
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // 启用背面剔除
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // 启用多重采样抗锯齿
        glEnable(GL_MULTISAMPLE);

        // 启用Gamma校正
        glEnable(GL_FRAMEBUFFER_SRGB);

        return true;
    }

    void beginFrame(const glm::vec4& clearColor = {0.1f, 0.1f, 0.1f, 1.0f}) {
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void setCamera(const glm::mat4& view, const glm::mat4& projection) {
        viewMatrix_ = view;
        projectionMatrix_ = projection;
    }

    void addDirectionalLight(const glm::vec3& direction,
                             const glm::vec3& color, float intensity) {
        dirLights_.push_back({direction, color, intensity});
    }

    void addPointLight(const glm::vec3& position,
                       const glm::vec3& color, float intensity) {
        pointLights_.push_back({position, color, intensity,
                                1.0f, 0.09f, 0.032f});
    }

    void drawMesh(const Mesh& mesh, const Shader& shader,
                  const glm::mat4& modelMatrix) {
        shader.use();

        // 设置变换矩阵
        shader.setMat4("uModel", glm::value_ptr(modelMatrix));
        shader.setMat4("uView", glm::value_ptr(viewMatrix_));
        shader.setMat4("uProjection", glm::value_ptr(projectionMatrix_));

        glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));
        shader.setMat4("uNormalMatrix", glm::value_ptr(normalMatrix));

        // 绘制
        mesh.draw();
    }

    void endFrame() {
        // 清空光源列表（每帧重新设置）
        dirLights_.clear();
        pointLights_.clear();
    }

private:
    glm::mat4 viewMatrix_ = glm::mat4(1.0f);
    glm::mat4 projectionMatrix_ = glm::mat4(1.0f);
    std::vector<DirectionalLight> dirLights_;
    std::vector<PointLight> pointLights_;
};
```

### 6.3 帧缓冲与后处理

```cpp
// 帧缓冲对象（FBO）封装 - 用于后处理效果
class Framebuffer {
public:
    bool init(int width, int height) {
        width_ = width;
        height_ = height;

        // 创建帧缓冲
        glGenFramebuffers(1, &fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

        // 创建颜色附件（HDR浮点纹理）
        glGenTextures(1, &colorTexture_);
        glBindTexture(GL_TEXTURE_2D, colorTexture_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0,
                     GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, colorTexture_, 0);

        // 创建深度/模板附件
        glGenRenderbuffers(1, &rbo_);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER, rbo_);

        // 检查帧缓冲完整性
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "帧缓冲不完整!" << std::endl;
            return false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return true;
    }

    void bindForWriting() const {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    }

    void bindForReading() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorTexture_);
    }

    void blitToScreen(int screenWidth, int screenHeight) const {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, width_, height_, 0, 0, screenWidth, screenHeight,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    void cleanup() {
        glDeleteFramebuffers(1, &fbo_);
        glDeleteTextures(1, &colorTexture_);
        glDeleteRenderbuffers(1, &rbo_);
    }

private:
    GLuint fbo_ = 0, colorTexture_ = 0, rbo_ = 0;
    int width_ = 0, height_ = 0;
};
```

后处理着色器示例：

```glsl
// 泛光（Bloom）效果 - 高通滤波提取亮区
#version 450 core
in vec2 vTexCoord;
uniform sampler2D uScene;
uniform float uThreshold;
out vec4 FragColor;

void main() {
    vec3 color = texture(uScene, vTexCoord).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > uThreshold) {
        FragColor = vec4(color, 1.0);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
```

```glsl
// 高斯模糊（两遍分离）
#version 450 core
in vec2 vTexCoord;
uniform sampler2D uImage;
uniform bool uHorizontal;
uniform float uWeight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
out vec4 FragColor;

void main() {
    vec2 texOffset = 1.0 / textureSize(uImage, 0);  // 单像素偏移
    vec3 result = texture(uImage, vTexCoord).rgb * uWeight[0];

    if (uHorizontal) {
        for (int i = 1; i < 5; ++i) {
            result += texture(uImage, vTexCoord + vec2(texOffset.x * i, 0.0)).rgb * uWeight[i];
            result += texture(uImage, vTexCoord - vec2(texOffset.x * i, 0.0)).rgb * uWeight[i];
        }
    } else {
        for (int i = 1; i < 5; ++i) {
            result += texture(uImage, vTexCoord + vec2(0.0, texOffset.y * i)).rgb * uWeight[i];
            result += texture(uImage, vTexCoord - vec2(0.0, texOffset.y * i)).rgb * uWeight[i];
        }
    }

    FragColor = vec4(result, 1.0);
}
```

## 7. 本章小结

本章深入讲解了OpenGL核心模式与着色器编程，核心要点如下：

| 主题 | 核心要点 |
|------|---------|
| OpenGL核心模式 | VAO/VBO/EBO数据组织、着色器程序编译链接 |
| GLSL着色器语言 | 数据类型、限定符、内置函数、swizzle操作 |
| 顶点/片段着色器 | 变换矩阵、属性传递、TBN矩阵、输出合并 |
| 纹理映射 | 2D纹理加载、立方体贴图、Mipmap、各向异性过滤 |
| 光照模型 | Phong/Blinn-Phong、PBR（GGX/Schlick/Smith）、多光源 |
| C++结合 | RAII资源管理、渲染器类设计、帧缓冲与后处理 |

**关键理解**：

1. **核心模式是现代OpenGL**：抛弃固定管线，完全由着色器控制渲染流程
2. **着色器是灵魂**：顶点着色器处理几何变换，片段着色器决定像素颜色
3. **PBR是工业标准**：基于物理的渲染提供更真实的光照效果
4. **RAII管理GL资源**：利用C++的RAII模式避免OpenGL资源泄漏
5. **后处理提升画面**：帧缓冲+着色器可以实现泛光、色调映射等效果

> **下一步**：在[Vulkan基础](02-Vulkan基础.md)中，我们将学习更底层、更高效的Vulkan图形API。
