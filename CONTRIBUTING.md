# 贡献指南

欢迎为本教程项目做出贡献！

## 1. 如何贡献

### 1. 报告问题

如果您发现文档错误、代码BUG或有改进建议，请[提交Issue](https://github.com/reai-xue/cpp-tutorial/issues)。

提交Issue时请包含：
- 清晰的标题
- 问题描述
- 复现步骤（如适用）
- 预期行为和实际行为

### 2. 提交代码

1. **Fork** 本仓库
2. 创建特性分支：`git checkout -b feature/your-feature`
3. 提交更改：`git commit -m 'Add some feature'`
4. 推送到分支：`git push origin feature/your-feature`
5. 提交 **Pull Request**

### 3. 文档规范

- 使用Markdown格式
- 保持代码示例简洁清晰
- 添加必要的注释说明
- 遵循现有文档的风格

### 4. 代码规范

- C代码遵循C17标准
- C++代码遵循C++20标准
- 使用有意义的变量名
- 添加函数注释
- 避免内存泄漏

## 2. 开发流程

```bash
# 克隆仓库
git clone https://github.com/reai-xue/cpp-tutorial.git
cd cpp-tutorial

# 创建分支
git checkout -b feature/your-feature

# 修改代码/文档...

# 测试构建
cd code
mkdir build && cd build
cmake ..
cmake --build .

# 提交更改
git add .
git commit -m "Your commit message"
git push origin feature/your-feature
```

## 3. 审查流程

所有Pull Request都需要经过至少一位维护者审查。审查要点：
- 代码质量
- 文档完整性
- 测试覆盖
- 风格一致性

## 4. 行为准则

- 尊重他人
- 接受建设性批评
- 关注技术问题
- 保持友善

感谢您的贡献！
