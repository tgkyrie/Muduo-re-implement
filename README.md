# muduo网络库

## 特点
+ 使用Epoll LT模式，非阻塞IO，实现主从Reactor模式
+ 封装条件变量+RAII风格锁解决生产者-消费者问题，实现线程池
+ 改用最小堆结合Linux timefd 实现定时器管理,使用懒惰删除取消定时任务,与原muduo红黑树定时器相比性能更好
+ 使用智能指针管理内存
+ 实现class Any类代替原有代码中的boost::any类,实现的Any类代码中使用了std::enable_if等元编程技术
+ 支持HTTP1.0\1.1,支持GET请求，新增POST请求
+ 新增Json模块，使用LL1文法分析解析Json，使用模板元技术和变参模板实现get和set的多态.
+ 改用链式数组实现Buffer(待实现)

## muduo结构梳理
### TcpServer
TcpServer拥有一个acceptor和Eventloop线程池，还有一个loop指出当前Server所属loop

构造时，TcpServer设置acceptor处理新链接的回调

新链接到达,acceptor调用回调newConnection,获取一个Eventloop线程池中的loop，将该链接分配给loop

### Acceptor
acceptor拥有一个loop指针指出所属loop，一个socket对象，一个channel

acceptor构造时创建一个非阻塞socket,并创建一个channel，设置channel的读回调为acceptor::handleRead

acceptor listen,调用listen，然后再调用channel->enableRead,channel->enableRead会调用update更新loop中的channel

### EventLoopThreadPool
EventLoopThreadPool接口有start(),getNextLoop()

start()创建多个EventLoopThread，并加入到EventLoopThread列表中,getNextLoop()公平的返回一个loop

### Channel
channel拥有一个所属loop指针,一个文件描述符,关注事件和触发事件

channel属于loop



### EventLoopThread

## 主从Reactor模式

### io多路复用
io多路复用是 一个线程监听多个文件描述符是否可以进行IO操作的能力

### 为什么需要io多路复用
对于单线程而言，假如同时与多个客户端通信，有多个socketfd，若只使用阻塞IO，读一个fd就会阻塞，直到它关闭，没办法与多个客户端通信

若使用非阻塞IO，则没有IO事件的时候会一直轮询，busy wating浪费CPU资源

而使用IO多路复用，没有IO事件就会阻塞，让出CPU资源，而任意一个fd有IO事件就会被唤醒处理,这样能保证同时处理多个IO事件，也能合理利用CPU资源

### Linux IO复用
Linux有三种IO复用接口 select，poll，epoll

#### select
```c
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
```
select调用时，首先把fd_set拷贝到内核，然后遍历fd_set，如果有IO事件就设置对应用户态fd_set中的fd，没有就阻塞，直到有IO事件或超时。

用户检查IO事件的时候也要遍历fd_set，通过FD_ISSET宏来判断某个fd是否有IO事件。也就是说select不返回一个有IO事件的set，而是通过设置用户态中fd_set中某个fd的标志位来通知用户有IO事件

#### select缺点
+ 涉及到用户态到内核态的内存拷贝，消耗大
+ 用户检查时要遍历所有fd，尽管多个fd中只有一个fd有IO事件
+ 监听fd数量有限
+ 若监听fd最小值为min，最大值为max，检查时就要把min到max中的所有描述符都检查一遍

#### poll
poll和select类似，只不过poll用数组保存要监听描述符，检查时只要遍历数组

#### epoll
+ 执行epoll_create会创建一个红黑树和一个就绪链表，链表保存就绪的事件
+ 加入监听fd，会往红黑树插入fd，然后注册一个回调函数，当fd有IO事件会调用回调函数，将fd放入就绪链表中
+ 返回时，通过将就绪链表的fd移动到数组中返回给用户，通过mmap共享数组内存避免拷贝

#### epoll LT模式
和select poll一样，有事件epoll就会返回可读事件（可读事件指内核读缓冲区有字节）。

而可写事件是指内核写缓冲区有空闲区域可写，所以一开始都不监听可写事件，等到内核缓冲区写满了，write返回0才监听。可写事件触发，写入后若内核缓冲区还有空闲则关闭监听。

#### epoll ET模式
边缘触发，类似数电中的上升沿检测器，当不可读->可读，或有新数据时epoll返回

ET模式下，有数据就一直读，读到返回0或者EAGAIN错误
写数据，只要可写就一直写，直到写完或返回EAGAIN错误

### 非阻塞IO

#### 为什么要使用非阻塞IO
在网络程序中，需要与多个客户端通信，若使用阻塞IO，则一个线程read时就阻塞到对等方发完数据，read返回0之后才能处理其他客户端，而线程占用内存，数量是有限制的，这就会影响并发度。

使用非阻塞IO，有IO事件时就读取存入用户缓冲区，调用用户定义的onMessage回调，之后继续处理其他IO事件

### 多种Reactor模式

#### 单Reactor单线程
epoll返回后有IO事件就将回调加入队列中，处理完后执行队列里的任务

#### 单Reactor多线程或线程池
主线程负责监听所有IO事件，有客户端的读写事件则分配一个线程执行回调，涉及跨线程调用

#### 主从Reactor模式
主Reactor只负责accept的IO事件，当有新链接时，分配一个副Reactor关注该链接的读写事件。

## 线程池

### 条件变量
#### pthread条件变量api pthread_cond_wait为什么有mutex参数
避免死锁，在等待条件之前释放锁，在返回前获得锁

如果没有mutex参数，就得这样写
```cpp
lock(mutex);
while(condition){
    unlock(mutex);
    condition.wait();
    lock(mutex);
}
//do something
```
为简化所以加了mutex参数

### RAII风格锁之lock_guard
lock_guard原理很简单,代码如下
```cpp
class lock_guard{
    mutex& mutex_
    lock_guard(mutex& mutex):mutex_(mutex){
        mutex_.lock();
    }
    ~lock_guard(){
        mutex_.unlock();
    }
}
```
构造时申请锁，析构时释放锁，避免忘记释放锁

### 线程池
线程池是一个典型的多生产者-多消费者问题，消费者是空闲线程，生产者是用户，用户将任务放入任务队列中，空闲线程取出队列执行
```cpp
Mutex mutex;
queue<task> q; //Guarded by mutex
Condition notEmpty(mutex); //Guarded by mutex
Condition notFull(mutex); //Guarded by mutex

//product
void run(task f){
    lock_guard lock(mutex);
    while(q.full()){
        notFull.wait();
    }
    q.push(f);
    notEmpty.notify();
}
//consume
void take(){
    lock_guard lock(mutex);
    while(q.empty()){
        notEmpty.wait();
    }
    task f=q.pop();
    notFull.notify();
    f();
}
void threadFunc(){
    while(1)take();
}

```

## 定时器
linux内核提供timerfd接口，把定时器事件作为一个文件处理，当发生超时，timerfd上有可读事件，可以结合epoll来实现定时器管理。

### 如何实现定时器
#### 定时器操作
+ 添加定时任务
+ 删除定时任务
+ 执行超时任务

最简单的做法是一个定时器分配一个timerfd，用epoll监听所有timerfd并注册回调。这种做法实际上直接让内核管理定时器，效率不高，因为fd资源有限，一般采用下面的方法



时间是有序的，因此可以维护一个有序的数据结构，key值为超时时间
+ 对于添加定时任务，我们需要快速在有序数据结构中插入
+ 对于删除定时任务，需要快速找到定时任务，并删除
+ 对于执行超时任务，需要快速定位超时时间小于now的任务，并删除

### 数据结构的选择
对于时间复杂度
+ 有序链表
  + 添加定时任务，时间复杂度O(n)
  + 删除定时任务，复杂度O(1)
  + 对于k个超时任务，复杂度O(k)
+ 最小堆
  + 添加定时任务，复杂度O(lgn)
  + 删除定时任务，理论上需要查找O(n)加O(lgn)删除,实际上通过懒惰删除策略可以降为O(1)
  + 执行k个超时任务，O(klgn)
+ 红黑树
  + 添加定时任务，O(lgn)
  + 删除定时任务，O(lgn)删除
  + 执行k个超时任务，O(klgn)

上述只是理论值，虽然红黑树和最小堆看上去性能差不多，但是由于采用大O时间复杂度，常数不一样，而且要考虑摊还代价，实际上最小堆性能却优于红黑树。

我曾经也一度认为这两个性能应该相差不大，选用红黑树实现定时器，但在后续实际测试了红黑树和最小堆的性能后才改成了最小堆。经测试，最小堆插入比标准库std::set中的红黑树插入快10倍，而pop k个最小值时间差别不大。

### 实现方法
对于插入操作，直接push即可

超时事件处理,只要堆的top的超时 < 当前时间，就pop掉进行处理

删除操作，如果要删除指定定时任务，需要遍历所有任务，这在定时任务量多的时候开销特别大，对比以下$10^6$和$log10^6=6log10$，差距很大

为了解决这个问题，可以采用懒惰删除，即在定时任务类中设置一个Bool变量标志该任务是否被删除。在取消任务的时候，设置这个标志即可

定时器管理结构TimerQueue在遍历超时任务的时候会查看这个标志，如果已取消了，那么就不再执行，并且后续将进行析构，不再插入回最小堆中

## Any类

### 为什么需要Any类
典型的例子就是HttpServer,在解析一个客户端链接的Http请求的时候，Http报文可能不完整，需要保存当前解析的状态，而对于不同的应用，其状态的数据类型不一样(Http中是HttpContent),所以需要一个Any类来存储任意类型的数据

### 如何实现
可以使用void* 类型指针，但他不能保证类型安全

也可以用模板
```cpp
template<typename T>
class Any{
    T data_;
};
```
这样可以实现
```cpp
Any<int> a=1;
Any<string> b="string";
```
这其实没什么用，Any类的目标是支持下面这种用法
```cpp
Any a=1;//或Any a(1);
Any b="string";
int c=a;
string d=a; //error
```

那么有什么办法呢,可以再封装一层
```cpp
class Any{
    template<typename T>
    class DataType{
        T data_;
    };
    DataType<T>* ptr_;
};
```
但这个还不行,没办法在构造的时候知道ptr_的类型,因为它是模板类型的指针,而Any类不是模板类，因此Any类中就不能有模板类型的成员变量

但可以在Any中定义一个非模板的基类指针，让基类指针指向派生模板类DataType对象加上虚函数实现多态
```cpp
class Any{
    class BaseType{
    public
        BaseType()=default;
        virtual ~BaseType()=default;
    }
    template<typename T>
    class DataType:public BaseType
    {
        template<typename T>
        DataType(const T&);
        template<typename T>
        DataType(T&&);
        T data_;
    };
    BaseType* ptr_;
    template<typename T>
    operator=(T&& val){
        delete ptr_;
        ptr_=new DataType<T>(std::forward<T>(val));
    }
}
```
这样就可以实现Any类的目标了，剩下就是一些拷贝移动，内存管理，类型检查等内容，可以参考`muduo/base/Any.h`

### 元编程
元编程主要作用在于
+ 编译期计算，提高运行时速度
+ 提高类型安全
  
在实现的代码中使用了模板元`std::enable_if`,`std::decay`,`std::is_same`，主要用于类型安全，原理可以自行查阅资料




