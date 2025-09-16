// 这是一个头文件保护指令，作用和传统的 #ifndef / #define / #endif 一样。它保证头文件只会被编译器包含一次，避免重复定义错误。
#pragma once

namespace PerCache
{
    // 定义一个命名空间 KamaCache，这样就可以把缓存相关的代码都组织在这个空间下，避免和其他库的类、函数名冲突。
    template <typename Key, typename Value>
    class KICachePolicy
    {
    public:
        // 析构函数
        virtual ~KICachePolicy() {};

        // 缓存接口get 和 put
        virtual bool get(Key key, Value &value) = 0; // 访问到的值以传出参数value形式返回
        virtual Value get(Key key) = 0;
        virtual void put(Key key, Value value) = 0;
        /*
        = 0 是 C++ 里的一个特殊语法，叫做 纯虚函数（pure virtual function）
        普通虚函数：virtual void foo();    // 可以有默认实现，也可以被子类 override
        纯虚函数：virtual void foo() = 0;  // 没有实现，子类必须实现
    它告诉编译器：这是一个抽象方法，我这里只是声明，不提供实现。谁继承我，谁必须去实现它。
        */
    };
} // namespace PerCache
