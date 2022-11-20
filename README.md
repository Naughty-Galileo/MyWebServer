# 快速开始
## 初始化数据库
```bash
// 建立yourdb库
create database yourdb;

// 创建user表
USE yourdb;
CREATE TABLE user(
    username char(50) NULL,
    passwd char(50) NULL
)ENGINE=InnoDB;

// 添加数据
INSERT INTO user(username, passwd) VALUES('name', 'passwd');
```

## 修改main.cpp中的数据库初始化信息
```c++
//数据库登录名,密码,库名
string user = "root";
string passwd = "root";
string databasename = "yourdb";
```

## build
```bash
sh ./build.sh
```

## 启动并浏览
```bash
./server

ip:9006
```

# 改进
- 新增时间轮的实现，相较于基于升序链表的计时器更加高效

