#include"muduo/net/TcpConnection.h"
#include"muduo/base/Log.h"

using namespace muduo;
using namespace muduo::net;

void muduo::net::defaultConnCallback(const TcpConnectionPtr& conn){
    // LOG_TRACE<<conn->localAddr()<<"->"
    //          <<conn->peerAddr()<<"is"
    //          <<conn->connected()?"UP":"DOWN";
}

TcpConnection::TcpConnection(EventLoop* loop,
                            const string& name,
                            int sockfd,
                            const InetAddr& localAddr,
                            const InetAddr& peerAddr):
state_(kConnecting),
loop_(loop),
reading_(true),
name_(name),
socket_(new Socket(sockfd)),
sockChannel_(new Channel(loop,sockfd)),
localAddr_(localAddr),
peerAddr_(peerAddr)
{
    sockChannel_->setReadCallback(
        std::bind(&TcpConnection::handleRead,
            this,std::placeholders::_1
        )
    );

    sockChannel_->setErrorCallback(
        std::bind(&TcpConnection::handleError,this)
    );

    sockChannel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose,this)
    );

    sockChannel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite,this)
    );
    //不能在这里enable channel,因为还没构造完，绑定的this指针无效
}

TcpConnection::~TcpConnection(){
    // LOG_DEBUG << "TcpConnection::dtor[" <<  name_ << "] at " << this
    //             << " fd=" << sockChannel_->fd()
    //             << " state=" << state_<<"\n";
    assert(state_ == kDisConnected);
}

void TcpConnection::connectEstablished(){
    assert(loop_->isInLoopThread());
    state_=kConnected;
    sockChannel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed(){
    loop_->assertInLoopThread();
    if(state_==kConnected){
        state_=kDisConnected;
        sockChannel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    sockChannel_->remove();

}

void TcpConnection::handleRead(Timestamp receiveTime){
    int savedErrno;
    //LOG_DEBUG<<"TcpConnection::handleRead(),fd="+std::to_string(socket_->fd())+"\n";
    size_t n=inputBuffer_.readFd(socket_->fd(),savedErrno);
    if(n>0){
        //messageCallback
        //LOG_DEBUG<<"TcpConnection::handleRead() ,msgcb\n";
        messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);
    }
    else if(n==0){
        //LOG_DEBUG<<"TcpConnection::handleRead() ,read FIN\n";
        handleClose();
    }
    else{
        errno = savedErrno;
        //LOG_DEBUG<<"TcpConnection::handleRead() ,ERROR n<0\n";
        // LOG_SYSERR << "TcpConnection::handleRead";
        handleError();
    }
}

void TcpConnection::handleWrite(){
    loop_->assertInLoopThread();
    //LOG_INFO<<peerAddr_.IpPort()<<std::endl;
    assert(sockChannel_->isWriting());
    if(sockChannel_->isWriting()){
        ssize_t nwrote=0;
        size_t readable=outputBuffer_.readableBytes();
        nwrote=write(socket_->fd(),outputBuffer_.peek(),outputBuffer_.readableBytes());
        if(nwrote>0){
            outputBuffer_.retrieve(nwrote);
            if(outputBuffer_.readableBytes()==0){
                sockChannel_->disableWriting();
                if(writeCompleteCallback_){
                    loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
                }
            }
        }
        else{
            // LOG_SYSERR << "TcpConnection::handleWrite";
        }
    }
}

void TcpConnection::handleClose(){
    // LOG_TRACE<<"TcpConnection::handleClose() "+peerAddr_.IpPort()+" closed\n";
    // assert(state_ == kConnected || state_ == kDisConnecting);
    state_=kDisConnected;
    sockChannel_->disableAll();
    TcpConnectionPtr guardthis(shared_from_this());
    connectionCallback_(guardthis);
    closeCallback_(guardthis);
}

void TcpConnection::handleError(){
    
}

void TcpConnection::send(const char* s){
    if(state_!=kConnected)return;
    send((const void*)s,strlen(s));
}

void TcpConnection::send(Buffer* buf){
    if(state_!=kConnected)return;
    sendInLoop(buf->peek(),buf->readableBytes());
    buf->retrieve(buf->readableBytes());
}

void TcpConnection::send(const std::string& s){
    if(state_!=kConnected)return;
    if(loop_->isInLoopThread()){
        sendInLoop(s.c_str(),s.size());
    }
    else{
        loop_->runInLoop(
            std::bind(&TcpConnection::sendInLoopAsString,this,s)
        );
    }
}

void TcpConnection::send(const void* buf,size_t len){
    if(state_!=kConnected)return;
    if(loop_->isInLoopThread()){
        sendInLoop(buf,len);
    }
    else{
        loop_->runInLoop(
            std::bind(&TcpConnection::sendInLoop,this,buf,len)
        );
    }
    
}

void TcpConnection::sendInLoopAsString(const std::string s){
    if(state_!=kConnected)return;
    loop_->assertInLoopThread();
    sendInLoop(s.c_str(),s.size());
}

void TcpConnection::sendInLoop(const void* buf,size_t len){
    if(state_!=kConnected)return;
    loop_->assertInLoopThread();
    size_t nwrote=0;
    bool faultError=false;
    size_t remain=len;
    if(!sockChannel_->isWriting()&&outputBuffer_.readableBytes()==0){
        nwrote=write(socket_->fd(),buf,len);
        if(nwrote>=0){
            remain=len-nwrote;
            if(remain==0&&writeCompleteCallback_){
                //写完回调
                //这里不直接调用是为了避免栈溢出，因为回调可能执行sendInLoop
                loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
            }
        }
        else{
            nwrote=0;
            // LOG_SYSERR << "TcpConnection::sendInLoop";
            if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
            {
                faultError = true;
            }
        }
    }
    if(remain>0&&!faultError){
        outputBuffer_.append((const char*)buf+nwrote,remain);
        if(!sockChannel_->isWriting()){
            sockChannel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown()
{
  // FIXME: use compare and swap
    if (state_ == kConnected)
    {
        state_=kDisConnecting;
        // FIXME: shared_from_this()?
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if (!sockChannel_->isWriting())
    {
        // we are not writing
        //LOG_DEBUG<<"shutdown writeing"<<std::endl;
        socket_->shutdownWrite();
    }
}