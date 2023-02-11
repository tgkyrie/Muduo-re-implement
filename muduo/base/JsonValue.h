#if !defined(_MUDUO_JSON_JSONVALUE_H_)
#define _MUDUO_JSON_JSONVALUE_H_
#include<memory>
#include<vector>
#include<string>
#include<algorithm>
#include<fstream>
#include<iterator>
#include<assert.h>
// #define DEBUG
#ifdef DEBUG
#define DBGprint(...) printf(__VA_ARGS__)
#else
#define DBGprint(...)
#endif
namespace muduo{
namespace json
{
class JsonValue
{
public:
    enum JType {
            kInt,
            kFloat,
            kString,
            kHash,
            kArray,
            kBool,
            kNull,
        };
    using JsonValuePtr=std::unique_ptr<JsonValue>;
private:
    template<typename T>
    using DecayType=typename std::decay<T>::type;
    template<typename T>
    using RemoveCVRef=std::__remove_cvref_t<T>;

    template<typename T>
    struct IsString{
        static constexpr bool value=std::is_same<DecayType<T>,char*>::value||
                                    std::is_same<DecayType<T>,std::string>::value;
    };
    template <int N>
    struct IsString<char[N]>{
        static constexpr bool value=true;
    };
    template<typename T>
    struct IsInteger{
        static constexpr bool value=std::is_same<DecayType<T>,std::int8_t>::value||
                                    std::is_same<DecayType<T>,std::int16_t>::value||
                                    std::is_same<DecayType<T>,std::int32_t>::value||
                                    std::is_same<DecayType<T>,std::int64_t>::value;
    };
    template<typename T>
    struct IsFloat{
        static constexpr bool value=std::is_same<DecayType<T>,float>::value||
                                    std::is_same<DecayType<T>,double>::value;
    };
    template<typename T>
    struct IsBool{
        static constexpr bool value=std::is_same<DecayType<T>,bool>::value;
    };
    //parser
    struct Parser{
    public:

        static const char* skipSpace(const char* begin){
            while (*begin!='\0'&&*begin<=32)
            {
                begin++;
            }
            return begin;
        }
        static const char* parseHash(const char* next,std::vector<JsonValuePtr>& output){
            output.clear();
            assert(*next=='{');
            next=skipSpace(next+1);
            while (*next!='}')
            {
                assert(*next=='\"');
                JsonValuePtr element(std::make_unique<JsonValue>());
                next=parseString(next,element->key_);
                assert(*next==':');
                next++;
                next=parseObject(next,element.get());
                output.push_back(std::move(element));
                if(*next==','){
                    next++;
                    next=skipSpace(next);
                }
            }
            //next == }
            return skipSpace(next+1);
        }
        static const char* parseArray(const char* next,std::vector<JsonValuePtr>& output){
            output.clear();
            assert(*next=='[');
            next=skipSpace(next+1);
            while (*next!=']')
            {
                JsonValuePtr element(std::make_unique<JsonValue>());
                next=parseObject(next,element.get());
                output.push_back(std::move(element));
                if(*next==','){
                    next++;
                    next=skipSpace(next);
                }
            }
            return skipSpace(next+1);
        }
        static const char* parseString(const char* next,std::string& output){
            output.clear();
            assert(*next=='\"');
            next=skipSpace(next+1);
            while (*next!='\"')
            {
                //
                // DBGprint("%c",*next);
                output+=*next;
                next++;
            }
            return skipSpace(next+1);
        }
        static const char * parseNumber(const char* next,void* output,bool& isFloat){
            const char* end=next;
            std::string numStr;
            isFloat=false;
            while (isdigit(*end)||*end=='.')
            {
                end++;
                if(*end=='.')isFloat=true;
            }
            numStr.assign(next,end);
            if(isFloat)*(double*)output=std::strtod(next,nullptr);
            else *(long long*)output=std::stoll(numStr);
            return skipSpace(end);
            
        }
        static const char* parseFalse(const char* next,bool* output){
            *output=false;
            return skipSpace(next+5);
        }
        static const char* parseTrue(const char* next,bool* output){
            *output=true;
            return skipSpace(next+4);
        }
        static const char* parseObject(const char* next,JsonValue* output){
            next=skipSpace(next);
            if(*next=='{'){
                next=parseHash(next,output->elements_);
                output->type_=kHash;
            }
            else if(*next=='['){
                next=parseArray(next,output->elements_);
                output->type_=kArray;
            }
            else if(*next=='\"'){
                next=parseString(next,output->str_);
                output->type_=kString;
            }
            else if(isdigit(*next)){
                bool isFloat;
                next=parseNumber(next,&output->intNum_,isFloat);
                output->type_=isFloat?kFloat:kInt;
            }
            else if(next[0]=='t'&&next[1]=='r'&&next[2]=='u'&&next[3]=='e'){
                next=parseTrue(next,&output->boolNum_);
                output->type_=kBool;
            }
            else if(next[0]=='f'&&next[1]=='a'&&next[2]=='l'&&next[3]=='s'&&next[4]=='e'){
                next=parseFalse(next,&output->boolNum_);
                output->type_=kBool;
            }
            else if(tolower(next[0])=='n'&&tolower(next[1])=='u'&&tolower(next[2])=='l'&&tolower(next[3])=='l'){
                output->type_=kNull;
            }
            return skipSpace(next);
        } //parseObject

    };//struct parser



    // template<class Key>
    // JsonValue& operator[](const Key& key){
    //     if constexpr(IsInteger<Key>::value){
    //         // return key<elements_.size()?elements_[key]:nullptr;
    //         assert(key<elements_.size());
    //         return *(elements_[key].get());
    //     }
    //     if constexpr(IsString<Key>::value){
    //         auto it=std::find(elements_.begin(),elements_.end(),key);
    //         return it==elements_.end()?*it:nullptr;
    //     }
    //     // return nullptr;
    //     bool keyIsNotStringOrInt=true;
    //     assert(keyIsNotStringOrInt==true);
    // }


    // JsonValue Clone(const JsonValue& json){
    //     switch (json.type_)
    //     {
    //     case 
    //         break;
        
    //     default:
    //         break;
    //     }
    // }

public:
    ~JsonValue(){}

    JsonValue(): type_(JType::kNull){}
    JsonValue(JsonValue && json){
        *this = std::move(json);
    }
    JsonValue(const JsonValue& json){

    }
    //operator =
    JsonValue& operator=(const JsonValue& json){

    }
    JsonValue& operator=(JsonValue&& json){
        elements_ = std::move(json.elements_);
        intNum_=json.intNum_;
        str_ = std::move(json.str_);
        type_ = std::move(json.type_);
        key_ = std::move(json.key_);
        json.type_ = kNull;
        return *this;
    }
    template<class T,typename std::enable_if < !std::is_same<JsonValue,DecayType<T>>::value , bool>::type>
    JsonValue& operator=(T&& val){
        using RawType=RemoveCVRef<T>;
        if constexpr(IsInteger<RawType>::value){
            elements_.clear();
            intNum_=val;
            type_=kInt;
        }
        if constexpr(IsFloat<RawType>::value){
            elements_.clear();
            floatNum_=val;
            type_=kFloat;
        }
        if constexpr(IsString<RawType>::value){
            elements_.clear();
            str_=std::forward<T>(val);
            type_=kString;
        }
        if constexpr(IsBool<RawType>::value){
            elements_.clear();
            boolNum_=val;
            type_=kBool;
        }
    }
    //get
    template<class ...Keys>
    JsonValue* get(const Keys& ...keys){
        return getKeys(keys...);
    }
    //insert
    template<class Val,class ...Keys>
    JsonValue* insert(Val&& val,const Keys& ...keys){
        return set(std::forward<Val>(val),keys...);
    }
    int toInt(){
        return intNum_;
    }
    const int& toInt()const{
        return intNum_;
    }
    const std::string& toStr()const{
        return str_;
    }
    bool toBool()const{
        return boolNum_;
    }
    // template<class Key,class Val>
    // JsonValuePtr insert(Key&& key,Val&& val){
    //     if constexpr(IsInteger<Key>::value)assert(type_==kArray);
    //     if constexpr(IsString<Key>::value)assert(type_==kHash);
    //     JsonValuePtr json=get(key);
    //     if(json==nullptr){
    //         JsonValuePtr newVal=std::make_unique<JsonValue>(std::forward<Val>(val));
    //         if constexpr(IsString<Key>::value){
    //             newVal->key_=std::forward<Key>(key);
    //         }
    //         elements_.push_back(newVal);
    //         return newVal;
    //     }
    //     else{
    //         json->set(std::forward<Val>(val));
    //     }
    //     return json;
    // }


    const std::string& key()const{
        return key_;
    }
    void fromString(const std::string& str){
        Parser::parseObject(str.c_str(),this);
    }
    void fromFile(const std::string & fname)
    {
        std::ifstream ifile(fname);
        std::string buffer;
        std::copy(std::istream_iterator<char>(ifile), 
                std::istream_iterator<char>(), 
                std::back_inserter(buffer));
        fromString(buffer);
    }
    std::string serialize(){
        std::string ret="";
        serilizeImpl(ret,0);
        return std::move(ret);
    }
private:
    void serilizeImpl(std::string& output,int tabNum){
        std::string tab(tabNum,'\t');
        if(type_==kHash){
            output+="{\n";
            int i=0;
            for(;i<elements_.size()-1;i++){
                output+=tab+"\t\""+elements_[i]->key_+"\":";
                elements_[i]->serilizeImpl(output,tabNum+1);
                output+=",\n";
            }
            output+=tab+"\t\""+elements_[i]->key_+"\":";
            elements_[i]->serilizeImpl(output,tabNum+1);
            output+="\n"+tab+"}";
        }
        else if(type_==kArray){
            output+="[\n";
            int i=0;
            for(;i<elements_.size()-1;i++){
                output+=tab+"\t";
                elements_[i]->serilizeImpl(output,tabNum+1);
                output+=",\n";
            }
            output+=tab+"\t";
            elements_[i]->serilizeImpl(output,tabNum+1);
            output+="\n"+tab+"]";
        }
        else if(type_==kInt){
            output+=std::to_string(intNum_);
        }
        else if(type_==kFloat){
            output+=std::to_string(floatNum_);
        }
        else if(type_==kBool){
            if(boolNum_){
                output+="true";
            }
            else {
                output+="false";
            }
        }
        else if(type_==kString){
            output+="\""+str_+"\"";
        }
    }
    template<typename Key,typename ...Keys>
    JsonValue* getKeys(const Key& key,const Keys& ...keys){
        JsonValue* nextNode=getKeys(key);
        if(nextNode==nullptr){
            // DBGprint("notFound\n");
            return nullptr;
        }
        return nextNode->getKeys(keys...);
    }
    template<typename Key>
    JsonValue* getKeys(const Key& key){
        // DBGprint("isString?=%d\n",IsString<Key>::value);
        using RawType=RemoveCVRef<Key>;
        if constexpr(IsInteger<RawType>::value){
            return (key<elements_.size()&&type_==kArray)?
                    elements_[key].get():nullptr;
        }
        if constexpr(IsString<RawType>::value){
            if(type_!=kHash){
                // DBGprint("not Hash\n");
                return nullptr;
            }
            auto it=std::find(elements_.begin(),elements_.end(),key);
            // DBGprint("key=%s\n",key_.c_str());
            return (it!=elements_.end())?
                    (it->get()):nullptr;
        }
        // DBGprint("not int or String\n");
        return nullptr;
    }
    template<class Val>
    JsonValue* set(Val&& val){
        using RawType=RemoveCVRef<Val>;
        if constexpr(IsInteger<RawType>::value){
            elements_.clear();
            intNum_=val;
            type_=kInt;
            return this;
        }
        if constexpr(IsFloat<RawType>::value){
            elements_.clear();
            floatNum_=val;
            type_=kFloat;
            return this;
        }
        if constexpr(IsString<RawType>::value){
            elements_.clear();
            str_=std::forward<Val>(val);
            type_=kString;
            return this;
        }
        if constexpr(IsBool<RawType>::value){
            elements_.clear();
            boolNum_=val;
            type_=kBool;
            return this;
        }
        // DBGprint("Not valid type\n");
        return nullptr;
    }
    template<class Val,class Key,class ...Keys>
    JsonValue* set(Val&& val,const Key& key,const Keys& ...keys){
        JsonValue* jsonPtr=getKeys(key);
        // DBGprint("1\n");
        using RawType=RemoveCVRef<Key>;
        if(jsonPtr==nullptr){
            if constexpr(IsString<RawType>::value){
                JsonValuePtr newVal(std::make_unique<JsonValue>());
                JsonValue* rawPtr=newVal.get();
                JsonValue* ret=rawPtr->set(std::forward<Val>(val),keys...);
                if(ret==nullptr){
                    return nullptr;//failed
                }
                type_=kHash;
                elements_.clear();
                newVal->key_=key;
                elements_.push_back(std::move(newVal));
                return ret;
            }
            if constexpr(IsInteger<RawType>::value){
                JsonValuePtr newVal(std::make_unique<JsonValue>());
                JsonValue* rawPtr=newVal.get();
                JsonValue* ret=rawPtr->set(std::forward<Val>(val),keys...);
                if(ret==nullptr){
                    return nullptr;//failed
                }
                type_=kArray;
                elements_=std::vector<JsonValuePtr>(key+1,nullptr);
                elements_[key]=std::move(newVal);
                return ret;
            }
            //key is not a string or int
            return nullptr;
        }
        return jsonPtr->set(std::forward<Val>(val),keys...);
    }
    /* data */
    std::vector<JsonValuePtr> elements_;
    std::string str_;
    std::string key_;
    union 
    {
        double floatNum_;
        long long intNum_;
        bool boolNum_;
    };
    JType type_;


};
inline bool operator==(const JsonValue::JsonValuePtr& ptr,const std::string& key){
    return ptr->key()==key;
}
inline bool operator!=(const JsonValue::JsonValuePtr& ptr,const std::string& key){
    return ptr->key()!=key;
}


    
} // namespace json
}



#endif // _JSON_JSONVALUE_H_
