/** @file 01_example_file_stream.cpp
 *  @brief 文件流基础：ifstream, ofstream, fstream, 文本/二进制模式
 *  @description 对应文档: 02-CPP/18-file-io | 演示文件读写的基本操作
 *  编译命令: g++ -std=c++20 01_example_file_stream.cpp -o 01_example_file_stream
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

const std::string TEST_DIR = "test_io_data/";

void ensure_test_dir() {
#ifdef _WIN32
    system("if not exist test_io_data mkdir test_io_data");
#else
    system("mkdir -p test_io_data");
#endif
}

void demo_ofstream_write() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  ofstream —— 文件写入\n";
    std::cout << "═══════════════════════════════════════\n\n";

    {
        std::ofstream out(TEST_DIR + "hello.txt");
        if (!out) {
            std::cerr << "无法打开文件进行写入\n";
            return;
        }
        out << "Hello, 文件IO!\n";
        out << "第二行内容\n";
        out << "数字: " << 42 << " 浮点: " << 3.14 << "\n";
    }

    {
        std::ofstream out(TEST_DIR + "append.txt", std::ios::app);
        out << "追加的第一行\n";
    }
    {
        std::ofstream out(TEST_DIR + "append.txt", std::ios::app);
        out << "追加的第二行\n";
    }

    {
        std::ofstream out(TEST_DIR + "lines.txt");
        std::vector<std::string> lines = {"苹果", "香蕉", "橙子", "葡萄"};
        for (const auto& line : lines) {
            out << line << "\n";
        }
    }

    std::cout << "文件写入完成，请查看 test_io_data/ 目录\n";
    std::cout << "  hello.txt   —— 基本写入\n";
    std::cout << "  append.txt  —— 追加模式写入\n";
    std::cout << "  lines.txt   —— 逐行写入\n";
}

void demo_ifstream_read() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  ifstream —— 文件读取\n";
    std::cout << "═══════════════════════════════════════\n\n";

    {
        std::ifstream in(TEST_DIR + "hello.txt");
        if (!in) {
            std::cerr << "无法打开文件进行读取\n";
            return;
        }

        std::cout << "1. 逐行读取 (getline):\n";
        std::string line;
        while (std::getline(in, line)) {
            std::cout << "  |" << line << "|\n";
        }
    }

    {
        std::ifstream in(TEST_DIR + "hello.txt");
        std::cout << "\n2. 逐词读取 (>>):\n";
        std::string word;
        while (in >> word) {
            std::cout << "  [" << word << "]\n";
        }
    }

    {
        std::ifstream in(TEST_DIR + "hello.txt");
        std::cout << "\n3. 逐字符读取 (get):\n";
        int count = 0;
        char ch;
        while (in.get(ch)) {
            count++;
        }
        std::cout << "  文件共 " << count << " 个字符\n";
    }

    {
        std::ifstream in(TEST_DIR + "hello.txt");
        std::cout << "\n4. 一次性读取全部内容:\n";
        std::string content((std::istreambuf_iterator<char>(in)),
                            std::istreambuf_iterator<char>());
        std::cout << "  内容长度: " << content.size() << " 字节\n";
        std::cout << "  内容: " << content;
    }
}

void demo_binary_io() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  二进制文件读写\n";
    std::cout << "═══════════════════════════════════════\n\n";

    struct Record {
        int id;
        double score;
        char name[32];
    };

    {
        std::ofstream out(TEST_DIR + "records.bin", std::ios::binary);
        if (!out) {
            std::cerr << "无法打开二进制文件\n";
            return;
        }

        Record records[] = {
            {1, 95.5, "张三"},
            {2, 87.3, "李四"},
            {3, 92.1, "王五"},
        };

        for (const auto& r : records) {
            out.write(reinterpret_cast<const char*>(&r), sizeof(Record));
        }
        std::cout << "写入 " << 3 << " 条记录到 records.bin\n";
    }

    {
        std::ifstream in(TEST_DIR + "records.bin", std::ios::binary);
        if (!in) {
            std::cerr << "无法打开二进制文件\n";
            return;
        }

        std::cout << "读取二进制记录:\n";
        Record r;
        while (in.read(reinterpret_cast<char*>(&r), sizeof(Record))) {
            std::cout << "  ID: " << r.id
                      << ", 姓名: " << r.name
                      << ", 分数: " << r.score << "\n";
        }
    }

    std::cout << "\n二进制 vs 文本模式:\n";
    std::cout << "  文本模式: 换行符转换(LF ↔ CRLF)，适合文本\n";
    std::cout << "  二进制模式: 原样读写，适合结构体、图片等\n";
    std::cout << "  Windows上文本模式会自动将 \\n 转为 \\r\\n\n";
    std::cout << "  Linux上两者无区别\n";
}

void demo_fstream_readwrite() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  fstream —— 读写模式\n";
    std::cout << "═══════════════════════════════════════\n\n";

    {
        std::fstream fs(TEST_DIR + "rw_test.txt",
                        std::ios::in | std::ios::out | std::ios::trunc);
        if (!fs) {
            std::cerr << "无法打开文件\n";
            return;
        }

        fs << "第一行\n第二行\n第三行\n";
        fs.flush();

        fs.seekg(0);
        std::string line;
        std::cout << "写入后立即读取:\n";
        while (std::getline(fs, line)) {
            std::cout << "  " << line << "\n";
        }
    }

    std::cout << "\n打开模式标志:\n";
    std::cout << "  ios::in      —— 读\n";
    std::cout << "  ios::out     —— 写\n";
    std::cout << "  ios::app     —— 追加(每次写入在末尾)\n";
    std::cout << "  ios::ate     —— 打开时定位到末尾\n";
    std::cout << "  ios::trunc   —— 截断(清空文件)\n";
    std::cout << "  ios::binary  —— 二进制模式\n";
}

void demo_file_seek() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  文件定位 (seek)\n";
    std::cout << "═══════════════════════════════════════\n\n";

    {
        std::ofstream out(TEST_DIR + "seek_test.bin", std::ios::binary);
        for (int i = 0; i < 10; i++) {
            out.write(reinterpret_cast<const char*>(&i), sizeof(int));
        }
    }

    {
        std::fstream fs(TEST_DIR + "seek_test.bin",
                        std::ios::in | std::ios::out | std::ios::binary);

        int value = 99;
        fs.seekp(5 * sizeof(int), std::ios::beg);
        fs.write(reinterpret_cast<const char*>(&value), sizeof(int));

        fs.seekg(0, std::ios::beg);
        std::cout << "修改第6个元素为99后:\n  ";
        for (int i = 0; i < 10; i++) {
            int v;
            fs.read(reinterpret_cast<char*>(&v), sizeof(int));
            std::cout << v << " ";
        }
        std::cout << "\n";
    }

    std::cout << "\n定位函数:\n";
    std::cout << "  seekg(pos)          —— 设置读位置(绝对)\n";
    std::cout << "  seekg(off, dir)     —— 设置读位置(相对)\n";
    std::cout << "  seekp(pos)          —— 设置写位置(绝对)\n";
    std::cout << "  seekp(off, dir)     —— 设置写位置(相对)\n";
    std::cout << "  tellg()             —— 获取读位置\n";
    std::cout << "  tellp()             —— 获取写位置\n";
    std::cout << "\n方向:\n";
    std::cout << "  ios::beg  —— 文件开头\n";
    std::cout << "  ios::cur  —— 当前位置\n";
    std::cout << "  ios::end  —— 文件末尾\n";
}

int main() {
    ensure_test_dir();
    demo_ofstream_write();
    demo_ifstream_read();
    demo_binary_io();
    demo_fstream_readwrite();
    demo_file_seek();
    return 0;
}
