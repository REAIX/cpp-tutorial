/**
 * @file 01_example_interface.c
 * @brief 接口与抽象类实现
 * @description 对应文档: 29-C语言面向对象实现-进阶
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void (*open)(void *self, const char *path);
    int (*read)(void *self, char *buffer, int size);
    int (*write)(void *self, const char *buffer, int size);
    void (*close)(void *self);
    const char *(*name)(const void *self);
} StreamVTable;

typedef struct {
    const StreamVTable *vtable;
} Stream;

void stream_open(Stream *s, const char *path) { s->vtable->open(s, path); }
int stream_read(Stream *s, char *buf, int size) { return s->vtable->read(s, buf, size); }
int stream_write(Stream *s, const char *buf, int size) { return s->vtable->write(s, buf, size); }
void stream_close(Stream *s) { s->vtable->close(s); }
const char *stream_name(const Stream *s) { return s->vtable->name(s); }

typedef struct {
    Stream base;
    char path[256];
    char buffer[1024];
    int pos;
    int length;
} MemoryStream;

static void mem_stream_open(void *self, const char *path) {
    MemoryStream *ms = (MemoryStream *)self;
    strncpy(ms->path, path, sizeof(ms->path) - 1);
    ms->pos = 0;
    ms->length = 0;
    printf("  [MemoryStream] 打开: %s\n", path);
}

static int mem_stream_read(void *self, char *buffer, int size) {
    MemoryStream *ms = (MemoryStream *)self;
    int avail = ms->length - ms->pos;
    int to_read = size < avail ? size : avail;
    if (to_read <= 0) return 0;
    memcpy(buffer, ms->buffer + ms->pos, to_read);
    ms->pos += to_read;
    return to_read;
}

static int mem_stream_write(void *self, const char *buffer, int size) {
    MemoryStream *ms = (MemoryStream *)self;
    int avail = (int)sizeof(ms->buffer) - ms->pos;
    int to_write = size < avail ? size : avail;
    if (to_write <= 0) return 0;
    memcpy(ms->buffer + ms->pos, buffer, to_write);
    ms->pos += to_write;
    ms->length = ms->pos > ms->length ? ms->pos : ms->length;
    return to_write;
}

static void mem_stream_close(void *self) {
    MemoryStream *ms = (MemoryStream *)self;
    printf("  [MemoryStream] 关闭: %s\n", ms->path);
}

static const char *mem_stream_name(const void *self) {
    (void)self;
    return "MemoryStream";
}

static const StreamVTable mem_stream_vtable = {
    .open = mem_stream_open,
    .read = mem_stream_read,
    .write = mem_stream_write,
    .close = mem_stream_close,
    .name = mem_stream_name
};

Stream *mem_stream_create(void) {
    MemoryStream *ms = (MemoryStream *)calloc(1, sizeof(MemoryStream));
    if (!ms) return NULL;
    ms->base.vtable = &mem_stream_vtable;
    return (Stream *)ms;
}

typedef struct {
    Stream base;
    char path[256];
    FILE *fp;
} FileStream;

static void file_stream_open(void *self, const char *path) {
    FileStream *fs = (FileStream *)self;
    strncpy(fs->path, path, sizeof(fs->path) - 1);
    fs->fp = fopen(path, "w+");
    printf("  [FileStream] 打开: %s (%s)\n", path, fs->fp ? "成功" : "失败");
}

static int file_stream_read(void *self, char *buffer, int size) {
    FileStream *fs = (FileStream *)self;
    if (!fs->fp) return -1;
    return (int)fread(buffer, 1, size, fs->fp);
}

static int file_stream_write(void *self, const char *buffer, int size) {
    FileStream *fs = (FileStream *)self;
    if (!fs->fp) return -1;
    return (int)fwrite(buffer, 1, size, fs->fp);
}

static void file_stream_close(void *self) {
    FileStream *fs = (FileStream *)self;
    if (fs->fp) {
        fclose(fs->fp);
        fs->fp = NULL;
    }
    printf("  [FileStream] 关闭: %s\n", fs->path);
}

static const char *file_stream_name(const void *self) {
    (void)self;
    return "FileStream";
}

static const StreamVTable file_stream_vtable = {
    .open = file_stream_open,
    .read = file_stream_read,
    .write = file_stream_write,
    .close = file_stream_close,
    .name = file_stream_name
};

Stream *file_stream_create(void) {
    FileStream *fs = (FileStream *)calloc(1, sizeof(FileStream));
    if (!fs) return NULL;
    fs->base.vtable = &file_stream_vtable;
    return (Stream *)fs;
}

void demo_stream_interface(void) {
    printf("\n=== demo_stream_interface ===\n");
    printf("接口: Stream - 统一的流操作接口\n\n");

    Stream *streams[2];
    streams[0] = mem_stream_create();
    streams[1] = file_stream_create();

    const char *test_path = "test_stream_demo.txt";

    for (int i = 0; i < 2; i++) {
        printf("[%s]\n", stream_name(streams[i]));
        stream_open(streams[i], test_path);

        const char *data = "Hello, Stream Interface!";
        int written = stream_write(streams[i], data, (int)strlen(data));
        printf("  写入 %d 字节\n", written);

        stream_close(streams[i]);
        printf("\n");
    }

    for (int i = 0; i < 2; i++) free(streams[i]);

    remove(test_path);

    printf("接口设计原则:\n");
    printf("  1. 接口只定义行为, 不定义数据\n");
    printf("  2. 不同实现可以完全不同\n");
    printf("  3. 调用方只依赖接口, 不依赖实现\n");
    printf("  4. 新增实现不需要修改调用方\n");
}

typedef struct {
    double (*area)(const void *self);
    double (*perimeter)(const void *self);
    void (*print)(const void *self);
    const char *(*type)(const void *self);
} ShapeVTable;

typedef struct {
    const ShapeVTable *vtable;
} ShapeInterface;

typedef struct {
    ShapeInterface base;
    double radius;
} InterfaceCircle;

static double ic_area(const void *s) {
    const InterfaceCircle *c = (const InterfaceCircle *)s;
    return 3.14159265 * c->radius * c->radius;
}

static double ic_perimeter(const void *s) {
    const InterfaceCircle *c = (const InterfaceCircle *)s;
    return 2 * 3.14159265 * c->radius;
}

static void ic_print(const void *s) {
    const InterfaceCircle *c = (const InterfaceCircle *)s;
    printf("  圆: r=%.2f, 面积=%.2f, 周长=%.2f\n", c->radius, ic_area(s), ic_perimeter(s));
}

static const char *ic_type(const void *s) {
    (void)s;
    return "Circle";
}

static const ShapeVTable ic_vtable = { ic_area, ic_perimeter, ic_print, ic_type };

void demo_abstract_class(void) {
    printf("\n=== demo_abstract_class ===\n");
    printf("抽象类: 提供部分默认实现, 子类只需覆盖必要方法\n\n");

    InterfaceCircle c = { { &ic_vtable }, 5.0 };
    c.base.vtable->print(&c);

    printf("\n抽象类 vs 接口:\n");
    printf("  接口: 只声明方法, 没有任何实现\n");
    printf("  抽象类: 可以提供默认实现, 子类选择覆盖\n");
    printf("  C语言中: 接口=纯vtable, 抽象类=vtable+默认函数\n");
}

int main(void) {
    printf("接口与抽象类实现\n");

    demo_stream_interface();
    demo_abstract_class();

    printf("\n所有演示完成!\n");
    return 0;
}
