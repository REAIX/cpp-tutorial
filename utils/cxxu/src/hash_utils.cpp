#include "cu_utils/hash_utils.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>

namespace cu {

static std::string toHexLower(const uint8_t* data, size_t len) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        oss << std::setw(2) << static_cast<int>(data[i]);
    }
    return oss.str();
}

static uint32_t md5LeftRotate(uint32_t x, int c) {
    return (x << c) | (x >> (32 - c));
}

std::string HashUtils::md5(const std::string& data) {
    static const uint32_t T[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
    };

    static const int s[64] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
    };

    uint32_t a0 = 0x67452301;
    uint32_t b0 = 0xefcdab89;
    uint32_t c0 = 0x98badcfe;
    uint32_t d0 = 0x10325476;

    size_t origLen = data.size();
    std::vector<uint8_t> msg(data.begin(), data.end());

    msg.push_back(0x80);
    while (msg.size() % 64 != 56) {
        msg.push_back(0x00);
    }

    uint64_t bitLen = static_cast<uint64_t>(origLen) * 8;
    for (int i = 0; i < 8; ++i) {
        msg.push_back(static_cast<uint8_t>((bitLen >> (i * 8)) & 0xFF));
    }

    for (size_t offset = 0; offset < msg.size(); offset += 64) {
        uint32_t M[16];
        for (int i = 0; i < 16; ++i) {
            M[i] = static_cast<uint32_t>(msg[offset + i * 4])
                 | (static_cast<uint32_t>(msg[offset + i * 4 + 1]) << 8)
                 | (static_cast<uint32_t>(msg[offset + i * 4 + 2]) << 16)
                 | (static_cast<uint32_t>(msg[offset + i * 4 + 3]) << 24);
        }

        uint32_t A = a0, B = b0, C = c0, D = d0;

        for (int i = 0; i < 64; ++i) {
            uint32_t F;
            uint32_t g;
            if (i < 16) {
                F = (B & C) | (~B & D);
                g = static_cast<uint32_t>(i);
            } else if (i < 32) {
                F = (D & B) | (~D & C);
                g = static_cast<uint32_t>((5 * i + 1) % 16);
            } else if (i < 48) {
                F = B ^ C ^ D;
                g = static_cast<uint32_t>((3 * i + 5) % 16);
            } else {
                F = C ^ (B | ~D);
                g = static_cast<uint32_t>((7 * i) % 16);
            }
            F = F + A + T[i] + M[g];
            A = D;
            D = C;
            C = B;
            B = B + md5LeftRotate(F, s[i]);
        }

        a0 += A;
        b0 += B;
        c0 += C;
        d0 += D;
    }

    uint8_t digest[16];
    for (int i = 0; i < 4; ++i) {
        digest[i]      = static_cast<uint8_t>((a0 >> (i * 8)) & 0xFF);
        digest[i + 4]  = static_cast<uint8_t>((b0 >> (i * 8)) & 0xFF);
        digest[i + 8]  = static_cast<uint8_t>((c0 >> (i * 8)) & 0xFF);
        digest[i + 12] = static_cast<uint8_t>((d0 >> (i * 8)) & 0xFF);
    }

    return toHexLower(digest, 16);
}

std::string HashUtils::sha256(const std::string& data) {
    static const uint32_t K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    uint32_t h0 = 0x6a09e667;
    uint32_t h1 = 0xbb67ae85;
    uint32_t h2 = 0x3c6ef372;
    uint32_t h3 = 0xa54ff53a;
    uint32_t h4 = 0x510e527f;
    uint32_t h5 = 0x9b05688c;
    uint32_t h6 = 0x1f83d9ab;
    uint32_t h7 = 0x5be0cd19;

    size_t origLen = data.size();
    std::vector<uint8_t> msg(data.begin(), data.end());

    msg.push_back(0x80);
    while (msg.size() % 64 != 56) {
        msg.push_back(0x00);
    }

    uint64_t bitLen = static_cast<uint64_t>(origLen) * 8;
    for (int i = 7; i >= 0; --i) {
        msg.push_back(static_cast<uint8_t>((bitLen >> (i * 8)) & 0xFF));
    }

    for (size_t offset = 0; offset < msg.size(); offset += 64) {
        uint32_t W[64];
        for (int i = 0; i < 16; ++i) {
            W[i] = (static_cast<uint32_t>(msg[offset + i * 4]) << 24)
                 | (static_cast<uint32_t>(msg[offset + i * 4 + 1]) << 16)
                 | (static_cast<uint32_t>(msg[offset + i * 4 + 2]) << 8)
                 | static_cast<uint32_t>(msg[offset + i * 4 + 3]);
        }
        for (int i = 16; i < 64; ++i) {
            uint32_t s0 = ((W[i-15] >> 7) | (W[i-15] << 25)) ^ ((W[i-15] >> 18) | (W[i-15] << 14)) ^ (W[i-15] >> 3);
            uint32_t s1 = ((W[i-2] >> 17) | (W[i-2] << 15)) ^ ((W[i-2] >> 19) | (W[i-2] << 13)) ^ (W[i-2] >> 10);
            W[i] = W[i-16] + s0 + W[i-7] + s1;
        }

        uint32_t a = h0, b = h1, c = h2, d = h3;
        uint32_t e = h4, f = h5, g = h6, h = h7;

        for (int i = 0; i < 64; ++i) {
            uint32_t S1 = ((e >> 6) | (e << 26)) ^ ((e >> 11) | (e << 21)) ^ ((e >> 25) | (e << 7));
            uint32_t ch = (e & f) ^ (~e & g);
            uint32_t temp1 = h + S1 + ch + K[i] + W[i];
            uint32_t S0 = ((a >> 2) | (a << 30)) ^ ((a >> 13) | (a << 19)) ^ ((a >> 22) | (a << 10));
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t temp2 = S0 + maj;

            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        h0 += a; h1 += b; h2 += c; h3 += d;
        h4 += e; h5 += f; h6 += g; h7 += h;
    }

    uint8_t digest[32];
    for (int i = 0; i < 4; ++i) {
        digest[i]      = static_cast<uint8_t>((h0 >> (24 - i * 8)) & 0xFF);
        digest[i + 4]  = static_cast<uint8_t>((h1 >> (24 - i * 8)) & 0xFF);
        digest[i + 8]  = static_cast<uint8_t>((h2 >> (24 - i * 8)) & 0xFF);
        digest[i + 12] = static_cast<uint8_t>((h3 >> (24 - i * 8)) & 0xFF);
        digest[i + 16] = static_cast<uint8_t>((h4 >> (24 - i * 8)) & 0xFF);
        digest[i + 20] = static_cast<uint8_t>((h5 >> (24 - i * 8)) & 0xFF);
        digest[i + 24] = static_cast<uint8_t>((h6 >> (24 - i * 8)) & 0xFF);
        digest[i + 28] = static_cast<uint8_t>((h7 >> (24 - i * 8)) & 0xFF);
    }

    return toHexLower(digest, 32);
}

std::string HashUtils::fileMd5(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return "";

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return md5(content);
}

uint32_t HashUtils::crc32(const std::string& data) {
    static uint32_t table[256];
    static bool initialized = false;

    if (!initialized) {
        for (uint32_t i = 0; i < 256; ++i) {
            uint32_t crc = i;
            for (int j = 0; j < 8; ++j) {
                if (crc & 1) {
                    crc = (crc >> 1) ^ 0xEDB88320;
                } else {
                    crc >>= 1;
                }
            }
            table[i] = crc;
        }
        initialized = true;
    }

    uint32_t crc = 0xFFFFFFFF;
    for (unsigned char c : data) {
        crc = (crc >> 8) ^ table[(crc ^ c) & 0xFF];
    }
    return crc ^ 0xFFFFFFFF;
}

}
