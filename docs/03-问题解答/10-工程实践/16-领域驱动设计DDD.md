# 领域驱动设计（DDD）— 让代码说业务语言
> 📖 相关章节：[编码规范](../../04-工程实践/00-编码规范.md)、[设计模式](../../04-工程实践/03-设计模式.md)、[单元测试](../../04-工程实践/05-单元测试.md)、[代码审查](../../04-工程实践/07-代码审查.md)

> "如果你不能向一个5岁的孩子解释你的系统在做什么，那你的设计就有问题。" — DDD的核心思想：代码应该反映业务，而不是反过来。

---

## 1. DDD是什么 — 用业务驱动设计，而不是用技术驱动

### 1. 一句话定义

**领域驱动设计（Domain-Driven Design，DDD）** 是一种软件开发方法论，核心思想是：**软件的结构应该反映业务的结构，代码应该用业务语言来写。**

### 2. 通俗比喻

```
传统开发方式（技术驱动）：

  产品经理说："用户可以下单买商品"
  开发者写成：
    insert_order(user_id, product_id, quantity)
    update_stock(product_id, -quantity)
    create_payment_record(order_id, amount)

  问题：代码里全是技术术语（insert/update/create），
       看不出业务在做什么

DDD开发方式（业务驱动）：

  产品经理说："用户可以下单买商品"
  开发者写成：
    order = PlaceOrder(customer, product, quantity)
    order.reserveStock()
    order.requestPayment()

  优势：代码就像在说业务语言——"下单""预留库存""请求支付"
       产品经理几乎能看懂代码在做什么
```

**更精确的比喻**：DDD像翻译官——把业务专家脑子里的知识，精确地翻译成代码结构。不是让业务适应代码，而是让代码适应业务。

### 3. DDD要解决什么问题

```
传统开发中的典型问题：

问题1：沟通鸿沟
  └── 开发者说"数据库表加个字段"
  └── 业务专家听不懂
  └── 业务专家说"客户可以退货"
  └── 开发者理解成"删除订单记录"
  └── 双方说的不是同一种语言

问题2：贫血模型
  └── "用户"只是一个数据容器（getter/setter）
  └── 所有业务逻辑都在Service层
  └── 对象没有行为，只是数据库表的映射
  └── 业务规则散落在各处，难以维护

问题3：大泥球
  └── 所有功能搅在一起
  └── 改一个功能要改十几个文件
  └── 新人看不懂系统在做什么
  └── 代码和业务脱节

DDD的解决方案：
  └── 统一语言 → 消除沟通鸿沟
  └── 充血模型 → 对象=数据+行为
  └── 限界上下文 → 划分边界，隔离复杂度
```

---

## 2. DDD的核心概念 — 7个关键词

### 1. 统一语言（Ubiquitous Language）

**比喻**：统一语言像团队内部的"官方语言"——不管是产品经理、开发者还是测试，都说同一种术语，写代码也用这些术语。

```
没有统一语言：
  产品经理说："客户取消订单"
  开发者写成：cancel_order()
  数据库字段：order_status = 3
  测试用例：test_order_cancellation()

  三个人用了三种表达，沟通成本高

有统一语言：
  产品经理说："取消订单"
  开发者写成：Order.cancel()
  数据库字段：status = CANCELLED
  测试用例：test_cancel_order()

  所有人用同一套术语，零沟通成本
```

**统一语言的规则**：

| 规则 | 说明 | 反例 |
|------|------|------|
| 术语唯一 | 同一个概念只用一个词 | "用户"/"客户"/"账户"混用 |
| 代码即文档 | 代码中的命名就是业务术语 | `data1`/`info`/`tmp` |
| 拒绝技术术语 | 业务讨论中不用技术词汇 | "加个外键"→"建立关联关系" |
| 持续演进 | 随业务理解加深而更新 | 一开始叫"订单"，后来发现应该是"工单" |

### 2. 限界上下文（Bounded Context）

**比喻**：限界上下文像公司的部门——销售部说"客户"指的是买方，法务部说"客户"指的是合同签署方，技术部说"客户"指的是系统用户。同一个词在不同部门有不同含义，每个部门就是一个限界上下文。

```
电商系统中的"商品"在不同上下文中的含义：

┌─────────────────────────────────────────────────────┐
│  商品目录上下文                                      │
│  "商品" = 名称、描述、图片、分类、品牌               │
│  关心：展示、搜索、分类                              │
├─────────────────────────────────────────────────────┤
│  库存上下文                                          │
│  "商品" = SKU、数量、仓库位置、安全库存              │
│  关心：有多少、在哪、够不够                          │
├─────────────────────────────────────────────────────┤
│  订单上下文                                          │
│  "商品" = 单价、折扣、数量、小计                     │
│  关心：多少钱、买几个                                │
├─────────────────────────────────────────────────────┤
│  物流上下文                                          │
│  "商品" = 重量、体积、易碎标记、配送要求             │
│  关心：多重、多大、怎么送                            │
└─────────────────────────────────────────────────────┘

同一个"商品"，4个上下文4种理解
  └── 不要试图用一个"商品"类包含所有信息
  └── 而是让每个上下文有自己的"商品"模型
```

**限界上下文的划分原则**：

```
如何划分限界上下文？

1. 按业务能力划分（推荐）
   └── 一个上下文 = 一个独立的业务能力
   └── 例：订单、库存、支付、物流

2. 按语言边界划分
   └── 同一个词在不同地方含义不同 → 不同上下文
   └── 例："账户"在用户上下文=个人信息，在财务上下文=余额

3. 按团队划分（康威定律）
   └── 系统架构应该反映组织架构
   └── 一个团队负责一个上下文
```

### 3. 实体（Entity）

**比喻**：实体像人——每个人都有唯一的身份证号，即使你改了名字、换了发型，你还是你（身份不变，属性可变）。

```
实体的核心特征：
  ├── 有唯一标识（ID）
  ├── 属性可以变化
  └── 通过ID判断是否是同一个对象

例：订单是实体
  ├── 唯一标识：订单号（ORDER-2024-001）
  ├── 属性变化：状态从"待支付"→"已支付"→"已发货"
  └── 不管状态怎么变，订单号不变，还是同一个订单

反例：收货地址不是实体（在DDD意义上）
  └── 没有独立身份，只是订单的一个属性
```

### 4. 值对象（Value Object）

**比喻**：值对象像钞票——两张100元是等价的，你不在乎用的是哪张，只在乎面值。值对象没有身份，只关心值。

```
值对象的核心特征：
  ├── 没有唯一标识
  ├── 通过属性值判断相等性
  └── 不可变（创建后不能修改）

实体 vs 值对象：

  实体（Entity）：有身份证的人
    └── 张三改了名字还是张三
    └── 通过ID区分

  值对象（Value Object）：100元钞票
    └── 两张100元没有区别
    └── 通过值区分
    └── 不需要"这张100元的编号是多少"

代码示例：

  // 实体
  class Order {
      string order_id;    // 唯一标识
      Money total;        // 值对象
      Address shipping;   // 值对象
      OrderStatus status; // 值对象（枚举）
  };

  // 值对象
  class Money {
      const int amount;       // 不可变
      const string currency;  // 不可变

      bool equals(const Money& other) const {
          return amount == other.amount && currency == other.currency;
      }

      Money add(const Money& other) const {
          return Money(amount + other.amount, currency);  // 返回新对象
      }
  };
```

### 5. 聚合（Aggregate）

**比喻**：聚合像原子——原子由原子核和电子组成，你不会单独操作一个电子，而是操作整个原子。聚合是一组必须一起操作的对象的边界。

```
聚合的核心概念：

聚合根（Aggregate Root）= 原子核
  └── 外部只能通过聚合根访问聚合内的对象
  └── 聚合根保证内部一致性

聚合内对象 = 电子
  └── 不能被外部直接访问
  └── 由聚合根管理

例：订单聚合

┌─────────────────────────────────────┐
│         订单聚合                     │
│                                     │
│  [Order] ← 聚合根                   │
│    ├── order_id                     │
│    ├── status                       │
│    ├── [OrderItem] ← 聚合内实体     │
│    │     ├── product_id             │
│    │     ├── quantity               │
│    │     └── price                  │
│    ├── [OrderItem]                  │
│    └── [Address] ← 值对象           │
│                                     │
│  规则：                             │
│  1. 外部不能直接修改OrderItem        │
│  2. 必须通过Order来操作              │
│  3. Order保证：订单总金额=所有项之和 │
└─────────────────────────────────────┘

为什么需要聚合？
  └── 保证数据一致性
  └── 如果外部能直接改OrderItem的价格
  └── 但Order的总金额没更新 → 数据不一致
  └── 聚合根负责维护所有内部一致性规则
```

**聚合的设计原则**：

| 原则 | 说明 | 原因 |
|------|------|------|
| 小聚合 | 聚合尽量小，只包含必须一致的对象 | 大聚合=高争用=低性能 |
| 通过ID引用 | 聚合之间通过ID引用，不直接持有对象 | 减少耦合，独立操作 |
| 一次事务一个聚合 | 一个事务只修改一个聚合 | 保证一致性，减少锁 |
| 最终一致性 | 跨聚合的一致性通过事件异步保证 | 避免分布式事务 |

### 6. 领域事件（Domain Event）

**比喻**：领域事件像广播——"订单已创建！"这个消息发出后，库存、物流、通知等所有关心的人都能听到并各自行动。

```
领域事件的核心特征：
  ├── 表示业务中已经发生的事情
  ├── 用过去时命名（OrderPlaced / PaymentReceived）
  ├── 触发其他上下文的操作
  └── 实现跨聚合/跨上下文的最终一致性

例：电商下单流程中的领域事件

[订单上下文]                     [库存上下文]
  PlaceOrder()                     │
    │                              │
    ▼ 发布事件                      │
  OrderPlaced ─────────────────→ ReserveStock()
    │                              │
    │                  [支付上下文]  │
    │                      ▲       │
    │                      │       │
    └──→ RequestPayment() ─┘       │
           │                        │
           ▼ 发布事件                │
         PaymentReceived ──────────→ ConfirmShipment()
                                    │
                           [物流上下文]

关键：各上下文通过事件松耦合
  └── 订单不需要知道库存怎么处理
  └── 库存不需要知道支付怎么处理
  └── 每个上下文只关心自己的事件
```

### 7. 仓储（Repository）

**比喻**：仓储像仓库管理员——你不需要知道货物在仓库的哪个货架，只需要告诉管理员"给我订单123"，他就给你拿来。

```
仓储的核心职责：
  ├── 隐藏数据存储细节
  ├── 提供类似集合的接口
  └── 只为聚合根提供仓储

仓储接口（领域层定义）：

  class OrderRepository {
  public:
      virtual Order findById(const string& id) = 0;
      virtual void save(const Order& order) = 0;
      virtual void remove(const string& id) = 0;
  };

仓储实现（基础设施层）：

  class MySQLOrderRepository : public OrderRepository {
  public:
      Order findById(const string& id) override {
          // SQL查询、结果映射 → 返回Order对象
          // 领域层不需要知道用的是MySQL还是MongoDB
      }
      void save(const Order& order) override {
          // INSERT or UPDATE
      }
  };

关键：领域层只定义接口，不依赖具体存储技术
  └── 换数据库？只换实现，不换接口
  └── 测试？用内存实现的Mock Repository
```

---

## 3. DDD的分层架构 — 四层洋葱

### 1. 四层结构

```
DDD的分层架构（从内到外）：

┌──────────────────────────────────────────┐
│           用户界面/展示层（UI）            │  ← 最外层
│     Controller / REST API / CLI          │
├──────────────────────────────────────────┤
│           应用层（Application）           │
│     用例编排 / 事务管理 / 权限检查         │
├──────────────────────────────────────────┤
│           领域层（Domain）                │  ← 核心
│     实体 / 值对象 / 聚合 / 领域事件        │
│     仓储接口 / 领域服务                    │
├──────────────────────────────────────────┤
│           基础设施层（Infrastructure）     │
│     数据库 / 消息队列 / 外部API / 文件系统  │
└──────────────────────────────────────────┘

依赖规则（关键！）：
  └── 外层可以依赖内层
  └── 内层不能依赖外层
  └── 领域层不依赖任何外层（最纯粹）

  UI → 应用层 → 领域层 ← 基础设施层
                      ↑
           基础设施实现领域层定义的接口
```

### 2. 各层职责

| 层 | 职责 | 不做什么 | 比喻 |
|---|------|---------|------|
| **用户界面层** | 接收用户输入、展示结果 | 不含业务逻辑 | 餐厅服务员 |
| **应用层** | 编排用例、协调领域对象、事务 | 不含业务规则 | 餐厅经理 |
| **领域层** | 业务规则、领域模型 | 不关心存储和展示 | 厨师 |
| **基础设施层** | 技术实现（数据库/MQ/外部API） | 不含业务逻辑 | 食材供应商 |

### 3. C++代码示例

```cpp
// ========== 领域层（Domain）==========
// 不依赖任何外层

class Money {
    const long amount_;
    const string currency_;
public:
    Money(long amount, string currency)
        : amount_(amount), currency_(move(currency)) {}

    bool equals(const Money& other) const {
        return amount_ == other.amount_ && currency_ == other.currency_;
    }

    Money add(const Money& other) const {
        return Money(amount_ + other.amount_, currency_);
    }

    bool is_negative() const { return amount_ < 0; }
    long amount() const { return amount_; }
};

enum class OrderStatus { PLACED, PAID, SHIPPED, CANCELLED };

class OrderItem {
    string product_id_;
    int quantity_;
    Money price_;
public:
    OrderItem(string pid, int qty, Money price)
        : product_id_(move(pid)), quantity_(qty), price_(price) {}

    Money subtotal() const {
        return Money(price_.amount() * quantity_, "CNY");
    }

    const string& product_id() const { return product_id_; }
    int quantity() const { return quantity_; }
};

class Order {
    string order_id_;
    string customer_id_;
    vector<OrderItem> items_;
    OrderStatus status_ = OrderStatus::PLACED;
public:
    Order(string id, string customer)
        : order_id_(move(id)), customer_id_(move(customer)) {}

    void addItem(string product_id, int quantity, Money price) {
        if (status_ != OrderStatus::PLACED)
            throw runtime_error("Cannot add item to non-placed order");
        items_.emplace_back(move(product_id), quantity, move(price));
    }

    void cancel() {
        if (status_ == OrderStatus::SHIPPED)
            throw runtime_error("Cannot cancel shipped order");
        status_ = OrderStatus::CANCELLED;
    }

    void markAsPaid() {
        if (status_ != OrderStatus::PLACED)
            throw runtime_error("Only placed order can be paid");
        status_ = OrderStatus::PAID;
    }

    Money totalAmount() const {
        long total = 0;
        for (const auto& item : items_)
            total += item.subtotal().amount();
        return Money(total, "CNY");
    }

    const string& id() const { return order_id_; }
    OrderStatus status() const { return status_; }
};

struct OrderPlacedEvent {
    string order_id;
    string customer_id;
};

class OrderRepository {
public:
    virtual ~OrderRepository() = default;
    virtual Order findById(const string& id) = 0;
    virtual void save(const Order& order) = 0;
};

// ========== 应用层（Application）==========
// 编排领域对象，不含业务规则

class OrderApplicationService {
    OrderRepository& repo_;
public:
    OrderApplicationService(OrderRepository& repo) : repo_(repo) {}

    string placeOrder(const string& customer_id,
                      const vector<tuple<string,int,Money>>& items) {
        string order_id = generateId();
        Order order(order_id, customer_id);
        for (const auto& [pid, qty, price] : items)
            order.addItem(pid, qty, price);
        repo_.save(order);
        // 发布领域事件 OrderPlacedEvent
        return order_id;
    }

    void cancelOrder(const string& order_id) {
        Order order = repo_.findById(order_id);
        order.cancel();
        repo_.save(order);
    }
};

// ========== 基础设施层（Infrastructure）==========
// 实现领域层定义的接口

class MySQLOrderRepository : public OrderRepository {
    Connection& db_;
public:
    MySQLOrderRepository(Connection& db) : db_(db) {}

    Order findById(const string& id) override {
        auto row = db_.query("SELECT * FROM orders WHERE id = ?", id);
        Order order(row.get("id"), row.get("customer_id"));
        // ... 加载OrderItem等
        return order;
    }

    void save(const Order& order) override {
        db_.execute("INSERT INTO orders ...", order.id(), ...);
    }
};

// ========== 用户界面层（UI）==========
// 接收HTTP请求，调用应用层

void handlePlaceOrder(HttpRequest& req) {
    auto service = OrderApplicationService(repo);
    string order_id = service.placeOrder(req.get("customer"), items);
    req.respond(201, {{"order_id", order_id}});
}
```

---

## 4. 战略设计 — 看清业务全貌

### 1. 战略设计 vs 战术设计

```
DDD的两个层面：

战略设计（Strategic Design）— 看清大局
  └── 回答：系统有哪些业务领域？边界在哪？
  └── 工具：限界上下文、上下文映射、统一语言
  └── 关注：业务边界、团队边界、系统集成

战术设计（Tactical Design）— 填充细节
  └── 回答：一个上下文内部怎么建模？
  └── 工具：实体、值对象、聚合、领域事件、仓储
  └── 关注：代码结构、对象关系、业务规则

类比：
  战略设计 = 城市规划（哪里是商业区、住宅区、工业区）
  战术设计 = 建筑设计（每栋楼内部怎么设计）
```

### 2. 上下文映射（Context Map）

**比喻**：上下文映射像城市地图——标注各个区域（限界上下文）之间的关系和交通方式（集成模式）。

```
电商系统的上下文映射：

┌──────────────┐     ┌──────────────┐
│   商品目录    │────→│    订单      │
│   上下文      │     │    上下文    │
└──────────────┘     └──────┬───────┘
       ↑                     │
       │                     ↓
┌──────────────┐     ┌──────────────┐
│    搜索      │     │    支付      │
│    上下文    │     │    上下文    │
└──────────────┘     └──────────────┘
                            │
                            ↓
                     ┌──────────────┐
                     │    物流      │
                     │    上下文    │
                     └──────────────┘

上下文之间的关系模式：

1. 合作关系（Partnership）
   └── 两个上下文紧密协作，同步协调
   └── 例：订单↔支付（下单后必须支付）

2. 客户-供应商（Customer-Supplier）
   └── 下游（客户）依赖上游（供应商）提供的数据
   └── 例：订单（客户）← 商品目录（供应商）

3. 遵奉者（Conformist）
   └── 下游完全遵循上游的模型
   └── 例：搜索上下文完全遵循商品目录的数据格式

4. 防腐层（Anti-Corruption Layer，ACL）
   └── 下游通过翻译层隔离上游的影响
   └── 例：订单上下文通过ACL对接外部支付系统
   └── 外部系统的变化不会"腐蚀"订单模型

5. 开放主机服务（Open Host Service）
   └── 上游提供标准化的API给所有下游
   └── 例：商品目录提供REST API给所有需要商品数据的上下文

6. 发布语言（Published Language）
   └── 上下文之间使用标准化的交换格式
   └── 例：JSON Schema / Protocol Buffers
```

### 3. 防腐层详解 — 保护你的领域不被"腐蚀"

```
为什么需要防腐层？

场景：你的订单系统需要对接一个老旧的ERP系统
  └── ERP的API设计混乱：字段名是德文、状态码是数字、日期格式奇怪
  └── 如果直接在订单领域中使用ERP的模型 → 你的领域被"腐蚀"了

没有防腐层：
  [订单领域] ─── 直接调用 ──→ [老旧ERP API]
     │
     └── 订单代码里充满了ERP的奇怪字段名和状态码
     └── 领域模型被污染

有防腐层：
  [订单领域] ───→ [防腐层] ───→ [老旧ERP API]
                     │
                     └── 翻译：ERP的奇怪格式 → 领域的统一语言
                     └── ERP改了？只改防腐层，领域不受影响

代码示例：

  // ERP的奇怪响应
  struct ErpOrderResponse {
      string AUFNR;      // 德文：订单号
      int STAT;          // 0=新建 1=处理中 3=完成
      string DATUM;      // DD.MM.YYYY格式
  };

  // 防腐层：翻译成领域语言
  class ErpOrderAdapter {
  public:
      Order translate(const ErpOrderResponse& erp) {
          Order order;
          order.setId(erp.AUFNR);
          order.setStatus(translateStatus(erp.STAT));
          order.setDate(parseDate(erp.DATUM));
          return order;
      }

  private:
      OrderStatus translateStatus(int stat) {
          switch (stat) {
              case 0: return OrderStatus::PLACED;
              case 1: return OrderStatus::PROCESSING;
              case 3: return OrderStatus::COMPLETED;
              default: throw runtime_error("Unknown status");
          }
      }
  };
```

---

## 5. 领域服务 — 不属于任何实体的业务逻辑

### 1. 什么是领域服务

```
有些业务逻辑不属于任何实体或值对象：
  └── 它涉及多个聚合的协调
  └── 它是一个业务操作，不是实体的行为
  └── 这时用领域服务来承载

例：转账
  └── 涉及两个账户（两个聚合）
  └── 不能放在Account实体中（因为要操作另一个Account）
  └── 用领域服务：TransferService.transfer(from, to, amount)

领域服务的特征：
  ├── 无状态（不保存数据）
  ├── 表达业务操作（不是CRUD）
  ├── 只在领域层
  └── 用动词命名（TransferFunds / CalculateDiscount）
```

### 2. 领域服务 vs 应用服务

```
容易混淆的两个概念：

领域服务（Domain Service）
  └── 位置：领域层
  └── 职责：业务规则和业务逻辑
  └── 特征：表达核心业务操作
  └── 例：TransferService.transfer() — 转账规则（余额不能为负）

应用服务（Application Service）
  └── 位置：应用层
  └── 职责：编排用例流程
  └── 特征：不包含业务规则，只协调
  └── 例：TransferAppService.execute() — 调用转账→发通知→记日志

区别：
  领域服务 = 厨师做菜（核心技能）
  应用服务 = 经理安排（协调流程）
```

---

## 6. DDD与架构的关系

### 1. DDD不是架构，但指导架构

```
DDD vs 架构：

DDD ─── 方法论（怎么思考业务和建模）
  └── 回答：系统有哪些领域？边界在哪？模型怎么建？

架构 ─── 技术方案（怎么组织代码和部署）
  └── 回答：代码怎么分层？服务怎么部署？数据怎么存储？

关系：DDD指导架构决策
  └── 限界上下文 → 可以映射为微服务
  └── 聚合 → 可以映射为服务内的模块
  └── 领域事件 → 可以映射为消息队列通信
```

### 2. DDD与微服务

```
DDD与微服务的天然契合：

限界上下文 ≈ 微服务边界
  └── 一个限界上下文 → 一个微服务
  └── 上下文映射 → 服务间通信

聚合 ≈ 微服务内的模块
  └── 一个聚合 → 一个内聚的模块
  └── 聚合根 → 模块的入口

领域事件 ≈ 服务间异步通信
  └── 事件驱动 → 消息队列
  └── 最终一致性 → 不需要分布式事务

但注意：
  └── 不是所有限界上下文都要变成微服务
  └── 小项目用模块化单体 + DDD就够了
  └── 先做好领域建模，再决定技术架构
```

### 3. DDD与六边形架构

```
DDD和六边形架构（端口与适配器）是最佳拍档：

         ┌─ [REST API适配器]
         │
[DB适配器]┤─ [领域核心] ├─ [消息队列适配器]
         │
         └─ [测试适配器]

领域核心 = DDD的领域层
  └── 实体、值对象、聚合、领域事件、领域服务、仓储接口

端口 = 领域层定义的接口
  └── 仓储接口（Repository Interface）
  └── 领域事件发布接口

适配器 = 基础设施层的实现
  └── MySQL仓储实现
  └── Kafka事件发布实现
  └── REST Controller

好处：领域核心完全独立，不依赖任何技术
```

---

## 7. DDD的适用场景 — 不是所有项目都适合

### 1. 适合DDD的场景

```
✅ 适合DDD：

1. 业务复杂
   └── 业务规则多、变化频繁
   └── 例：金融交易、保险理赔、医疗系统

2. 领域知识密集
   └── 需要和业务专家深度协作
   └── 例：电商、物流、供应链

3. 长期演进
   └── 系统会持续迭代多年
   └── 例：企业核心业务系统

4. 团队较大
   └── 多团队协作，需要清晰边界
   └── 例：大型互联网公司
```

### 2. 不适合DDD的场景

```
❌ 不适合DDD：

1. 简单CRUD
   └── 只是增删改查，没有复杂业务规则
   └── 例：简单的后台管理系统

2. 技术驱动型项目
   └── 核心是技术挑战，不是业务复杂度
   └── 例：编译器、数据库引擎、游戏引擎

3. 原型/MVP
   └── 快速验证想法，不需要精心设计
   └── 例：创业初期的产品原型

4. 数据分析/批处理
   └── 关注数据处理流程，不是业务建模
   └── 例：ETL管道、报表系统

5. C/C++系统编程
   └── 嵌入式、驱动、操作系统等底层开发
   └── 业务领域就是"技术"本身，DDD帮助有限
```

### 3. 决策树

```
你的项目适合DDD吗？

业务规则复杂吗？
├── 不复杂（简单CRUD）
│   └── ❌ 不需要DDD，用分层架构就够了
│
└── 复杂
    ├── 会长期演进吗？
    │   ├── 不会（一次性项目）
    │   │   └── ❌ DDD投入产出比不高
    │   │
    │   └── 会
    │       ├── 有业务专家可以协作吗？
    │       │   ├── 没有
    │       │   │   └── ⚠️ 先建立业务沟通渠道
    │       │   │
    │       │   └── 有
    │       │       └── ✅ 适合DDD
    │       │
    │       └── 团队多大？
    │           ├── 1-3人
    │           │   └── ⚠️ 可以用DDD思想，但不必全套
    │           │
    │           └── 4+人
    │               └── ✅ 适合DDD
```

---

## 8. DDD实战步骤 — 从0到1

### 1. DDD项目启动流程

```
Step 1：事件风暴（Event Storming）
  └── 全团队（业务+开发）一起贴便签
  └── 梳理业务流程中的关键事件
  └── 例：订单已创建 → 支付已完成 → 库存已预留 → 商品已发货

Step 2：识别限界上下文
  └── 根据事件风暴结果，划分业务边界
  └── 同一个业务流程中的事件属于同一个上下文

Step 3：建立统一语言
  └── 每个上下文定义自己的术语表
  └── 确保所有人理解一致

Step 4：领域建模
  └── 在每个上下文内建立领域模型
  └── 识别实体、值对象、聚合

Step 5：定义上下文映射
  └── 明确上下文之间的关系
  └── 确定集成方式（同步/异步、防腐层等）

Step 6：代码实现
  └── 按分层架构实现
  └── 领域层 → 应用层 → 基础设施层 → UI层
```

### 2. 事件风暴示例

```
电商系统的事件风暴结果（橙色=事件，蓝色=命令，黄色=外部系统）：

[用户] ── 下单 ──→ [订单已创建] ── 预留库存 ──→ [库存已预留]
                        │                              │
                   请求支付                            确认发货
                        │                              │
                        ▼                              ▼
                   [支付已完成] ──────────────→ [商品已发货]
                        │                              │
                    发送通知                         物流追踪
                        │                              │
                        ▼                              ▼
                   [通知已发送]                  [商品已签收]

从事件中识别限界上下文：
  ├── 订单上下文：订单已创建、订单已取消
  ├── 支付上下文：支付已完成、支付已退款
  ├── 库存上下文：库存已预留、库存已释放
  └── 物流上下文：商品已发货、商品已签收
```

---

## 9. DDD概念速查表

| 概念 | 一句话定义 | 比喻 |
|------|-----------|------|
| 统一语言 | 全团队使用同一套业务术语 | 公司的官方语言 |
| 限界上下文 | 业务模型的边界，同一术语在不同上下文有不同含义 | 公司的部门 |
| 实体 | 有唯一标识的对象，属性可变 | 有身份证的人 |
| 值对象 | 没有标识、通过值判断相等的不可变对象 | 钞票 |
| 聚合 | 一致性边界，外部只通过聚合根访问 | 原子（核+电子） |
| 聚合根 | 聚合的入口和一致性守护者 | 原子核 |
| 领域事件 | 表示业务中已发生的事实 | 广播通知 |
| 仓储 | 隐藏存储细节的集合式接口 | 仓库管理员 |
| 领域服务 | 不属于任何实体的业务操作 | 厨师做菜 |
| 应用服务 | 编排用例流程，不含业务规则 | 餐厅经理 |
| 防腐层 | 隔离外部系统影响的翻译层 | 翻译官 |
| 上下文映射 | 限界上下文之间的关系图 | 城市地图 |

***

### 1. 相关章节

- [框架引擎中间件与架构概念指南](15-框架引擎中间件与架构.md) — 框架/引擎/中间件/前后端/10种架构模式
- [编程范式实战指南](13-编程范式概览与过程式编程.md) — OOP范式与DDD充血模型的关系
- [设计原则SOLID](../../04-工程实践/02-设计原则SOLID.md) — DDD遵循SOLID原则
- [设计模式](../../04-工程实践/03-设计模式.md) — DDD中常用的工厂/策略/观察者模式
- [项目理解分析与工程化编码指南](12-项目理解分析与工程化编码.md) — 创建项目5步法、工程化重构
- [编程工程师与成长之道](../../01-C语言/导学/01-编程工程师与成长之道.md) — 开发本质、技术四层面

***

### 相关阅读

- [框架引擎中间件与架构](15-框架引擎中间件与架构.md)
- [依赖注入与控制反转](18-依赖注入与控制反转.md)
- [API-SDK-协议与接口](17-API-SDK-协议与接口.md)