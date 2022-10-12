# MyWebServer
> Linux下C++轻量级Web服务器，逐模块分解学习，逐渐重构属于自己的WebServer.

>## 写在前面
> 感谢各位大佬的无私开源与知识分享，使后来者入门变得容易 \
> 特别感谢[TinyWebServer](https://github.com/qinguoyi/TinyWebServer)这一优秀项目


## 线程同步 ./lock/locker.h :smile:
> 为允许在线程或进程间共享数据，同步通常是必须的，常见的同步方式有：**互斥锁、条件变量、读写锁、信号量**。\
> 另外，对于进程间的同步，也可以通过进程间通信的方式进行同步，包括**管道（无名管道、有名管道）、信号量、消息队列、共享内存、远程过程调用、Socket**

### 信号量
> 访问具有原子性，用以解决进程或线程间共享资源引发的同步问题
> 等待信号量：信号量为0，程序等待；信号量大于0，信号量减一，程序继续运行
> 发送信号量：信号量加1

```c++
#include <semaphore.h>
```
- 相关函数
    - 创建信号量 
    ```c++
    int sem_init(sem_t *sem, int pshared, unsigned int value); // 初始化由 sem 指向的信号对象，并给它一个初始的整数值 value。pshared 控制信号量的类型，值为 0 代表该信号量用于多线程间的同步，值如果大于 0 表示可以共享，用于多个相关进程间的同步
    ```
   - sem_wait、sem_trywait
    ```c++
    int sem_wait(sem_t *sem); // 阻塞的函数 sem value > 0，则该信号量值减去 1 并立即返回。若sem value = 0，则阻塞直到 sem value > 0，此时立即减去 1，然后返回
    int sem_trywait(sem_t *sem); // 非阻塞的函数
    ```
   - sem_post
    ```c++
    int sem_post(sem_t *sem); // 把指定的信号量 sem 的值加 1，唤醒正在等待该信号量的任意线程 
    ```
    - sem_getvalue
    ```c++
    int sem_getvalue(sem_t *sem, int *sval); // 获取信号量 sem 的当前值，把该值保存在 sval，若有 1 个或者多个线程正在调用 sem_wait 阻塞在该信号量上，该函数返回阻塞在该信号量上进程或线程个数
    ```
    - sem_destroy
    ```c++
    int sem_destroy(sem_t *sem); // 清理信号量,成功则返回 0，失败返回 -1
    ```

### 互斥锁
- 线程互斥
    > 任何时刻，保证只有一个执行流进入临界区访问临界资源，通常对临界资源起到保护作用     
- 临界资源
    > 多线程执行流共享的资源就叫做临界资源临界区
- 原子性
    > 不会被任何调度机制打断的操作，该操作只有两态（无中间态，即使被打断，也不会受影响），要么完成，要么未完成
- 互斥量mutex
    > 多个线程对一个共享变量进行操控时，会引发数据不一致的问题。此时就引入了互斥量（也叫互斥锁）的概念，来保证共享数据操作的完整性。在被加锁的任一时刻，临界区的代码只能被一个线程访问。
- 互斥量接口
    >  **互斥量其实就是一把锁，是一个类型为pthread_mutex_t 的变量**，使用前需要进行初始化操作，使用完之后需要对锁资源进行释放
    - 初始化互斥量  
        - 静态分配
        ```c++
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
        ```
        - 动态分配
        ```c++
        int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutex_attr_t *mutexattr);
        ```
    - 加锁
    >对共享资源的访问，要对互斥量进行加锁，如果互斥量已经上了锁，调用线程会阻塞，直到互斥量被解锁
    ```c++
    int pthread_mutex_lock(pthread_mutex *mutex);
    int pthread_mutex_trylock(pthread_mutex_t *mutex);
    ```
    - 解锁
    > 在完成了对共享资源的访问后，要对互斥量进行解锁
    ```c++
    int pthread_mutex_unlock(pthread_mutex_t *mutex);
    ```
    - 销毁锁
    > 使用完成后，需要进行销毁以释放资源  
    ```c++
    int pthread_mutex_destroy(pthread_mutex *mutex);
    ```

### 条件变量
> 条件变量用来等待而不是上锁，用于自动阻塞一个线程，直到某特殊情况发送为止，通常条件变量与互斥锁同时使用

- 相关函数
    ```c++
    #include <pthread.h>
    //销毁条件变量
    int pthread_cond_destroy(pthread_cond_t *cond);
    //初始化条件变量
    int pthread_cond_init(pthread_cond_t *restrict cond,
        const pthread_condattr_t *restrict attr);
    //阻塞在条件变量上
    int pthread_cond_wait(pthread_cond_t *cv,
                        pthread_mutex_t *mutex);
    //解除在条件变量上的阻塞
    int pthread_cond_signal(pthread_cond_t *cv);
    //释放阻塞的所有线程
    int pthread_cond_broadcast(pthread_cond_t *cv);
    //阻塞直到指定时间
    int pthread_cond_timedwait(pthread_cond_t *cv,
    pthread_mutex_t *mp, const structtimespec * abstime);
    ```

## 线程池  ./threadpool/threadpool.h  :smile:
> 主线程负责读写，工作线程（线程池中的线程）负责处理逻辑（HTTP请求报文的解析）

### pthread相关函数
```c++
#include <pthread.h>

// 创建一个线程
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);

// 等待某个线程结束
int pthread_join(pthread_t thread, void **retval);

// 将已经运行中的线程设定为分离状态
// 线程的默认属性是非分离状态，这种情况下，原有的线程等待创建的线程结束。只有当pthread_join()函数返回时，创建的线程才算终止，才能释放自己占用的系统资源。而分离线程不是这样子的，它没有被其他的线程所等待，自己运行结束了，线程也就终止了，马上释放系统资源
int pthread_detach(pthread_t thread);

// 线程退出
//在线程执行的函数中调用此接口
void pthread_exit(void *value_ptr);
// pthread_exit(NULL);

// 线程取消
int pthread_cancel(pthread_t thread);
// pthread_cancel(pthread_self()); // pthread_self()获取PID
```

## 数据库连接池 ./CGImysql/sql_connection_pool.cpp :smile:
> 每一个HTTP连接获取一个数据库连接，获取其中的用户账号密码进行对比，而后再释放该数据库连接 \
> 在程序初始化的时候，集中创建多个数据库连接，并把他们集中管理，供程序使用，可以保证较快的数据库读写速度，更加安全可靠 \
> 类似于线程池的操作

### connection_pool 连接池
- 单例 设计模式

#### mysql API
- 使用mysql_init()初始化连接
- 使用mysql_real_connect()建立一个到mysql数据库的连接
- 使用mysql_query()执行查询语句
- 使用result = mysql_store_result(mysql)获取结果集
- 使用mysql_num_fields(result)获取查询的列数，mysql_num_rows(result)获取结果集的行数
- 通过mysql_fetch_row(result)不断获取下一行，然后循环输出
- 使用mysql_free_result(result)释放结果集所占内存
- 使用mysql_close(conn)关闭连接

```c++
// 初始化
MYSQL *mysql_init(MYSQL *mysql); // 返回分配的句柄MYSQL指针


// 连接数据库
MYSQL *mysql_real_connect(MYSQL *mysql, const char *host, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag); // 成功返回 连接句柄,失败返回 NULL

// 关闭连接
mysql_close(mysql);

// 读取数据
int mysql_query(MYSQL *mysql, const char *query);

// 获取结果集
MTSQL_RES * mysql_store_result (MYSQL * mysql); // 成功返回结果集，失败返回NULL

typedef struct MYSQL_RES {
  uint64_t row_count;
  MYSQL_FIELD *fields;
  struct MYSQL_DATA *data;
  MYSQL_ROWS *data_cursor;
  unsigned long *lengths; /* column lengths of current row */
  MYSQL *handle;          /* for unbuffered reads */
  const struct MYSQL_METHODS *methods;
  MYSQL_ROW row;         /* If unbuffered read */
  MYSQL_ROW current_row; /* buffer to current row */
  struct MEM_ROOT *field_alloc;
  unsigned int field_count, current_field;
  bool eof; /* Used by mysql_fetch_row */
  /* mysql_stmt_close() had to cancel this result */
  bool unbuffered_fetch_cancelled;
  enum enum_resultset_metadata metadata;
  void *extension;
} MYSQL_RES;

// 获取结果集 一行一行fetch结果集中的数据
MYSQL_ROW * mysql_fetch_row(MYSQL_RES * result);

typedef struct MYSQL_ROWS {
  struct MYSQL_ROWS *next; /* list of rows */
  MYSQL_ROW data;
  unsigned long length;
} MYSQL_ROWS;

// 解析结果多少行
unsigned int mysql_num_fields (MYSQL_RES *res);

// 从mysql中句柄中解析有多少行
unsigned int mysql_field_count(MYSQL *mysql);

// 获取表头
MYSQL_FIELD *mysql_fetch_fields(MYSQL_RES *res);

typedef struct st_mysql_field {                                                             
 96   char *name;                 /* Name of column */
 97   char *org_name;             /* Original column name, if an alias */
 98   char *table;                /* Table of column if column was a field */
 99   char *org_table;            /* Org table name, if table was an alias */
100   char *db;                   /* Database for table */
101   char *catalog;          /* Catalog for table */
102   char *def;                  /* Default value (set by mysql_list_fields) */
}MYSQL_FIELD
```

### RAII机制
> Resource Acquisition Is Initialization（资源获取即初始化），RAII是C++语法体系中的一种常用的合理管理资源避免出现内存泄漏的常用方法。以对象管理资源，利用的就是C++构造的对象最终会被对象的析构函数销毁的原则

- 优点
    - 不需要显式地释放资源。
    - 采用这种方式，对象所需的资源只在其生命期内始终保持有效


