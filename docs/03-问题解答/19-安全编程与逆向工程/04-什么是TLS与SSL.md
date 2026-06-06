# 什么是TLS与SSL
> 📖 相关章节：[安全编程概述](../../13-安全编程与逆向工程/00-安全编程概述.md)、[内存安全](../../13-安全编程与逆向工程/01-内存安全与漏洞防御.md)

> **TLS就是互联网的"加密信封"——你寄出的信件被装进加密信封，只有收件人能打开。** 没有TLS，你的密码、银行卡号在网络上是明文传输的，任何中间人都能看到。

***

### 1. 要点直击

**TLS（Transport Layer Security，传输层安全协议）** 是在两个通信应用程序之间提供保密性和数据完整性的协议。它是SSL（Secure Sockets Layer）的继任者——SSL已被废弃，但人们仍习惯说"SSL"。TLS通过握手建立加密通道，确保数据在传输过程中不被窃听或篡改。

***

### 2. 生活类比

| 类比 | 说明 |
|------|------|
| TLS = 加密信封 | 信件内容只有收件人能读 |
| 证书 = 身份证 | 证明服务器是它声称的那个 |
| CA = 公证处 | 为身份证背书的权威机构 |
| 握手 = 接头暗号 | 双方确认身份、协商加密方式 |
| 对称加密 = 同一把钥匙 | 加密和解密用同一把钥匙，速度快 |
| 非对称加密 = 公钥+私钥 | 公钥加密、私钥解密，安全但慢 |

***

### 3. TLS握手流程

#### 3.1 TLS 1.2握手

```
客户端                                    服务器
  │                                         │
  │  1. ClientHello                         │
  │  ├─ 支持的TLS版本                       │
  │  ├─ 支持的密码套件列表                   │
  │  ├─ 客户端随机数(ClientRandom)          │
  │  └─ 支持的压缩方法                      │
  │ ───────────────────────────────────────→ │
  │                                         │
  │  2. ServerHello                         │
  │  ├─ 选择的TLS版本                       │
  │  ├─ 选择的密码套件                      │
  │  ├─ 服务器随机数(ServerRandom)          │
  │  └─ 会话ID                              │
  │ ←─────────────────────────────────────── │
  │                                         │
  │  3. Certificate                         │
  │  └─ 服务器证书链                        │
  │ ←─────────────────────────────────────── │
  │                                         │
  │  4. ServerKeyExchange（可选）            │
  │  └─ DH参数（如果使用DHE/ECDHE）         │
  │ ←─────────────────────────────────────── │
  │                                         │
  │  5. ServerHelloDone                     │
  │ ←─────────────────────────────────────── │
  │                                         │
  │  6. ClientKeyExchange                   │
  │  └─ 预主密钥(用服务器公钥加密)           │
  │ ───────────────────────────────────────→ │
  │                                         │
  │  [双方计算主密钥]                        │
  │  MasterSecret = PRF(PreMasterSecret,     │
  │    "master secret", ClientRandom+ServerRandom) │
  │                                         │
  │  7. ChangeCipherSpec                    │
  │  └─ 切换到加密通信                      │
  │ ───────────────────────────────────────→ │
  │                                         │
  │  8. Finished（加密）                     │
  │  └─ 验证握手完整性                      │
  │ ───────────────────────────────────────→ │
  │                                         │
  │  9. ChangeCipherSpec                    │
  │ ←─────────────────────────────────────── │
  │                                         │
  │  10. Finished（加密）                    │
  │ ←─────────────────────────────────────── │
  │                                         │
  │  ══════ 加密通信开始 ══════              │
```

#### 3.2 TLS 1.3握手（简化为1-RTT）

```
客户端                                    服务器
  │                                         │
  │  1. ClientHello                         │
  │  ├─ 支持的TLS版本(1.3)                  │
  │  ├─ 密码套件列表                        │
  │  ├─ 客户端随机数                        │
  │  └─ 密钥共享(KeyShare) ← 新！          │
  │ ───────────────────────────────────────→ │
  │                                         │
  │  2. ServerHello                         │
  │  ├─ 选择的密码套件                      │
  │  ├─ 服务器随机数                        │
  │  └─ 密钥共享                            │
  │ ←─────────────────────────────────────── │
  │                                         │
  │  3. Certificate + CertificateVerify     │
  │ ←─────────────────────────────────────── │
  │                                         │
  │  4. Finished                            │
  │ ←─────────────────────────────────────── │
  │                                         │
  │  5. Finished                            │
  │ ───────────────────────────────────────→ │
  │                                         │
  │  ══════ 加密通信开始 ══════              │

TLS 1.3改进：
- 握手从2-RTT减少到1-RTT
- 0-RTT恢复（已连接过的服务器可以立即发送数据）
- 移除了不安全的密码套件
- 强制前向保密
```

***

### 4. 证书链验证

#### 4.1 证书链结构

```
证书链验证：

根证书（Root CA）
  │ 签发
  ↓
中间证书（Intermediate CA）
  │ 签发
  ↓
服务器证书（End-Entity Certificate）
  │ 包含
  ├─ 服务器域名
  ├─ 服务器公钥
  ├─ 有效期
  └─ CA的数字签名

验证过程：
1. 检查服务器证书是否由可信CA签发
2. 沿证书链向上验证签名
3. 直到根证书（浏览器/OS内置的可信根证书列表）
4. 检查证书是否过期
5. 检查证书是否被吊销（CRL/OCSP）
6. 检查域名是否匹配
```

#### 4.2 证书验证代码

```cpp
// OpenSSL证书验证示例
#include <openssl/ssl.h>
#include <openssl/err.h>

int verify_callback(int preverify_ok, X509_STORE_CTX* ctx) {
    X509* cert = X509_STORE_CTX_get_current_cert(ctx);
    int depth = X509_STORE_CTX_get_error_depth(ctx);
    int err = X509_STORE_CTX_get_error(ctx);

    if (!preverify_ok) {
        printf("证书验证失败！深度=%d, 错误=%d: %s\n",
               depth, err, X509_verify_cert_error_string(err));
    }

    return preverify_ok;  // 返回0拒绝连接
}

SSL* create_tls_connection(const char* hostname, int port) {
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());

    // 加载可信CA证书
    SSL_CTX_load_verify_locations(ctx, "/etc/ssl/certs/ca-certificates.crt", NULL);

    // 设置验证回调
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);

    // 创建连接
    SSL* ssl = SSL_new(ctx);
    // ... 绑定socket、执行握手

    // 验证主机名
    X509* cert = SSL_get_peer_certificate(ssl);
    // 检查证书中的CN或SAN是否匹配hostname

    return ssl;
}
```

***

### 5. 密码套件

#### 5.1 密码套件组成

```
密码套件命名格式：
TLS_密钥交换_身份认证_对称加密_消息认证

示例：
TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256
│     │     │         │       │    │
│     │     │         │       │    └─ HMAC算法(SHA256)
│     │     │         │       └────── AEAD模式(GCM)
│     │     │         └────────────── 对称加密(AES-128)
│     │     └──────────────────────── 身份认证(RSA)
│     └────────────────────────────── 密钥交换(ECDHE)
└──────────────────────────────────── 协议(TLS)
```

#### 5.2 安全vs不安全的密码套件

```
安全的密码套件（TLS 1.3仅允许这些）：
✅ TLS_AES_128_GCM_SHA256
✅ TLS_AES_256_GCM_SHA384
✅ TLS_CHACHA20_POLY1305_SHA256

安全的密码套件（TLS 1.2推荐）：
✅ TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256
✅ TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384
✅ TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256

不安全的密码套件（应禁用）：
❌ TLS_RSA_WITH_AES_128_CBC_SHA     （无前向保密）
❌ TLS_RSA_WITH_3DES_EDE_CBC_SHA    （3DES不安全）
❌ TLS_RSA_WITH_RC4_128_SHA          （RC4已破解）
❌ TLS_RSA_EXPORT_WITH_RC4_40_MD5    （出口级加密，极弱）
❌ TLS_RSA_WITH_NULL_SHA             （无加密！）
```

***

### 6. TLS 1.2 vs TLS 1.3

| 特性 | TLS 1.2 | TLS 1.3 |
|------|---------|---------|
| 握手延迟 | 2-RTT | 1-RTT |
| 恢复延迟 | 1-RTT | 0-RTT |
| 密码套件数量 | 37+ | 5 |
| 前向保密 | 可选 | 强制 |
| RSA密钥交换 | 允许 | 禁止 |
| CBC模式 | 允许 | 禁止 |
| RC4/3DES | 允许 | 禁止 |
| 压缩 | 允许 | 禁止 |
| 重协商 | 允许 | 禁止 |
| 安全性 | 中等 | 高 |

***

### 7. 常见TLS漏洞

| 漏洞 | 年份 | 影响 | 原理 |
|------|------|------|------|
| BEAST | 2011 | CBC模式IV可预测 | CBC模式的初始化向量可预测 |
| CRIME | 2012 | 压缩侧信道 | TLS压缩泄露Cookie |
| Heartbleed | 2014 | 内存泄露 | OpenSSL心跳扩展越界读取 |
| POODLE | 2014 | SSL 3.0降级 | SSL 3.0的CBC填充漏洞 |
| FREAK | 2015 | 出口级加密降级 | 强制使用弱加密 |
| Logjam | 2015 | DH参数降级 | 512位DH可被破解 |
| DROWN | 2016 | SSLv2跨协议攻击 | SSLv2漏洞影响TLS |
| ROBOT | 2017 | RSA PKCS#1 v1.5攻击 | Bleichenbacher攻击变体 |

```cpp
// Heartbleed漏洞示例（CVE-2014-0160）
// OpenSSL的心跳扩展存在越界读取

// 漏洞代码（简化）：
int dtls1_process_heartbeat(SSL* ssl, unsigned char* p, unsigned int length) {
    unsigned int payload_length = (p[0] << 8) | p[1];  // 攻击者声称的长度
    // 没有验证 payload_length <= 实际数据长度！

    unsigned char* pl = p + 3;  // 心跳数据起始

    // 分配响应缓冲区
    unsigned char* buffer = OPENSSL_malloc(1 + 2 + payload_length);

    // 拷贝数据——如果payload_length > 实际长度，会越界读取！
    memcpy(buffer, pl, payload_length);  // 越界读取！
    // 可能读取到私钥、密码等敏感数据

    return 1;
}

// 修复：验证长度
if (1 + 2 + payload_length > length) {
    return 0;  // 长度不匹配，拒绝
}
```

***

### 8. 安全配置

```
Nginx TLS安全配置示例：

server {
    listen 443 ssl http2;

    # 仅允许TLS 1.2和1.3
    ssl_protocols TLSv1.2 TLSv1.3;

    # 仅允许安全的密码套件
    ssl_ciphers ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:
                ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:
                ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305;

    # 服务器偏好（防止客户端选择弱套件）
    ssl_prefer_server_ciphers on;

    # 证书和密钥
    ssl_certificate     /etc/ssl/certs/server.crt;
    ssl_certificate_key /etc/ssl/private/server.key;

    # HSTS（强制HTTPS）
    add_header Strict-Transport-Security "max-age=31536000; includeSubDomains" always;

    # OCSP装订
    ssl_stapling on;
    ssl_stapling_verify on;
}
```

***

### 9. 常见误区

| 误区 | 事实 |
|------|------|
| SSL和TLS是同一个东西 | SSL已废弃，TLS是继任者 |
| HTTPS就绝对安全 | HTTPS只保证传输安全，不保证服务器安全 |
| 自签名证书和CA证书一样安全 | 自签名证书无法验证身份，易受中间人攻击 |
| TLS 1.3完全兼容1.2 | 1.3移除了很多1.2的特性 |
| 证书越贵越安全 | 加密强度与价格无关，贵的证书验证更严格 |

***

### 10. 总结

| 维度 | 核心要点 |
|------|---------|
| 本质 | 传输层加密协议，SSL的继任者 |
| 握手 | 协商密钥、验证身份、建立加密通道 |
| 证书链 | 根CA→中间CA→服务器证书，逐级验证 |
| TLS 1.3 | 1-RTT握手、强制前向保密、移除弱算法 |
| 安全配置 | 禁用旧版本和弱套件、启用HSTS |

**核心记忆**：TLS = 加密传输 + 身份验证 + 数据完整性。TLS 1.3比1.2更快更安全。证书链验证确保你连接的是真正的服务器。禁用弱密码套件和旧版本是基本安全要求。