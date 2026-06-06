#include "cu/hash_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* to_hex(const unsigned char* data, size_t len) {
    char* out = (char*)malloc(len * 2 + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; i++) {
        sprintf(out + i * 2, "%02x", data[i]);
    }
    out[len * 2] = '\0';
    return out;
}

typedef struct {
    unsigned int state[4];
    unsigned int count[2];
    unsigned char buffer[64];
} MD5_CTX;

#define F(x,y,z) (((x)&(y))|((~x)&(z)))
#define G(x,y,z) (((x)&(z))|((y)&(~z)))
#define H(x,y,z) ((x)^(y)^(z))
#define I(x,y,z) ((y)^((x)|(~z)))

#define ROTL(x,n) (((x)<<(n))|((x)>>(32-(n))))

#define FF(a,b,c,d,x,s,ac) { (a) += F((b),(c),(d)) + (x) + (unsigned int)(ac); (a) = ROTL((a),(s)); (a) += (b); }
#define GG(a,b,c,d,x,s,ac) { (a) += G((b),(c),(d)) + (x) + (unsigned int)(ac); (a) = ROTL((a),(s)); (a) += (b); }
#define HH(a,b,c,d,x,s,ac) { (a) += H((b),(c),(d)) + (x) + (unsigned int)(ac); (a) = ROTL((a),(s)); (a) += (b); }
#define II(a,b,c,d,x,s,ac) { (a) += I((b),(c),(d)) + (x) + (unsigned int)(ac); (a) = ROTL((a),(s)); (a) += (b); }

static void md5_transform(unsigned int state[4], const unsigned char block[64]) {
    unsigned int a = state[0], b = state[1], c = state[2], d = state[3];
    unsigned int x[16];

    for (int i = 0, j = 0; i < 16; i++, j += 4) {
        x[i] = ((unsigned int)block[j]) | ((unsigned int)block[j+1] << 8) |
               ((unsigned int)block[j+2] << 16) | ((unsigned int)block[j+3] << 24);
    }

    FF(a,b,c,d,x[ 0], 7,0xd76aa478); FF(d,a,b,c,x[ 1],12,0xe8c7b756);
    FF(c,d,a,b,x[ 2],17,0x242070db); FF(b,c,d,a,x[ 3],22,0xc1bdceee);
    FF(a,b,c,d,x[ 4], 7,0xf57c0faf); FF(d,a,b,c,x[ 5],12,0x4787c62a);
    FF(c,d,a,b,x[ 6],17,0xa8304613); FF(b,c,d,a,x[ 7],22,0xfd469501);
    FF(a,b,c,d,x[ 8], 7,0x698098d8); FF(d,a,b,c,x[ 9],12,0x8b44f7af);
    FF(c,d,a,b,x[10],17,0xffff5bb1); FF(b,c,d,a,x[11],22,0x895cd7be);
    FF(a,b,c,d,x[12], 7,0x6b901122); FF(d,a,b,c,x[13],12,0xfd987193);
    FF(c,d,a,b,x[14],17,0xa679438e); FF(b,c,d,a,x[15],22,0x49b40821);

    GG(a,b,c,d,x[ 1], 5,0xf61e2562); GG(d,a,b,c,x[ 6], 9,0xc040b340);
    GG(c,d,a,b,x[11],14,0x265e5a51); GG(b,c,d,a,x[ 0],20,0xe9b6c7aa);
    GG(a,b,c,d,x[ 5], 5,0xd62f105d); GG(d,a,b,c,x[10], 9,0x02441453);
    GG(c,d,a,b,x[15],14,0xd8a1e681); GG(b,c,d,a,x[ 4],20,0xe7d3fbc8);
    GG(a,b,c,d,x[ 9], 5,0x21e1cde6); GG(d,a,b,c,x[14], 9,0xc33707d6);
    GG(c,d,a,b,x[ 3],14,0xf4d50d87); GG(b,c,d,a,x[ 8],20,0x455a14ed);
    GG(a,b,c,d,x[13], 5,0xa9e3e905); GG(d,a,b,c,x[ 2], 9,0xfcefa3f8);
    GG(c,d,a,b,x[ 7],14,0x676f02d9); GG(b,c,d,a,x[12],20,0x8d2a4c8a);

    HH(a,b,c,d,x[ 5], 4,0xfffa3942); HH(d,a,b,c,x[ 8],11,0x8771f681);
    HH(c,d,a,b,x[11],16,0x6d9d6122); HH(b,c,d,a,x[14],23,0xfde5380c);
    HH(a,b,c,d,x[ 1], 4,0xa4beea44); HH(d,a,b,c,x[ 4],11,0x4bdecfa9);
    HH(c,d,a,b,x[ 7],16,0xf6bb4b60); HH(b,c,d,a,x[10],23,0xbebfbc70);
    HH(a,b,c,d,x[13], 4,0x289b7ec6); HH(d,a,b,c,x[ 0],11,0xeaa127fa);
    HH(c,d,a,b,x[ 3],16,0xd4ef3085); HH(b,c,d,a,x[ 6],23,0x04881d05);
    HH(a,b,c,d,x[ 9], 4,0xd9d4d039); HH(d,a,b,c,x[12],11,0xe6db99e5);
    HH(c,d,a,b,x[15],16,0x1fa27cf8); HH(b,c,d,a,x[ 2],23,0xc4ac5665);

    II(a,b,c,d,x[ 0], 6,0xf4292244); II(d,a,b,c,x[ 7],10,0x432aff97);
    II(c,d,a,b,x[14],15,0xab9423a7); II(b,c,d,a,x[ 5],21,0xfc93a039);
    II(a,b,c,d,x[12], 6,0x655b59c3); II(d,a,b,c,x[ 3],10,0x8f0ccc92);
    II(c,d,a,b,x[10],15,0xffeff47d); II(b,c,d,a,x[ 1],21,0x85845dd1);
    II(a,b,c,d,x[ 8], 6,0x6fa87e4f); II(d,a,b,c,x[15],10,0xfe2ce6e0);
    II(c,d,a,b,x[ 6],15,0xa3014314); II(b,c,d,a,x[13],21,0x4e0811a1);
    II(a,b,c,d,x[ 4], 6,0xf7537e82); II(d,a,b,c,x[11],10,0xbd3af235);
    II(c,d,a,b,x[ 2],15,0x2ad7d2bb); II(b,c,d,a,x[ 9],21,0xeb86d391);

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
}

static void md5_init(MD5_CTX* ctx) {
    ctx->count[0] = 0;
    ctx->count[1] = 0;
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xefcdab89;
    ctx->state[2] = 0x98badcfe;
    ctx->state[3] = 0x10325476;
}

static void md5_update(MD5_CTX* ctx, const unsigned char* data, size_t len) {
    size_t index = (size_t)(ctx->count[0] >> 3) & 0x3F;
    size_t i = 0;
    unsigned int old_count = ctx->count[0];
    ctx->count[0] += (unsigned int)(len << 3);
    if (ctx->count[0] < old_count) ctx->count[1]++;
    ctx->count[1] += (unsigned int)(len >> 29);

    size_t part_len = 64 - index;
    if (len >= part_len) {
        memcpy(ctx->buffer + index, data, part_len);
        md5_transform(ctx->state, ctx->buffer);
        for (i = part_len; i + 64 <= len; i += 64) {
            md5_transform(ctx->state, data + i);
        }
        index = 0;
    }
    memcpy(ctx->buffer + index, data + i, len - i);
}

static void md5_final(unsigned char digest[16], MD5_CTX* ctx) {
    unsigned char padding[64];
    memset(padding, 0, 64);
    padding[0] = 0x80;

    size_t index = (size_t)(ctx->count[0] >> 3) & 0x3F;
    size_t pad_len = (index < 56) ? (56 - index) : (120 - index);
    md5_update(ctx, padding, pad_len);

    unsigned char bits[8];
    for (int i = 0; i < 4; i++) {
        bits[i] = (unsigned char)(ctx->count[0] >> (i * 8));
        bits[i + 4] = (unsigned char)(ctx->count[1] >> (i * 8));
    }
    md5_update(ctx, bits, 8);

    for (int i = 0; i < 4; i++) {
        digest[i * 4 + 0] = (unsigned char)(ctx->state[i] & 0xff);
        digest[i * 4 + 1] = (unsigned char)((ctx->state[i] >> 8) & 0xff);
        digest[i * 4 + 2] = (unsigned char)((ctx->state[i] >> 16) & 0xff);
        digest[i * 4 + 3] = (unsigned char)((ctx->state[i] >> 24) & 0xff);
    }
}

char* md5_hash(const char* data, size_t data_len) {
    if (!data) return NULL;
    MD5_CTX ctx;
    md5_init(&ctx);
    md5_update(&ctx, (const unsigned char*)data, data_len);
    unsigned char digest[16];
    md5_final(digest, &ctx);
    return to_hex(digest, 16);
}

typedef struct {
    unsigned int state[8];
    unsigned char buffer[64];
    unsigned long long bitlen;
    unsigned int datalen;
} SHA256_CTX;

#define ROTR(x,n) (((x)>>(n))|((x)<<(32-(n))))
#define CH(x,y,z) (((x)&(y))^((~(x))&(z)))
#define MAJ(x,y,z) (((x)&(y))^((x)&(z))^((y)&(z)))
#define EP0(x) (ROTR(x,2)^ROTR(x,13)^ROTR(x,22))
#define EP1(x) (ROTR(x,6)^ROTR(x,11)^ROTR(x,25))
#define SIG0(x) (ROTR(x,7)^ROTR(x,18)^((x)>>3))
#define SIG1(x) (ROTR(x,17)^ROTR(x,19)^((x)>>10))

static const unsigned int sha256_k[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void sha256_transform(SHA256_CTX* ctx, const unsigned char* data) {
    unsigned int w[64];
    for (int i = 0; i < 16; i++) {
        w[i] = ((unsigned int)data[i*4] << 24) | ((unsigned int)data[i*4+1] << 16) |
               ((unsigned int)data[i*4+2] << 8) | ((unsigned int)data[i*4+3]);
    }
    for (int i = 16; i < 64; i++) {
        w[i] = SIG1(w[i-2]) + w[i-7] + SIG0(w[i-15]) + w[i-16];
    }

    unsigned int a = ctx->state[0], b = ctx->state[1], c = ctx->state[2], d = ctx->state[3];
    unsigned int e = ctx->state[4], f = ctx->state[5], g = ctx->state[6], h = ctx->state[7];

    for (int i = 0; i < 64; i++) {
        unsigned int t1 = h + EP1(e) + CH(e,f,g) + sha256_k[i] + w[i];
        unsigned int t2 = EP0(a) + MAJ(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX* ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372; ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
}

static void sha256_update(SHA256_CTX* ctx, const unsigned char* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        ctx->buffer[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->buffer);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(unsigned char digest[32], SHA256_CTX* ctx) {
    unsigned int i = ctx->datalen;
    if (ctx->datalen < 56) {
        ctx->buffer[i++] = 0x80;
        while (i < 56) ctx->buffer[i++] = 0x00;
    } else {
        ctx->buffer[i++] = 0x80;
        while (i < 64) ctx->buffer[i++] = 0x00;
        sha256_transform(ctx, ctx->buffer);
        memset(ctx->buffer, 0, 56);
    }

    ctx->bitlen += ctx->datalen * 8;
    ctx->buffer[63] = (unsigned char)(ctx->bitlen & 0xff);
    ctx->buffer[62] = (unsigned char)((ctx->bitlen >> 8) & 0xff);
    ctx->buffer[61] = (unsigned char)((ctx->bitlen >> 16) & 0xff);
    ctx->buffer[60] = (unsigned char)((ctx->bitlen >> 24) & 0xff);
    ctx->buffer[59] = (unsigned char)((ctx->bitlen >> 32) & 0xff);
    ctx->buffer[58] = (unsigned char)((ctx->bitlen >> 40) & 0xff);
    ctx->buffer[57] = (unsigned char)((ctx->bitlen >> 48) & 0xff);
    ctx->buffer[56] = (unsigned char)((ctx->bitlen >> 56) & 0xff);
    sha256_transform(ctx, ctx->buffer);

    for (int j = 0; j < 8; j++) {
        digest[j*4+0] = (unsigned char)((ctx->state[j] >> 24) & 0xff);
        digest[j*4+1] = (unsigned char)((ctx->state[j] >> 16) & 0xff);
        digest[j*4+2] = (unsigned char)((ctx->state[j] >> 8) & 0xff);
        digest[j*4+3] = (unsigned char)(ctx->state[j] & 0xff);
    }
}

char* sha256_hash(const char* data, size_t data_len) {
    if (!data) return NULL;
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (const unsigned char*)data, data_len);
    unsigned char digest[32];
    sha256_final(digest, &ctx);
    return to_hex(digest, 32);
}

char* file_md5(const char* path) {
    if (!path) return NULL;
    FILE* fp = fopen(path, "rb");
    if (!fp) return NULL;

    MD5_CTX ctx;
    md5_init(&ctx);

    unsigned char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
        md5_update(&ctx, buf, n);
    }
    fclose(fp);

    unsigned char digest[16];
    md5_final(digest, &ctx);
    return to_hex(digest, 16);
}

unsigned int crc32_hash(const char* data, size_t data_len) {
    if (!data) return 0;

    static unsigned int table[256];
    static int table_init = 0;
    if (!table_init) {
        for (unsigned int i = 0; i < 256; i++) {
            unsigned int crc = i;
            for (int j = 0; j < 8; j++) {
                if (crc & 1) {
                    crc = (crc >> 1) ^ 0xEDB88320;
                } else {
                    crc >>= 1;
                }
            }
            table[i] = crc;
        }
        table_init = 1;
    }

    unsigned int crc = 0xFFFFFFFF;
    for (size_t i = 0; i < data_len; i++) {
        crc = (crc >> 8) ^ table[(crc ^ (unsigned char)data[i]) & 0xFF];
    }
    return crc ^ 0xFFFFFFFF;
}
