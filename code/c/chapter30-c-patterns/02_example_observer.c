/**
 * @file 02_example_observer.c
 * @brief 观察者/回调模式
 * @description 对应文档: 30-C语言设计模式实现
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_OBSERVERS 16

typedef void (*ObserverCallback)(const char *event_name, const void *data, void *user_data);

typedef struct {
    ObserverCallback callback;
    void *user_data;
} Observer;

typedef struct {
    Observer observers[MAX_OBSERVERS];
    int count;
} EventEmitter;

void emitter_init(EventEmitter *em) {
    em->count = 0;
    for (int i = 0; i < MAX_OBSERVERS; i++) {
        em->observers[i].callback = NULL;
        em->observers[i].user_data = NULL;
    }
}

int emitter_subscribe(EventEmitter *em, ObserverCallback cb, void *user_data) {
    if (em->count >= MAX_OBSERVERS) return -1;
    em->observers[em->count].callback = cb;
    em->observers[em->count].user_data = user_data;
    em->count++;
    return 0;
}

void emitter_emit(EventEmitter *em, const char *event_name, const void *data) {
    for (int i = 0; i < em->count; i++) {
        if (em->observers[i].callback) {
            em->observers[i].callback(event_name, data, em->observers[i].user_data);
        }
    }
}

void emitter_unsubscribe(EventEmitter *em, ObserverCallback cb) {
    for (int i = 0; i < em->count; i++) {
        if (em->observers[i].callback == cb) {
            for (int j = i; j < em->count - 1; j++) {
                em->observers[j] = em->observers[j + 1];
            }
            em->count--;
            em->observers[em->count].callback = NULL;
            em->observers[em->count].user_data = NULL;
            return;
        }
    }
}

typedef struct {
    int temperature;
    int humidity;
    EventEmitter events;
} WeatherStation;

void weather_station_init(WeatherStation *ws) {
    ws->temperature = 0;
    ws->humidity = 0;
    emitter_init(&ws->events);
}

void weather_station_update(WeatherStation *ws, int temp, int humidity) {
    int temp_changed = (ws->temperature != temp);
    int humid_changed = (ws->humidity != humidity);

    ws->temperature = temp;
    ws->humidity = humidity;

    if (temp_changed) {
        emitter_emit(&ws->events, "temperature_change", &ws->temperature);
    }
    if (humid_changed) {
        emitter_emit(&ws->events, "humidity_change", &ws->humidity);
    }
}

void display_observer(const char *event, const void *data, void *user_data) {
    const char *name = (const char *)user_data;
    int value = *(const int *)data;
    printf("  [%s显示器] %s = %d\n", name, event, value);
}

void alert_observer(const char *event, const void *data, void *user_data) {
    int threshold = *(int *)user_data;
    int value = *(const int *)data;
    if (strcmp(event, "temperature_change") == 0 && value > threshold) {
        printf("  [高温警报] 温度 %d 超过阈值 %d!\n", value, threshold);
    }
}

void log_observer(const char *event, const void *data, void *user_data) {
    (void)user_data;
    int value = *(const int *)data;
    printf("  [日志] 事件: %s, 值: %d\n", event, value);
}

void demo_observer_pattern(void) {
    printf("\n=== demo_observer_pattern ===\n");
    printf("观察者模式: 一对多依赖, 状态变化自动通知\n\n");

    WeatherStation ws;
    weather_station_init(&ws);

    char display_name[] = "客厅";
    int threshold = 35;
    emitter_subscribe(&ws.events, display_observer, display_name);
    emitter_subscribe(&ws.events, alert_observer, &threshold);
    emitter_subscribe(&ws.events, log_observer, NULL);

    printf("更新温度=25, 湿度=60:\n");
    weather_station_update(&ws, 25, 60);

    printf("\n更新温度=38 (高温警报):\n");
    weather_station_update(&ws, 38, 60);

    printf("\n取消日志观察者:\n");
    emitter_unsubscribe(&ws.events, log_observer);

    printf("更新温度=30, 湿度=70:\n");
    weather_station_update(&ws, 30, 70);

    printf("\n观察者模式优势:\n");
    printf("  1. 松耦合: 被观察者不知道观察者的具体实现\n");
    printf("  2. 动态订阅: 运行时添加/移除观察者\n");
    printf("  3. 广播通信: 一次通知所有观察者\n");
}

typedef struct {
    void (*on_click)(void *widget, void *data);
    void (*on_hover)(void *widget, void *data);
    void *click_data;
    void *hover_data;
    char label[64];
} Button;

void button_init(Button *btn, const char *label) {
    strncpy(btn->label, label, sizeof(btn->label) - 1);
    btn->on_click = NULL;
    btn->on_hover = NULL;
    btn->click_data = NULL;
    btn->hover_data = NULL;
}

void button_set_on_click(Button *btn, void (*handler)(void *, void *), void *data) {
    btn->on_click = handler;
    btn->click_data = data;
}

void button_click(Button *btn) {
    printf("  按钮 \"%s\" 被点击\n", btn->label);
    if (btn->on_click) {
        btn->on_click(btn, btn->click_data);
    }
}

void submit_handler(void *widget, void *data) {
    (void)widget;
    const char *form = (const char *)data;
    printf("  -> 提交表单: %s\n", form);
}

void cancel_handler(void *widget, void *data) {
    (void)widget;
    (void)data;
    printf("  -> 取消操作\n");
}

void demo_callback_pattern(void) {
    printf("\n=== demo_callback_pattern ===\n");
    printf("回调模式: 将函数作为参数传递, 在特定事件时调用\n\n");

    Button submit_btn, cancel_btn;
    button_init(&submit_btn, "提交");
    button_init(&cancel_btn, "取消");

    char form_name[] = "用户注册";
    button_set_on_click(&submit_btn, submit_handler, form_name);
    button_set_on_click(&cancel_btn, cancel_handler, NULL);

    button_click(&submit_btn);
    button_click(&cancel_btn);

    printf("\n回调 vs 观察者:\n");
    printf("  回调: 一对一, 一个事件一个处理函数\n");
    printf("  观察者: 一对多, 一个事件多个处理函数\n");
    printf("  回调更简单, 观察者更灵活\n");
}

void demo_observer_pitfalls(void) {
    printf("\n=== demo_observer_pitfalls ===\n");
    printf("观察者模式陷阱:\n\n");

    printf("1. 内存泄漏: 观察者未取消订阅\n");
    printf("   观察者对象被销毁但仍在观察者列表中\n");
    printf("   解决: 观察者析构时自动取消订阅\n\n");

    printf("2. 通知顺序: 观察者收到通知的顺序不确定\n");
    printf("   解决: 文档明确说明, 或按优先级排序\n\n");

    printf("3. 递归通知: 观察者中修改被观察者导致循环\n");
    printf("   解决: 添加重入保护标志\n\n");

    printf("4. 性能: 大量观察者时通知开销大\n");
    printf("   解决: 批量通知, 异步通知, 按需订阅\n\n");

    printf("5. 线程安全: 多线程通知需要同步\n");
    printf("   解决: 加锁保护观察者列表\n");
}

int main(void) {
    printf("观察者/回调模式\n");

    demo_observer_pattern();
    demo_callback_pattern();
    demo_observer_pitfalls();

    printf("\n所有演示完成!\n");
    return 0;
}
