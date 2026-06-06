# 字符编码、解码与 Unicode 详解
> 📖 相关章节：[指针](../../01-C语言/06-指针.md)、[结构体与联合体](../../01-C语言/08-结构体与联合体.md)、[内存管理](../../01-C语言/09-内存管理.md)、[智能指针](../../02-CPP/08-智能指针与内存管理.md)、[内存模型](../../02-CPP/32-内存模型.md)

## 1. 为什么会有字符编码问题？

计算机只能处理二进制数据（0 和 1），但人类需要处理文字。**字符编码**就是将人类可读的字符（如 'A'、'中'）转换为计算机可存储的二进制数据的规则。

**核心问题：**
- 不同语言有不同的字符集（如英文只有26个字母，中文有数万个汉字）
- 不同国家/地区曾使用不同的编码方案，导致兼容性问题

---

## 2. 基本概念

### 1. 字符集 (Character Set)

字符集是**字符的集合**，定义了有哪些字符。

- **ASCII**：美国信息交换标准代码，包含英文字母、数字、标点符号等共128个字符
- **GB2312/GBK/GB18030**：中文国家标准字符集
- **Unicode**：国际通用字符集，包含世界上几乎所有语言的字符

### 2. 字符编码 (Character Encoding)

字符编码是**字符到二进制的映射规则**。同一个字符集可以有多种编码方式。

**常见编码方式：**

| 编码 | 字符集 | 特点 |
|------|--------|------|
| ASCII | ASCII | 1字节，只能表示英文 |
| UTF-8 | Unicode | 变长编码（1-4字节），兼容ASCII |
| UTF-16 | Unicode | 变长编码（2或4字节） |
| UTF-32 | Unicode | 定长编码（4字节） |
| GBK | GB2312扩展 | 双字节，支持中文 |

### 3. 编码与解码

```
编码 (Encode)：字符 → 字节序列
解码 (Decode)：字节序列 → 字符
```

**示例：**
```cpp
// 字符 'A' 的编码与解码
// 字符 'A' → 编码 → 字节 0x41
// 字节 0x41 → 解码 → 字符 'A'

// 字符 '中' 的编码与解码（UTF-8）
// 字符 '中' → 编码 → 字节 0xE4 0xB8 0xAD
// 字节 0xE4 0xB8 0xAD → 解码 → 字符 '中'
```

---

## 3. Unicode 是什么？

**Unicode** 是一个**字符集**，它的目标是包含世界上所有语言的字符。

### 1. Unicode 的特点

1. **统一字符集**：包含超过14万个字符，涵盖几乎所有已知语言
2. **字符码点**：每个字符有唯一的数字标识（码点），如：
   - 'A' 的码点是 U+0041
   - '中' 的码点是 U+4E2D
   - 'α' 的码点是 U+03B1

3. **不是编码方式**：Unicode 只定义了字符和码点的对应关系，不规定如何存储

### 2. Unicode 的编码方案

Unicode 需要通过具体的编码方案转换为二进制：

```
┌─────────────────────────────────────────────────────────────┐
│                      Unicode 字符集                         │
│  字符 'A' → 码点 U+0041                                    │
│  字符 '中' → 码点 U+4E2D                                   │
└──────────────────────┬──────────────────────────────────────┘
                       │
        ┌──────────────┼──────────────┐
        ▼              ▼              ▼
   ┌──────────┐   ┌──────────┐   ┌──────────┐
   │  UTF-8   │   │ UTF-16   │   │ UTF-32   │
   └────┬─────┘   └────┬─────┘   └────┬─────┘
        │              │              │
        ▼              ▼              ▼
   0x41          0x00 0x41      0x00 0x00 0x00 0x41  ('A')
   0xE4 0xB8 0xAD  0x4E 0x2D      0x00 0x00 0x4E 0x2D  ('中')
```

---

## 4. UTF-8 编码详解

UTF-8 是目前最流行的 Unicode 编码方式，具有以下特点：

### 1. 编码规则

| 码点范围 | 字节数 | 编码格式 |
|----------|--------|----------|
| U+0000 ~ U+007F | 1 | `0xxxxxxx` |
| U+0080 ~ U+07FF | 2 | `110xxxxx 10xxxxxx` |
| U+0800 ~ U+FFFF | 3 | `1110xxxx 10xxxxxx 10xxxxxx` |
| U+10000 ~ U+10FFFF | 4 | `11110xxx 10xxxxxx 10xxxxxx 10xxxxxx` |

### 2. UTF-8 的优势

1. **兼容 ASCII**：ASCII 字符（U+0000 ~ U+007F）用1字节表示，与 ASCII 完全兼容
2. **节省空间**：英文文本与 ASCII 相同大小，中文通常用3字节
3. **无字节序问题**：不需要考虑大端/小端

---

## 5. 编码问题的常见场景

### 1. 文件编码错误

```cpp
// 错误示例：用错误的编码打开文件
std::ifstream file("test.txt", std::ios::binary);
// 如果文件是 UTF-8 编码，但程序按 GBK 解码，会出现乱码
```

### 2. 网络传输

```cpp
// 网络传输需要明确编码
std::string data = "你好";
// 确保发送和接收双方使用相同的编码（通常是 UTF-8）
send(socket, data.c_str(), data.size(), 0);
```

### 3. 字符串处理

```cpp
// C++ 中的字符串编码
std::string str = u8"你好";  // UTF-8 字符串（C++11）
std::wstring wstr = L"你好"; // 宽字符串（平台相关编码）
```

### 4. 以 UTF-8 方式读取文本

**方法1：二进制读取 + 直接存储为 UTF-8**

```cpp
#include <fstream>
#include <string>

std::string read_utf8_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("无法打开文件");
    }
    
    // 读取全部内容
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    return content;  // content 包含 UTF-8 编码的文本
}

// 使用
std::string text = read_utf8_file("utf8_file.txt");
// text 现在包含 UTF-8 编码的内容，可以直接处理
```

**方法2：使用 std::wifstream + 语言环境（C++11及以上）**

```cpp
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>

std::wstring read_utf8_file_wide(const std::string& filename) {
    std::wifstream file(filename);
    
    // 设置 UTF-8 语言环境
    file.imbue(std::locale(file.getloc(), 
                          new std::codecvt_utf8<wchar_t>()));
    
    std::wstringstream wss;
    wss << file.rdbuf();
    
    return wss.str();  // 返回宽字符串（wchar_t）
}
```

**方法3：C++20 std::u8string 方式**

```cpp
#include <fstream>
#include <string>

// C++20 可以直接读取为 std::u8string
std::u8string read_utf8_file_cpp20(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("无法打开文件");
    }
    
    std::string bytes((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
    
    // 转换为 std::u8string
    return std::u8string(reinterpret_cast<const char8_t*>(bytes.data()), 
                         bytes.size());
}
```

**注意事项：**

1. **BOM 处理**：UTF-8 文件可能包含 BOM（0xEF 0xBB 0xBF），需要手动处理
2. **错误处理**：读取后应验证 UTF-8 有效性
3. **跨平台**：Windows 和 Linux/macOS 的换行符不同（\r\n vs \n）

**完整示例（包含 BOM 处理）：**

```cpp
#include <fstream>
#include <string>

bool has_utf8_bom(const std::string& data) {
    return data.size() >= 3 && 
           static_cast<unsigned char>(data[0]) == 0xEF &&
           static_cast<unsigned char>(data[1]) == 0xBB &&
           static_cast<unsigned char>(data[2]) == 0xBF;
}

std::string read_utf8_file_safe(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("无法打开文件: " + filename);
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    // 移除 UTF-8 BOM（如果存在）
    if (has_utf8_bom(content)) {
        content = content.substr(3);
    }
    
    return content;
}
```

---

## 6. C++ 中的编码处理

### 1. 字符类型与编码

| 类型 | 编码 | 说明 |
|------|------|------|
| `char` | 通常是 UTF-8 或 ASCII | 窄字符 |
| `wchar_t` | 平台相关（Windows: UTF-16, Linux: UTF-32） | 宽字符 |
| `char16_t` | UTF-16 | C++11 |
| `char32_t` | UTF-32 | C++11 |
| `char8_t` | UTF-8 | C++20 |

### 2. 字符串字面量前缀

```cpp
char* s1 = "hello";      // 窄字符串，编码取决于源文件
char* s2 = u8"hello";    // UTF-8 字符串（C++11）
wchar_t* s3 = L"hello";  // 宽字符串
char16_t* s4 = u"hello"; // UTF-16 字符串（C++11）
char32_t* s5 = U"hello"; // UTF-32 字符串（C++11）
```

### 3. 编码转换示例

```cpp
#include <codecvt>
#include <locale>
#include <string>

// UTF-8 转宽字符串
std::wstring utf8_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

// 宽字符串转 UTF-8
std::string wstring_to_utf8(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}
```

---

## 7. 常见问题与误区

### 1. 误区1：Unicode 是一种编码方式

**纠正：** Unicode 是字符集，UTF-8/UTF-16/UTF-32 才是编码方式。

### 2. 误区2：所有中文都是双字节

**纠正：** 在 UTF-8 中，中文通常是3字节；在 GBK 中是2字节。

### 3. 误区3：char 一定是1字节

**纠正：** C++ 标准只规定 `char` 至少1字节，但某些平台可能更大。

### 4. 误区4：字符串长度就是字符数

**纠正：** 在 UTF-8 中，`std::string::size()` 返回的是字节数，不是字符数。

```cpp
std::string str = u8"你好";
str.size();      // 返回 6（3字节 × 2个字符）
// 需要使用专门的 Unicode 库来计算字符数
```

### 5. 误区5：char8_t 支持中文吗？

**答：** 支持，但需要注意理解方式。

`char8_t` 是 C++20 新增的类型，用于表示 **UTF-8 编码的单个字节**。

**关键点：**

1. **单个 char8_t 不存储完整中文字符**：
   - `char8_t` 是 1 字节类型
   - 中文在 UTF-8 中通常需要 3 字节表示
   - 单个 `char8_t` 只能存储中文字符的一个字节部分

2. **char8_t 数组/字符串支持中文**：
   ```cpp
   // 正确：使用 char8_t 数组存储 UTF-8 字符串
   const char8_t* chinese = u8"你好世界";
   
   // 每个中文字符占用3个 char8_t 字节
   // '你' → 0xE4 0xBD 0xA0（3个 char8_t）
   // '好' → 0xE5 0xA5 0xBD（3个 char8_t）
   ```

3. **std::u8string 支持中文**：
   ```cpp
   #include <string>
   
   std::u8string str = u8"你好世界";  // C++20
   // str 包含 12 个 char8_t（4个汉字 × 3字节）
   ```

**总结：** `char8_t` 类型本身是 UTF-8 编码的字节类型，可以用于存储中文文本，但单个 `char8_t` 不能存储完整的中文字符，需要使用数组或字符串。

---

## 8. 总结

```
字符集：定义有哪些字符（如 Unicode 包含全球字符）
字符编码：字符到二进制的映射规则（如 UTF-8、UTF-16）
编码：字符 → 字节（如 '中' → 0xE4 0xB8 0xAD）
解码：字节 → 字符（如 0xE4 0xB8 0xAD → '中'）

Unicode 是字符集，UTF-8/UTF-16/UTF-32 是编码方案
UTF-8 是最常用的编码，兼容 ASCII，节省空间
```

---

## 8. 极简口诀

```
字符集定有哪些，编码规则怎么存
Unicode 大一统，UTF-8 最常用
编码字符变字节，解码字节变字符
乱码多因编码错，统一 UTF-8 是关键

***

### 相关阅读

- [位操作技巧](./15-位操作技巧.md)
```