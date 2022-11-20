#ifndef WHEEL_TIMER
#define WHEEL_TIMER

#include <time.h>
#include <netinet/in.h>
#include <stdio.h>
#include "lst_timer.h"

class tw_timer;

// 连接资源
struct client_data
{
    // 客户端socket地址
    sockaddr_in address;
    // socket文件描述符
    int sockfd;
    // 定时器
    // util_timer *timer;
    tw_timer* timer;
};

// 定时器 需要记录对应的圈数和槽数
class tw_timer
{
public:
    tw_timer(int rot, int ts) : next(NULL), prev(NULL), rotation(rot), time_slot(ts) {}

public:
    int rotation;
    int time_slot;
    void (* cb_func)(client_data *);
    client_data* user_data;
    tw_timer* next;
    tw_timer* prev;
};

class time_wheel
{
public:
    time_wheel() : cur_slot(0)
    {
        for (int i=0; i<N; i++) {
            slots[i] = NULL;
        }
    }

    ~time_wheel()
    {
        for (int i=0; i<N; i++) {
            tw_timer* tmp = slots[i];
            while (tmp) {
                slots[i] = tmp->next;
                delete tmp;
                tmp = slots[i];
            }
        }
    }

    tw_timer* add_timer(int timeout)
    {
        if (timeout < 0) {
            return NULL;
        }

        int ticks = 0;

        if (timeout < SI) {
            ticks = 1;
        }
        else {
            ticks = timeout / SI;
        }

        int rotation = ticks / N;
        int ts = (cur_slot + (ticks % N)) % N;

        tw_timer* timer = new tw_timer(rotation, ts);

        if (!slots[ts]) {
            slots[ts] = timer;
        }
        else {
            timer->next = slots[ts];
            slots[ts]->prev = timer;
            slots[ts] = timer;
        }
        return timer;
    }

    void del_timer(tw_timer* timer)
    {
        if (!timer) {
            return;
        }

        int ts = timer->time_slot;

        if (timer == slots[ts]) {
            slots[ts] = slots[ts]->next;
            if (slots[ts]) {
                slots[ts]->prev = NULL;
            }
            delete timer;
        }
        else {
            timer->prev->next = timer->next;
            if (timer->next) {
                timer->next->prev = timer->prev;
            }
            delete timer;
        }
    }

    void tick()
    {
        tw_timer* tmp = slots[cur_slot];
        while (tmp)
        {
            if (tmp->rotation > 0) {
                tmp->rotation--;
                tmp = tmp->next;
            }
            else {
                tmp->cb_func(tmp->user_data);
                if (tmp == slots[cur_slot]) {
                    slots[cur_slot] = tmp->next;
                    delete tmp;
                    if (slots[cur_slot]) {
                        slots[cur_slot]->prev =NULL;
                    }
                    tmp = slots[cur_slot];
                }

                else {
                    tmp->prev->next = tmp->next;
                    if (tmp->next) {
                        tmp->next->prev = tmp->prev;
                    }
                    tw_timer* tmp2 = tmp->next;
                    delete tmp;
                    tmp = tmp2;
                }
            }
        }
        cur_slot = ++cur_slot % N;
    }

private:
    // 时间轮上的槽数
    static const int N = 60;
    // 槽间隔时间 1秒
    static const int SI = 1;
    // 每个槽对应的定时器链表
    tw_timer* slots[N];
    // 当前槽
    int cur_slot;
};
#endif