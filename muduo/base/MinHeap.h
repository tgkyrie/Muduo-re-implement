#if !defined(_MUDUO_BASE_MINHEAP_H_)
#define _MUDUO_BASE_MINHEAP_H_


#include<vector>

namespace muduo
{

template<typename T>
class MinHeap
{
private:
    /* data */
    std::vector<T> data_;
    ssize_t size_;
    ssize_t capacity_;
    std::function<void (T&,ssize_t)> setIndexCallback_;
    inline ssize_t parent(ssize_t i){
        return (i-1)/2;
    }
    inline ssize_t left(ssize_t i){
        return 2*i+1;
    }
    inline ssize_t right(ssize_t i){
        return 2*i+2;
    }
    static const int kInitSize=16;
public:
    MinHeap(int initSize=kInitSize):
        data_(initSize),
        size_(0),
        capacity_(initSize){}

    ~MinHeap()=default;
    ssize_t push(const T& x){
        // DBGprint("%d",size_);
        ssize_t i=size_++;
        while (i>0&&x<data_[parent(i)])
        {
            // std::swap(data_[i],data_[parent(i)]);
            data_[i]=std::move(data_[parent(i)]);
            i=parent(i);
        }
        data_[i]=x;
        expand();
        return i;        
    }
    ssize_t push(T&& x){
        ssize_t i=size_++;
        // DBGprint("%ld",size_);
        while (i>0&&x<data_[parent(i)])
        {
            // std::swap(data_[i],data_[parent(i)]);
            data_[i]=std::move(data_[parent(i)]);
            i=parent(i);
        }
        data_[i]=std::move(x);
        expand();
        return i;  
    }
    const T& top()const{
        return data_[0];
    }
    void pop(){
        T x=std::move(data_[size_-1]);
        size_-=1;
        ssize_t i=0;
        while (1)
        {
            ssize_t minIdx=-1;
            if(left(i)<size_&&x>data_[left(i)]){
                minIdx=left(i);
            }
            if(right(i)<size_&&x>data_[right(i)]){
                if(!(minIdx!=-1&&data_[left(i)]<data_[right(i)])){
                    minIdx=right(i);
                }
            }
            if(minIdx!=-1){
                data_[i]=std::move(data_[minIdx]);
                i=minIdx;
            }
            else break;
        }
        data_[i]=std::move(x);
        shrink();
    }
    void remove(ssize_t i){
        if(i==size_-1){
            size_-=1;
            return;
        }
        T x=std::move(data_[size_-1]);
        size_-=1;
        if(parent(i)>=0&&x<data_[parent(i)]){
            while (parent(i)>=0&&x<data_[parent(i)])
            {
                // std::swap(data_[i],data_[parent(i)]);
                data_[i]=std::move(data_[parent(i)]);
                i=parent(i);
            }
            data_[i]=std::move(x);
        }
        else{
            while (1)
            {
                ssize_t minIdx=-1;
                if(left(i)<size_&&x>data_[left(i)]){
                    minIdx=left(i);
                }
                if(right(i)<size_&&x>data_[right(i)]){
                    if(!(minIdx!=-1&&data_[right(i)]>data_[left(i)])){
                        minIdx=right(i);
                    }
                }
                if(minIdx!=-1){
                    data_[i]=std::move(data_[minIdx]);
                    i=minIdx;
                }
                else break;
            }
            data_[i]=std::move(x);
        }
        shrink();
    }
    inline void shrink(){
        if(__builtin_expect(size_<=capacity_/2,0)){
            data_.resize(size_);
            capacity_=size_;
        }
    }
    inline void expand(){
        if(__builtin_expect(size_==data_.capacity(),0)){
            data_.resize(2*size_);
        }
    }
    ssize_t size()const {
        return size_;
    }
    bool empty()const{
        return size_==0;
    }
    void setIndexCallback(const std::function<void (T&,ssize_t)>& cb){
        setIndexCallback_=cb;
    }

    //for debug
    void assertOrder(){
        for(int i=0;i<size_;i++){
            if(left(i)<size_){
                assert(data_[i]<data_[left(i)]);
            }
            if(right(i)<size_){
                assert(data_[i]<data_[right(i)]);
            }
        }
    }
};

}


#endif // _MUDUO_BASE_MINHEAP_H_
