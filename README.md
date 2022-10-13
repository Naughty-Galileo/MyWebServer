# MyWebServer
> Linux下C++轻量级Web服务器，逐模块分解学习，逐渐重构属于自己的WebServer.

>## 写在前面
> 感谢各位大佬的无私开源与知识分享，使后来者入门变得容易 \
> 特别感谢[TinyWebServer](https://github.com/qinguoyi/TinyWebServer)这一优秀项目


## <div align="center"> 线程同步 [./lock/locker.h](./lock/locker.h) :smile: </div> 
> 为允许在线程或进程间共享数据，同步通常是必须的，常见的同步方式有：**互斥锁、条件变量、读写锁、信号量**。\
> 另外，对于进程间的同步，也可以通过进程间通信的方式进行同步，包括**管道（无名管道、有名管道）、信号量、消息队列、共享内存、远程过程调用、Socket**

<details>
<summary> <b> 信号量 </b> </summary>

> 访问具有原子性，用以解决进程或线程间共享资源引发的同步问题 \
> 等待信号量：信号量为0，程序等待；信号量大于0，信号量减一，程序继续运行 \
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

</details>


<details>
<summary> <b> 互斥锁 </b> </summary>

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

</details>

<details>
<summary> <b> 条件变量 </b> </summary>

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

</details>


## <div align="center"> 线程池  [./threadpool/threadpool.h](./threadpool/threadpool.h)  :smile:</div> 
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

## <div align="center"> 数据库连接池 [./CGImysql/sql_connection_pool.h](CGImysql/sql_connection_pool.h) :smile:</div> 
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

<details>
<summary> <b> API </b> </summary>

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

</details>

### RAII机制
> Resource Acquisition Is Initialization（资源获取即初始化），RAII是C++语法体系中的一种常用的合理管理资源避免出现内存泄漏的常用方法。以对象管理资源，利用的就是C++构造的对象最终会被对象的析构函数销毁的原则

- 优点
    - 不需要显式地释放资源。
    - 采用这种方式，对象所需的资源只在其生命期内始终保持有效

## <div align="center"> HTTP连接 [./http/http_conn.h](http/http_conn.h) :smile:</div> 

<details>
<summary> <b> 1、HTTP介绍 </b> </summary>

#### HTTP报文
- 请求报文（浏览器向服务器发送）
    - 请求报文=请求行(request line) + 请求头部(header) + 空行 + 请求数据
- 响应报文（服务器处理后返回给浏览器端）
    - 响应报文 = 状态行 + 消息报头 + 空行 + 响应

#### HTTP方法
| 方法 | 描述 | 
| -- | -- |
| GET | 请求指定页面信息，返回实体主体 | 
| HEAD | 类似于GET，返回没有具体内容，用于获取报头 |
| POST | 向指定资源提交数据进行处理请求 |
| PUT | 从客户端向服务器传送的数据取代指定的文档内容 |
| DELETE | 请求服务器删除指定的页面 |
| CONNECT | HTTP/1.1协议中预留给能够将连接改为管道方式的代理服务器 |
| OPTIONS | 允许客服端查看服务器的性能 |
| TRACE | 回显服务器收到的请求，用于测试和诊断 |

#### HTTP状态码
- 1xx：指示信息--表示请求已接收，继续处理
- 2xx：成功--表示请求正常处理完毕
    - 200 OK：客户端请求被正常处理
    - 206 Partial content：客户端进行了范围请求
- 3xx：重定向--要完成请求必须进行更进一步的操作
    - 301 Moved Permanently：永久重定向，该资源已被永久移动到新位置，将来对该资源访问都要使用本响应返回的若干个URI之一
    - 302 Found：临时重定向，请求的资源临时从不同的URI中获得
- 4xx：客户端错误--请求有语法错误，服务器无法处理请求
    - 400 Bad Request：请求报文存在语法错误
    - 403 Forbidden：请求被服务器拒绝
    - 404 Not Found：请求不存在，服务器上找不到请求的资源
- 5xx：服务器端错误--服务器处理请求出错
    - 500 Internal Server Error：服务器在执行请求时出现错误

#### HTTP处理流程
- 处理连接请求 浏览器端发出http连接请求，主线程创建http对象接收请求并将所有数据读入对应buffer，将该对象插入任务队列，等待工作线程从任务队列中取出一个任务进行处理
- 处理报文请求 工作线程取出任务后，调用进程处理函数，通过主、从状态机对请求报文进行解析
- 返回响应报文 解析完之后，生成响应报文，返回给浏览器端

</details>

---

### 2、HTTP处理连接请求 
### （1）服务器接收http请求
```c++
#include <sys/socket.h>
#include <netinet/in.h>
/* 创建监听socket文件描述符 */
int listenfd = socket(PF_INET, SOCK_STREAM, 0);
/* 创建监听socket的TCP/IP的IPV4 socket地址 */
struct sockaddr_in address;
bzero(&address, sizeof(address));
address.sin_family = AF_INET;
address.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY：将套接字绑定到所有可用的接口 */
address.sin_port = htons(port);

int flag = 1;
/* SO_REUSEADDR 允许端口被重复使用 */
setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
/* 绑定socket和它的地址 */
ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));  
/* 创建监听队列以存放待处理的客户连接，在这些客户连接被accept()之前 */
ret = listen(listenfd, 5);
```

> 服务器端主线程创建http对象接收请求并将所有数据读入对应buffer，将该对象插入任务队列后，工作线程从任务队列中取出一个任务进行处理
> 各子线程通过process函数对任务进行处理，调用process_read函数和process_write函数分别完成报文解析与报文响应两个任务

### （2）http 读取请求报文 http_coon::read_once
```c++
int recv( SOCKET s, char FAR *buf, int len, int flags);
// 指定接收端套接字描述符
// 指明一个缓冲区，该缓冲区用来存放recv函数接收到的数据
// 指明buf的长度
// flag设0
```

### （3）解析报文 http_coon::process_read
- 判断条件
    - 主状态机转移到CHECK_STATE_CONTENT，该条件涉及解析消息体
    - 从状态机转移到LINE_OK，该条件涉及解析请求行和请求头部
- 循环体
    - 从状态机读取数据 prase_line()
    - get_line() 获取 text
    - 主状态机解析text 后parse_request_line(text)/parse_headers(text)/parse_content(text)

- 从状态机 解析报文 http_conn::parse_line()
    - 在HTTP报文中，每一行的数据由\r\n作为结束字符，空行则是仅仅是字符\r\n
    - 读到\r
        - 若下一位读到末尾 返回LINE_OPEN
        - 若下一位读到\n 置为\0\0 m_checked_idx指向下一行头部，返回LINE_OK
        - 否则 语法发送错误 返回LINE_BAD
    - 读到\n
        - 若前一个为\r 置为\0\0 m_checked_idx指向下一行头部，返回LINE_OK
    - 都不是 继续接收 返回LINE_OPEN

#### 主状态机
```
GET /562f2.jpg HTTP/1.1
Host:img.mukewang.com
User-Agent:Mozilla/5.0 (Windows NT 10.0; WOW64)
AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.106 Safari/537.36
Accept:image/webp,image/*,*/*;q=0.8
Referer:http://www.imooc.com/
Accept-Encoding:gzip, deflate, sdch
Accept-Language:zh-CN,zh;q=0.8
空行
请求数据为空
```
- parse_request_line() 处理请求行 涉及strpbrk/strcasecmp/strspn
- parse_headers()
    - 空行 
        - 若content-length为0则是GET请求 解析结束 
        - 不为0 为POST请求，则状态转移到CHECK_STATE_CONTENT
    - 不为空 分析connection字段，content-length，host字段 
        - connection判断长连接还是短连接 
        - content-length读取post请求的消息体长度
- parse_content()
    - 仅用于解析POST请求
    - 用于保存post请求消息体，为后面的登录和注册做准备

### （4）响应报文  http_coon::process_write
> 服务器子线程完成报文的解析与响应；主线程监测读写事件，调用read_once和http_conn::write完成数据的读取与发送

#### mmap 
> 用于将一个文件或其他对象映射到内存，提高文件的访问速度

```c++
void* mmap(void* start,size_t length,int prot,int flags,int fd,off_t offset);
int munmap(void* start,size_t length);
```

#### 1）do_request
- m_url有如下可能
    - / GET请求 跳转欢迎访问页面
    - /0 POST请求 跳转注册页面
    - /1 POST请求 跳转登录界面
    - /2CGISQL.cgi POST请求 登录校验
        - 验证成功跳转 资源请求成功页面
        - 验证失败跳转 登录失败页面
    - /3CGISQL.cgi POST请求 注册校验
        - 注册成功跳转 登录页面
        - 注册失败跳转 注册失败页面
    - /5 POST 请求 跳转图片请求页面
    - /6 POST请求 跳转视频请求页面
    - /7 POST请求 跳转关注页面

#### 2）process_write 向m_write_buf中写入响应报文
> 服务器子线程调用process_write向m_write_buf中写入响应报文
> 
- add_status_line
- add_headers
    - add_content_length(int content_len)
    - add_linger()
    - add_blank_line()
- add_content
> 均依赖于add_reaponse函数
- add_reaponse
- 两种响应报文
    - 请求文件 声明两个iovec 第一个指向m_write_buf 第二个指向mmap的地址m_file_address
    - 请求出错 只申请一个iovec 指向m_write_buf

<details>
<summary> 相关函数 </summary>

```c++
// 在一次函数调用中写多个非连续缓冲区，有时也将这该函数称为聚集写
// filedes表示文件描述符
// iov为io向量机制结构体iovec
// iovcnt为结构体的个数
#include <sys/uio.h>
ssize_t writev(int filedes, const struct iovec *iov, int iovcnt);

// 向量元素
struct iovec {
    void      *iov_base;      /* starting address of buffer */
    size_t    iov_len;        /* size of buffer */
};

// stat函数用于取得指定文件的文件属性，并将文件属性存储在结构体stat里
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//获取文件属性，存储在statbuf中
int stat(const char *pathname, struct stat *statbuf);

struct stat 
{
   mode_t    st_mode;        /* 文件类型和权限 */
   off_t     st_size;        /* 文件大小，字节数*/
};

// 向一个字符串缓冲区打印格式化字符串，且可以限定打印的格式化字符串的最大长度
int vsnprintf (char * sbuf, size_t n, const char * format, va_list arg );
```
</details>

#### 3）http_conn::write 将响应报文发送给浏览器端
> 服务器子线程调用process_write完成响应报文，随后注册epollout事件。服务器主线程检测写事件，并调用http_conn::write函数将响应报文发送给浏览器端


### 3、epoll
>  I/O event notification facility
> linux2.6内核的一个新系统调用
> sys/epoll.h
> 项目中epoll相关代码部分包括非阻塞模式、内核事件表注册事件、删除事件、重置EPOLLONESHOT事件四种

<details>
<summary> epoll相关 </summary>

### API
```c++
// 创建epoll实例 返回fd
int epoll_create (int __size); // __size无用
int epoll_create1 (int __flags);

// 将监听的文件描述符添加到epoll实例中 成功返回0
int epoll_ctl (int __epfd, int __op, int __fd, struct epoll_event *__event);

// 等待epoll事件从epoll实例中发生， 并返回事件以及对应文件描述符
int epoll_wait (int __epfd, struct epoll_event *__events, int __maxevents, int __timeout);
int epoll_pwait (int __epfd, struct epoll_event *__events, int __maxevents, int __timeout, const __sigset_t *__ss);

typedef union epoll_data
{
  void *ptr;
  int fd;
  uint32_t u32;
  uint64_t u64;
} epoll_data_t;

struct epoll_event
{
  uint32_t events;	/* Epoll events */
  epoll_data_t data;	/* User data variable */
} __EPOLL_PACKED;
```
### epoll事件类型
- EPOLLIN：表示对应的文件描述符**可以读（包括对端SOCKET正常关闭）**
- EPOLLOUT：表示对应的文件描述符**可以写**
- EPOLLPRI：表示对应的文件描述符**有紧急的数据可读**
- EPOLLERR：表示对应的文件描述符**发生错误**
- EPOLLHUP：表示对应的文件描述符**被挂断**
- EPOLLET：将EPOLL设为**边缘触发(Edge Triggered)模式**
- EPOLLONESHOT：**只监听一次事件**，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里 
- EPOLLRDHUP：表示读关闭，对端关闭，不是所有的内核版本都支持

### op动作
- EPOLL_CTL_ADD (注册新的fd到epfd)
- EPOLL_CTL_MOD (修改已经注册的fd的监听事件)
- EPOLL_CTL_DEL (从epfd删除一个fd)

### 设置非阻塞模式
- fcntl 根据文件描述词来操作文件的特性
```c++
#include <unistd.h>
#include <fcntl.h>

int fcntl(int fd, int cmd);

int fcntl(int fd, int cmd, long arg);         

int fcntl(int fd, int cmd, struct flock *lock);

// 复制一个现有的描述符（cmd=F_DUPFD）
// 获得／设置文件描述符标记(cmd=F_GETFD或F_SETFD)
// 获得／设置文件状态标记(cmd=F_GETFL或F_SETFL)
// 获得／设置异步I/O所有权(cmd=F_GETOWN或F_SETOWN)
// 获得／设置记录锁(cmd=F_GETLK,F_SETLK或F_SETLKW)
```

</details>


## <div align="center"> 定时器 [./timer/lst_timer.h](./timer/lst_timer.h) :smile:</div> 
> 由于非活跃连接占用了连接资源，严重影响服务器的性能，通过实现一个服务器定时器，处理这种非活跃连接，释放连接资源。利用alarm函数周期性地触发SIGALRM信号,该信号的信号处理函数利用管道通知主循环执行定时器链表上的定时任务

---

### 定时方法与信号通知流程

<details>
<summary> <b> 定时方法基础API </b> </summary>

sigaction结构体
```c++
struct sigaction {
    void (*sa_handler)(int);
    void (*sa_sigaction)(int, siginfo_t *, void *);
    sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
}
// sa_handler是一个函数指针，指向信号处理函数
// sa_sigaction同样是信号处理函数，有三个参数，可以获得关于信号更详细的信息
// sa_mask用来指定在信号处理函数执行期间需要被屏蔽的信号
// sa_flags用于指定信号处理的行为
//      SA_RESTART，使被信号打断的系统调用自动重新发起
//      SA_NOCLDSTOP，使父进程在它的子进程暂停或继续运行时不会收到 SIGCHLD 信号
//      SA_NOCLDWAIT，使父进程在它的子进程退出时不会收到 SIGCHLD 信号，这时子进程如果退出也不会成为僵尸进程
//      SA_NODEFER，使对信号的屏蔽无效，即在信号处理函数执行期间仍能发出这个信号
//      SA_RESETHAND，信号处理之后重新设置为默认的处理方式
//      SA_SIGINFO，使用 sa_sigaction 成员而不是 sa_handler 作为信号处理函数
// sa_restorer一般不使用
```

sigaction函数
```c++
#include <signal.h>
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
// signum表示操作的信号
// act表示对信号设置新的处理方式
// oldact表示信号原来的处理方式
// 返回值，0 表示成功，-1 表示有错误发生
```

sigfillset函数
```c++
#include <signal.h>
int sigfillset(sigset_t *set); // 用来将参数set信号集初始化，然后把所有的信号加入到此信号集里
```

SIGALRM、SIGTERM信号
```c++
#define SIGALRM  14     //由alarm系统调用产生timer时钟信号
#define SIGTERM  15     //终端发送的终止信号
```

alarm函数
```c++
#include <unistd.h>;
unsigned int alarm(unsigned int seconds);
// 设置信号传送闹钟，即用来设置信号SIGALRM在经过参数seconds秒数后发送给目前的进程
// 如果未设置信号SIGALRM的处理函数，那么alarm()默认处理终止进程
```

socketpair函数
```c++
#include <sys/types.h>
#include <sys/socket.h>

int socketpair(int domain, int type, int protocol, int sv[2]);
// domain表示协议族，PF_UNIX或者AF_UNIX
// type表示协议，可以是SOCK_STREAM或者SOCK_DGRAM，SOCK_STREAM基于TCP，SOCK_DGRAM基于UDP
// protocol表示类型，只能为0
// sv[2]表示套节字柄对，该两个句柄作用相同，均能进行读写双向操作
// 返回结果， 0为创建成功，-1为创建失败
```

send函数
```c++
#include <sys/types.h>
#include <sys/socket.h>

ssize_t send(int sockfd, const void *buf, size_t len, int flags); // send a message on a socket
```

</details>

### 信号通知流程
> Linux下的信号采用的==异步处理机制==，信号处理函数和当前进程是两条不同的执行路线

- 统一事件源
> 指将信号事件与其他事件一样被处理
> 信号处理函数使用管道将信号传递给主循环，信号处理函数往管道的写端写入信号值，主循环则从管道的读端读出信号值，使用I/O复用系统调用来监听管道读端的可读事件，这样信号事件与其他文件描述符都可以通过epoll来监测，从而实现统一处理

- 信号处理机制
  - 信号的接收
      - 接收信号的任务是由内核代理的，当内核接收到信号后，会将其放到对应进程的信号队列中，同时向进程发送一个中断，使其陷入内核态。注意，此时信号还只是在队列中，对进程来说暂时是不知道有信号到来的
  - 信号的检测
      - 进程从内核态返回到用户态前进行信号检测
      - 进程在内核态中，从睡眠状态被唤醒的时候进行信号检测
  - 信号的处理
      - (内核)信号处理函数是运行在用户态的，调用处理函数前，内核会将当前内核栈的内容备份拷贝到用户栈上，并且修改指令寄存器（eip）将其指向信号处理函数
      - (用户)接下来进程返回到用户态中，执行相应的信号处理函数
      - (内核)信号处理函数执行完成后，还需要返回内核态，检查是否还有其它信号未处理
      - (用户)如果所有信号都处理完成，就会将内核栈恢复（从用户栈的备份拷贝回来），同时恢复指令寄存器（eip）将其指向中断前的运行位置，最后回到用户态继续执行进程
  
---

### 定时器及其容器设计、定时任务处理
> 1、**定时器设计**，将连接资源和定时事件等封装起来，具体包括连接资源、超时时间和回调函数，这里的回调函数指向定时事件
> 2、**定时器容器设计**，将多个定时器串联组织起来统一处理，具体包括升序链表设计
> 3、**定时任务处理函数**，该函数封装在容器类中，具体的，函数遍历升序链表容器，根据超时时间，处理对应的定时器

- 定时器设计
  - 连接资源
    - 客户端套接字地址
    - 文件描述符
    - 定时器
  - 定时事件
    - 回调函数 删除非活动的socket上的注册事件
  - 超时时间
    - 浏览器和服务器连接时刻 + 固定时间(TIMESLOT)

### 定时器容器
> 项目中的定时器容器为带头尾结点的升序双向链表，具体的为每个连接创建一个定时器，将其添加到链表中，并按照超时时间升序排列。执行定时任务时，将到期的定时器从链表中删除。
> 主要是双向链表的查删改操作

- add_timer
- adjust_timer
- del_timer
- add_timer(util_timer *timer, util_timer *lst_head)


### 定时任务处理函数
> 使用统一事件源，SIGALRM信号每次被触发，主循环中调用一次定时任务处理函数，处理链表容器中到期的定时器

- 遍历链表
    - 当前时间小于定时器超时时间 后续都小 跳出循环
    - 大于超时时间 执行回调函数（删除注册事件、关闭连接） 删除此节点


### 怎么用的定时器？
- 浏览器与服务器连接时，创建该连接对应的定时器，并将该定时器添加到链表上
- 处理异常事件时，执行定时事件，服务器关闭连接，从链表上移除对应定时器
- 处理定时信号时，将定时标志设置为true
- 处理读事件时，若某连接上发生读事件，将对应定时器向后移动，否则，执行定时事件
- 处理写事件时，若服务器通过某连接给浏览器发送数据，将对应定时器向后移动，否则，执行定时事件

```c++
/* 定时器相关参数 */
static int pipefd[2];
static sort_timer_lst timer_lst

/* 每个user（http请求）对应的timer */
client_data* user_timer = new client_data[MAX_FD];
/* 每隔TIMESLOT时间触发SIGALRM信号 */
alarm(TIMESLOT);
/* 创建管道，注册pipefd[0]上的可读事件 */
int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
/* 设置管道写端为非阻塞 */
setnonblocking(pipefd[1]);
/* 设置管道读端为ET非阻塞，并添加到epoll内核事件表 */
addfd(epollfd, pipefd[0], false);

addsig(SIGALRM, sig_handler, false);
addsig(SIGTERM, sig_handler, false);
```
---

