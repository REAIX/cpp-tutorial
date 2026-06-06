/**
 * @file 02_example_binary_serialize.cpp
 * @brief 二进制序列化: 版本控制, 字节序处理, 前后向兼容
 * @description 对应文档: 02-CPP/36-序列化与日志
 */

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdint>
#include <cstring>

class BinaryWriter {
    std::vector<uint8_t> buffer_;

public:
    void write_uint8(uint8_t val) {
        buffer_.push_back(val);
    }

    void write_uint16(uint16_t val) {
        buffer_.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
        buffer_.push_back(static_cast<uint8_t>(val & 0xFF));
    }

    void write_uint32(uint32_t val) {
        buffer_.push_back(static_cast<uint8_t>((val >> 24) & 0xFF));
        buffer_.push_back(static_cast<uint8_t>((val >> 16) & 0xFF));
        buffer_.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
        buffer_.push_back(static_cast<uint8_t>(val & 0xFF));
    }

    void write_int32(int32_t val) {
        write_uint32(static_cast<uint32_t>(val));
    }

    void write_float(float val) {
        uint32_t bits;
        std::memcpy(&bits, &val, sizeof(bits));
        write_uint32(bits);
    }

    void write_double(double val) {
        uint64_t bits;
        std::memcpy(&bits, &val, sizeof(bits));
        write_uint32(static_cast<uint32_t>((bits >> 32) & 0xFFFFFFFF));
        write_uint32(static_cast<uint32_t>(bits & 0xFFFFFFFF));
    }

    void write_string(const std::string& val) {
        write_uint32(static_cast<uint32_t>(val.size()));
        for (char c : val) {
            buffer_.push_back(static_cast<uint8_t>(c));
        }
    }

    const std::vector<uint8_t>& data() const { return buffer_; }

    void clear() { buffer_.clear(); }
};

class BinaryReader {
    const std::vector<uint8_t>& buffer_;
    size_t pos_ = 0;

public:
    explicit BinaryReader(const std::vector<uint8_t>& buf) : buffer_(buf) {}

    uint8_t read_uint8() {
        return buffer_[pos_++];
    }

    uint16_t read_uint16() {
        uint16_t val = (static_cast<uint16_t>(buffer_[pos_]) << 8) |
                       static_cast<uint16_t>(buffer_[pos_ + 1]);
        pos_ += 2;
        return val;
    }

    uint32_t read_uint32() {
        uint32_t val = (static_cast<uint32_t>(buffer_[pos_]) << 24) |
                       (static_cast<uint32_t>(buffer_[pos_ + 1]) << 16) |
                       (static_cast<uint32_t>(buffer_[pos_ + 2]) << 8) |
                       static_cast<uint32_t>(buffer_[pos_ + 3]);
        pos_ += 4;
        return val;
    }

    int32_t read_int32() {
        return static_cast<int32_t>(read_uint32());
    }

    float read_float() {
        uint32_t bits = read_uint32();
        float val;
        std::memcpy(&val, &bits, sizeof(val));
        return val;
    }

    double read_double() {
        uint32_t hi = read_uint32();
        uint32_t lo = read_uint32();
        uint64_t bits = (static_cast<uint64_t>(hi) << 32) | lo;
        double val;
        std::memcpy(&val, &bits, sizeof(val));
        return val;
    }

    std::string read_string() {
        uint32_t len = read_uint32();
        std::string result(reinterpret_cast<const char*>(&buffer_[pos_]), len);
        pos_ += len;
        return result;
    }

    bool has_more() const { return pos_ < buffer_.size(); }
    size_t position() const { return pos_; }
};

void demo_basic_binary_serialize() {
    std::cout << "\n=== demo_basic_binary_serialize ===\n";
    std::cout << "基本二进制序列化\n\n";

    BinaryWriter writer;
    writer.write_uint8(0xAB);
    writer.write_uint16(1234);
    writer.write_uint32(999999);
    writer.write_int32(-42);
    writer.write_float(3.14f);
    writer.write_double(2.718281828);
    writer.write_string("Hello, 二进制!");

    std::cout << "写入数据, 总大小: " << writer.data().size() << " 字节\n";

    BinaryReader reader(writer.data());
    std::cout << "  uint8:  0x" << std::hex << (int)reader.read_uint8() << std::dec << "\n";
    std::cout << "  uint16: " << reader.read_uint16() << "\n";
    std::cout << "  uint32: " << reader.read_uint32() << "\n";
    std::cout << "  int32:  " << reader.read_int32() << "\n";
    std::cout << "  float:  " << reader.read_float() << "\n";
    std::cout << "  double: " << reader.read_double() << "\n";
    std::cout << "  string: " << reader.read_string() << "\n";
}

void demo_endianness() {
    std::cout << "\n=== demo_endianness ===\n";
    std::cout << "字节序(Endianness)处理\n\n";

    uint32_t value = 0x12345678;

    std::cout << "原始值: 0x" << std::hex << value << std::dec << "\n";
    std::cout << "内存布局:\n";

    uint8_t* bytes = reinterpret_cast<uint8_t*>(&value);
    std::cout << "  本机字节序: ";
    for (int i = 0; i < 4; ++i) {
        std::cout << std::hex << (int)bytes[i] << " ";
    }
    std::cout << std::dec << "\n";

    bool is_little_endian = (bytes[0] == 0x78);
    std::cout << "  本机是: " << (is_little_endian ? "小端(Little-Endian)" : "大端(Big-Endian)") << "\n\n";

    std::cout << "网络字节序: 大端(Big-Endian)\n";
    std::cout << "序列化时: 统一使用大端 (网络字节序)\n\n";

    auto to_big_endian = [](uint32_t val) -> uint32_t {
        return ((val & 0xFF) << 24) | ((val & 0xFF00) << 8) |
               ((val >> 8) & 0xFF00) | ((val >> 24) & 0xFF);
    };

    uint32_t be_val = to_big_endian(value);
    std::cout << "转为大端: 0x" << std::hex << be_val << std::dec << "\n\n";

    std::cout << "字节序处理策略:\n";
    std::cout << "  1. 序列化时统一转为大端\n";
    std::cout << "  2. 反序列化时从大端转回本机序\n";
    std::cout << "  3. 使用htons/htonl/ntohs/ntohl\n";
    std::cout << "  4. 或使用自定义的读写函数(如本示例)\n";
    std::cout << "  5. 在文件头标记字节序, 读取时检测\n";
}

void demo_versioning() {
    std::cout << "\n=== demo_versioning ===\n";
    std::cout << "序列化版本控制\n\n";

    struct RecordV1 {
        std::string name;
        int32_t age;
    };

    struct RecordV2 {
        std::string name;
        int32_t age;
        std::string email;
        double score;
    };

    auto serialize_v1 = [](const RecordV1& r) {
        BinaryWriter w;
        w.write_uint8(1);
        w.write_string(r.name);
        w.write_int32(r.age);
        return w.data();
    };

    auto serialize_v2 = [](const RecordV2& r) {
        BinaryWriter w;
        w.write_uint8(2);
        w.write_string(r.name);
        w.write_int32(r.age);
        w.write_string(r.email);
        w.write_double(r.score);
        return w.data();
    };

    auto deserialize_any = [](const std::vector<uint8_t>& data) {
        BinaryReader r(data);
        uint8_t version = r.read_uint8();
        std::string name = r.read_string();
        int32_t age = r.read_int32();

        std::string email;
        double score = 0.0;

        if (version >= 2) {
            email = r.read_string();
            score = r.read_double();
        }

        std::cout << "  版本=" << (int)version << ", name=" << name
                  << ", age=" << age << ", email=" << email
                  << ", score=" << score << "\n";
    };

    std::cout << "V1数据 -> V2读取器:\n";
    RecordV1 v1{"张三", 25};
    auto v1_data = serialize_v1(v1);
    deserialize_any(v1_data);

    std::cout << "\nV2数据 -> V2读取器:\n";
    RecordV2 v2{"李四", 30, "lisi@example.com", 95.5};
    auto v2_data = serialize_v2(v2);
    deserialize_any(v2_data);

    std::cout << "\n版本控制策略:\n";
    std::cout << "  1. 在数据头部写入版本号\n";
    std::cout << "  2. 新版本只能添加字段, 不能删除或修改旧字段\n";
    std::cout << "  3. 旧版本读取器忽略新增字段\n";
    std::cout << "  4. 新版本读取器为缺失字段提供默认值\n";
    std::cout << "  5. 可选字段用长度前缀, 旧版本可跳过\n";
}

void demo_forward_backward_compatibility() {
    std::cout << "\n=== demo_forward_backward_compatibility ===\n";
    std::cout << "前向兼容与后向兼容\n\n";

    std::cout << "后向兼容 (Backward Compatibility):\n";
    std::cout << "  新代码能读取旧格式数据\n";
    std::cout << "  方法: 为新增字段提供默认值\n\n";

    std::cout << "前向兼容 (Forward Compatibility):\n";
    std::cout << "  旧代码能读取新格式数据\n";
    std::cout << "  方法: 旧代码跳过未知字段\n\n";

    std::cout << "实现技巧:\n";
    std::cout << "  1. TLV (Type-Length-Value) 编码:\n";
    std::cout << "     每个字段: [类型][长度][值]\n";
    std::cout << "     旧代码遇到未知类型, 根据长度跳过\n\n";

    std::cout << "  2. 长度前缀:\n";
    std::cout << "     每个字段前写入长度\n";
    std::cout << "     旧代码可跳过不认识的字段\n\n";

    std::cout << "  3. 可选字段标记:\n";
    std::cout << "     用位图标记哪些字段存在\n";
    std::cout << "     旧代码只读取认识的字段\n\n";

    std::cout << "生产环境建议:\n";
    std::cout << "  Protocol Buffers: 自带版本控制\n";
    std::cout << "  FlatBuffers: 零拷贝, 前向兼容\n";
    std::cout << "  MessagePack: 紧凑二进制JSON\n";
    std::cout << "  Cap'n Proto: 零拷贝, 高性能\n";
}

int main() {
    std::cout << "二进制序列化演示\n";

    demo_basic_binary_serialize();
    demo_endianness();
    demo_versioning();
    demo_forward_backward_compatibility();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
