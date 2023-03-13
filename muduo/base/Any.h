#if !defined(_MUDUO_BASE_ANY_H_)
#define _MUDUO_BASE_ANY_H_
#include"muduo/base/Log.h"
#include<memory>
namespace muduo
{
namespace net
{
class Any
{
private:
    /* data */
    class InternalData{
    public:
        InternalData()=default;
        virtual ~InternalData()=default;
        virtual std::unique_ptr<InternalData> clone()const =0;
    };
    template<typename T>
    class InternalDataImpl:public InternalData{
    public:
        T data_;
        InternalDataImpl(const T& data):data_(data){
            //LOG_TRACE<<"InternalDataImpl(const T& data)\n";
        }
        InternalDataImpl(T&& data):data_(std::move(data)){
            //LOG_TRACE<<"InternalDataImpl(T&& data)\n";
        }

        virtual std::unique_ptr<InternalData> clone() const override{
            return std::unique_ptr<InternalDataImpl>
                    (new InternalDataImpl<T>(data_));
        }
    };

    std::unique_ptr<InternalData> dataPtr_;
public:
    Any()=default;
    Any(const Any& that){
        if(that.dataPtr_){
            dataPtr_=that.dataPtr_->clone();
        }
    }
    Any(Any&& that):dataPtr_(std::move(that.dataPtr_)){}
    ~Any(){
        // LOG_INFO<<"destruct any\n";
    }
    Any& operator=(const Any& that){
        if(that.dataPtr_){
            dataPtr_=that.dataPtr_->clone();
        }
        return *this;
    }
    Any& operator=(Any&& that){
        dataPtr_=std::move(that.dataPtr_);
        return *this;
    }

    template<typename T> using DecayType=typename std::decay<T>::type;
    template<typename T,typename std::enable_if<!std::is_same<DecayType<T>,Any>::value,bool>::type=true>
    Any(T &&data){
        dataPtr_.reset(new InternalDataImpl<DecayType<T>>(std::forward<T>(data)));
    }

    template<typename T,typename std::enable_if<!std::is_same<DecayType<T>,Any>::value,bool>::type=true>
    Any& operator=(T&& data){
        dataPtr_.reset(new InternalDataImpl<DecayType<T>>(std::forward<T>(data)));
        return *this;
    }

    bool empty()const {return dataPtr_==nullptr;}
    template<typename T>
    const T& cast()const{
        return static_cast<InternalDataImpl<T>*>(dataPtr_.get())->data_;
    }
    template<typename T>
    T& cast(){
        //LOG_TRACE<<"Any::cast()\n";
        return static_cast<InternalDataImpl<T>*>(dataPtr_.get())->data_;
    }
};


} // namespace net

    
} // namespace muduo



#endif // _MUDUO_BASE_ANY_H_
