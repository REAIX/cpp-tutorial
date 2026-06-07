# 虚函数表 vtable 详解
> 📖 相关章节：[继承与多态](../../02-CPP/04-继承与多态.md)、[类与对象](../../02-CPP/03-类与对象.md)

### 1. 精髓速览

**vtable** = **虚函数表**：编译器为每个含虚函数的类生成一张函数指针表，对象里存一个指针（vptr）指向这张表，运行时通过 vptr→vtable→函数指针 实现多态调用。

***

### 2. vtable 是什么

```cpp
class Animal {
public:
    virtual void speak() { cout << "Animal" << endl; }
    virtual ~Animal() = default;
};

class Dog : public Animal {
public:
    void speak() override { cout << "Woof" << endl; }
};
```

编译器为 `Animal` 和 `Dog` 各生成一张 vtable：

```
Animal vtable:          Dog vtable:
+------------------+   +------------------+
| Animal::speak    |   | Dog::speak       |
| Animal::~Animal  |   | Dog::~Dog        |
+------------------+   +------------------+
```

vtable 的存储位置：
- vtable 本身存储在 **代码段（.rodata）**，只读
- vptr 存储在 **对象内部**（通常开头），每个对象一份
- vtable 每类只有一份，由所有对象共享

### 3. vptr 在哪

每个对象内部藏着一个 **vptr**（虚函数表指针），指向所属类的 vtable：

```cpp
Animal* a = new Dog();
// a 指向的对象内部：
//   [vptr] → Dog 的 vtable
// a->speak() 的过程：
//   1. 通过 a 找到 vptr
//   2. 通过 vptr 找到 Dog 的 vtable
//   3. 从 vtable 中找到 Dog::speak 的地址
//   4. 调用 Dog::speak
```

多态调用性能分析：
```
虚函数调用:  mov  rax, [rcx]          ; 取 vptr（1次内存访问）
            call [rax + offset]       ; 取 vtable 中的函数指针（1次内存访问）
                                       ; 总共：2次内存访问 + 1次间接调用

普通函数调用: call func_addr           ; 1次直接调用
```

### 4. 多继承的 vtable

```cpp
class Base1 { virtual void f1(); };
class Base2 { virtual void f2(); };
class Derived : public Base1, public Base2 {
    void f1() override;
    void f2() override;
};
```

**Derived 对象内存布局**：

```
+------------------+
| vptr1 → vtable1  |  ← Base1 子对象
| Base1 成员       |
+------------------+
| vptr2 → vtable2  |  ← Base2 子对象
| Base2 成员       |
+------------------+
| Derived 成员     |
+------------------+
```

多继承 → 多个 vptr → 多张 vtable。

多继承下 vtable 的详细内容：
```
Derived 的 vtable1（对应 Base1）:
+--------------------------+
| offset_to_top (0)        |  ← this 指针调整偏移
| typeinfo for Derived     |  ← RTTI 信息
| Derived::f1()            |  ← 覆盖 Base1::f1
| Derived::f2()            |  ← 从 Base2 转发
+--------------------------+

Derived 的 vtable2（对应 Base2）:
+--------------------------+
| offset_to_top (-sizeof(Base1部分)) |  ← this 调整
| typeinfo for Derived              |
| thunk: Derived::f2()              |  ← 跳转调整 this 指针
+--------------------------+
```

**thunk 函数**：多继承中调用派生类覆盖的虚函数时，需先将 this 指针调整到正确的位置。

```cpp
Derived* d = new Derived();
Base2* b2 = d;  // 指针偏移到 Base2 子对象
b2->f2();       // 实际调用 Derived::f2()，但 this 需调整回 Derived 起始地址
                // 编译器自动插入 thunk 完成 this 调整
```

### 5. 虚继承的 vtable

```cpp
class Base { virtual void foo(); int a; };
class D1 : virtual public Base { int b; };
class D2 : virtual public Base { int c; };
class Final : public D1, public D2 { int d; };
```

虚继承的 vtable 更复杂，需要额外维护 **vbase offset**：

```
Final 对象布局：
+------------------+
| vptr1 → D1 vtable|  ← D1 部分
| b                 |
+------------------+
| vptr2 → D2 vtable|  ← D2 部分
| c                 |
+------------------+
| d                 |  ← Final 部分
+------------------+
| vptr_base → Base  |  ← 共享 Base 部分
| a                 |
+------------------+
```

D1 的 vtable 内容示例：
```
D1 vtable (用于 Final):
+--------------------------+
| vbase_offset (到 Base 的偏移) |  ← 找到共享 Base 子对象
| offset_to_top              |
| typeinfo for Final         |
| D1::foo() override         |
+--------------------------+
```

### 6. vtable 与 RTTI

RTTI（运行时类型识别）信息存储在 vtable 的头部：

```cpp
#include <typeinfo>

class Base { virtual ~Base() = default; };
class Derived : public Base {};

Base* b = new Derived();

// typeid 通过 vptr → vtable → typeinfo 获取类型信息
const std::type_info& ti = typeid(*b);
cout << ti.name() << endl;  // "class Derived"

// dynamic_cast 也依赖 vtable
Derived* d = dynamic_cast<Derived*>(b);
// 内部实现：
// 1. 通过 vptr 找到 vtable
// 2. 从 vtable 头部获取 typeinfo
// 3. 比较类型信息，决定转换是否合法

// 没有虚函数的类不能使用 RTTI
class NoVtable {};
// typeid(NoVtable)  // 编译期确定，不需要 vtable
// dynamic_cast  // 编译错误
```

vtable 中的 RTTI 信息布局（Itanium C++ ABI）：
```
vtable 结构:
+--------------------------+
| offset_to_top            |  ← 对象起始地址到 vtable 所在子对象的偏移
| typeinfo pointer         |  ← 指向 type_info 对象
+--------------------------+
| vfunc[0]                 |  ← 第一个虚函数
| vfunc[1]                 |  ← 第二个虚函数
| ...                      |
+--------------------------+
```

### 7. vtable 的开销

| 开销项 | 说明 |
|--------|------|
| 对象大小 | 每个对象多一个 vptr（通常8字节） |
| 内存 | 每个类一张 vtable（代码段） |
| 调用开销 | 一次间接寻址（vptr→vtable→函数） |
| 缓存影响 | 间接跳转可能影响指令缓存 |
| 内联限制 | 虚函数不能内联（除非编译器能确定实际类型） |
| RTTI 额外开销 | typeid/dynamic_cast 需查 vtable |

```cpp
// vptr 对对象大小的影响
class Empty {};                     // sizeof = 1
class WithVirtual {
    virtual void foo() {}
};                                  // sizeof = 8（vptr）+ 对齐

// 菱形继承中的 vptr 数量
class Base { virtual ~Base() = default; };
class A : virtual public Base {};
class B : virtual public Base {};
class Diamond : public A, public B {};
// Diamond 有 3 个 vptr（A、B、Base 各一个）
```

### 8. 什么时候没有 vtable

- 类没有虚函数 → 没有 vtable
- 类只有 `final` 虚函数 → 仍有 vtable（但可能被优化）
- 用 CRTP 替代虚函数 → 完全没有 vtable

```cpp
// CRTP（奇异递归模板模式）：编译期多态，无 vtable
template <typename Derived>
class Base {
public:
    void interface() {
        static_cast<Derived*>(this)->implementation();
    }
};

class Impl : public Base<Impl> {
public:
    void implementation() {
        cout << "Impl" << endl;
    }
};
// 没有虚函数 → 没有 vtable → 没有运行时开销
```

### 9. 手动访问 vtable（不推荐，仅用于理解）

```cpp
class Base {
public:
    virtual void f1() { cout << "Base::f1" << endl; }
    virtual void f2() { cout << "Base::f2" << endl; }
    virtual ~Base() = default;
};

// 通过指针操作访问 vtable（实现相关，不可移植！）
Base b;
void** vptr = *(void***)(&b);  // 取 vptr
using FuncPtr = void(*)();
FuncPtr f1 = (FuncPtr)vptr[0];  // vtable 中的第一个函数
f1();  // 调用 Base::f1()
```

### 10. 极简总结

**vtable = 每个多态类一份 + 存虚函数指针 + 编译期生成 + 只读共享；vptr = 每个对象一个 + 构造时初始化 + 指向所属类的vtable → 多态 = 运行时通过 vptr→vtable→函数指针 间接调用 → 代价 = 额外内存 + 间接跳转 + 不可内联**

***

### 相关阅读

- [C++对象内存布局](12-C++对象内存布局.md)
- [动态绑定与静态绑定](../04-CPP核心特性/12-动态绑定与静态绑定.md)
- [虚析构函数为什么重要](11-虚析构函数为什么重要.md)