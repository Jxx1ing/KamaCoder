#include <iostream>
#include <string>
#include <stdexcept> //out_of_range类
#include <sstream>   //isstring stream

template <typename T>
class Vector
{
private:
    T *elements;     // 指向动态数组的指针
    size_t capacity; // 数组的容量
    size_t size;     // 数组中的元素个数
public:
    // 1. 基础成员函数
    //  构造函数
    Vector() : elements(nullptr),
               capacity(0),
               size(0)
    {
    }

    // 析构函数
    ~Vector()
    {
        delete[] elements; // 释放 elements 指向的动态分配的内存
    }

    // 拷贝构造函数
    Vector(const Vector &other) : capacity(other.capacity),
                                  size(other.size)
    {
        // 深拷贝
        elements = new T[capacity];
        std::copy(other.elements, other.elements + size, elements);
    }
    /*
        first: 复制的源序列的起始位置（包含）
        last: 要复制的源序列的结束位置（不包含，即 [first, last)）
        d_first: 目标范围的起始位置
    */

    // 拷贝赋值操作符
    Vector &operator=(const Vector &other)
    {
        // 检查自赋值的情况。如果不是可以释放当前对象内存；如果是直接返回当前对象
        if (this != &other)
        {
            delete[] elements;
            capacity = other.capacity;
            size = other.size;
            elements = new T[capacity];
            std::copy(other.elements, other.elements + size, elements); // 深拷贝
        }
        return *this;
    }

    // 2.核心功能
    //  添加元素到数组末尾
    void push_back(const T &value)
    {
        if (size == capacity)
        {
            // 如果数组已满，扩展容量
            reserve(capacity == 0 ? 1 : 2 * capacity); // reserve申请内存 -> 复制元素 -> 释放旧内存
        }
        // reserve把成员变量 elements 赋值为新地址 （自定义实现reserve函数）
        elements[size++] = value;
    }

    // 获取数组中元素的个数
    size_t getSize() const
    {
        return size;
    }

    // 获取数组的容量
    size_t getCapacity() const
    {
        return capacity;
    }

    // 访问数组中的元素
    T &operator[](size_t index)
    {
        if (index >= size)
        {
            throw std::out_of_range("Index out of range");
        }
        return elements[index];
    }

    // const版本的访问数组中的元素
    const T &operator[](size_t index) const
    {
        if (index >= size)
        {
            throw std::out_of_range("Index out of range");
        }
        return elements[index];
    }
    /*
    第一个const: 返回值不可以被修改
    第二个const: const对象只能调用const成员函数 （但是非const对象都可以调用，不过优先非const成员函数）
    */

    // 在指定位置插入元素
    void insert(size_t index, const T &value)
    {
        if (index > size)
        {
            throw std::out_of_range("Index out of range");
        }
        if (size == capacity)
        {
            reserve(capacity == 0 ? 1 : capacity * 2);
        }
        for (size_t i = size; i > index; i--)
        {
            // 如果扩容,此时elements已经指向新的内存
            // 将当前索引及之后的元素向后移动一个位置
            elements[i] = elements[i - 1];
        }
        // 插入新元素
        elements[index] = value;
        size++;
    }

    // 删除数组末尾的元素
    void pop_back()
    {
        if (size > 0)
        {
            size--;
        }
    }

    // 清空数组
    void clear()
    {
        size = 0;
    }

    // 3、迭代与遍历
    // 使用迭代器遍历数组的初始位置
    T *begin()
    {
        return elements;
    }

    // 使用迭代器遍历数组的结束位置
    T *end()
    {
        return elements + size;
    }

    // 使用迭代器遍历数组的初始位置(const版本)
    const T *begin() const
    {
        return elements;
    }

    // 使用迭代器遍历数组的结束位置（const版本）
    const T *end() const
    {
        return elements + size;
    }

    // 打印数组中的元素
    void printElements() const
    {
        for (int i = 0; i < size; i++)
        {
            std::cout << elements[i] << " ";
        }
        std::cout << std::endl;
    }

    // 4. 高级特性
    void reserve(size_t newCapacity)
    {
        if (newCapacity > capacity)
        {
            T *newElements = new T[newCapacity];
            std::copy(elements, elements + size, newElements);
            delete[] elements;
            elements = newElements;
            capacity = newCapacity;
        }
    }
};

// main主函数
int main()
{
    // 创建一个Vector对象
    Vector<int> myVector;

    int N; // 第一行为正整数 N, 代表后续有 N 行命令序列
    std::cin >> N;
    getchar(); // 读走缓冲区中的回车符

    std::string line;
    for (int i = 0; i < N; i++)
    {
        getline(std::cin, line); // 读取一行

        std::istringstream iss(line);
        std::string command;
        iss >> command; // 得到当前行的命令

        if (command == "push")
        {
            int value;
            iss >> value;
            myVector.push_back(value);
        }
        else if (command == "size")
        {
            std::cout << myVector.getSize() << std::endl;
        }
        else if (command == "get")
        {
            int index;
            iss >> index;
            if ((index >= myVector.getSize()) || (index < 0))
                return -1;
            int value;
            std::cout << myVector[index] << std::endl;
        }
        else if (command == "insert")
        {
            int index;
            int value;
            iss >> index >> value;
            myVector.insert(index, value);
        }
        else if (command == "pop")
        {
            myVector.pop_back();
        }
        else if (command == "print")
        {
            if (myVector.getSize() == 0)
            {
                std::cout << "empty" << std::endl;
                continue;
            }
            // 这里调用打印函数（索引）
            myVector.printElements();
        }
        else if (command == "iterator")
        {
            if (myVector.getSize() == 0)
            {
                std::cout << "empty" << std::endl;
                continue;
            }
            // 这里采用迭代器（实际上也是指针）打印数组
            for (auto it = myVector.begin(); it != myVector.end(); it++)
            {
                std::cout << *it << " ";
            }
            std::cout << std::endl;
        }
        else if (command == "foreach")
        {
            if (myVector.getSize() == 0)
            {
                std::cout << "empty" << std::endl;
                continue;
            }

            /* 这里采用for增强循环打印数组
            for增强循环的底层机制：
            {
                auto && __range = myVector;                  // 获取容器引用
                auto __begin = __range.begin();              // 调用 begin()
                auto __end = __range.end();                  // 调用 end()
                for (; __begin != __end; ++__begin) {
                const auto &element = *__begin;             // 解引用迭代器
                std::cout << element << " ";                // 使用元素
            }

            */

            // for增强循环的底层机制主要使用begin（） & end()，因为这里使用const对象，因此对应使用的是const版begin() 和 end()
            for (const auto &element : myVector)
            {
                std::cout << element << std::endl;
            }
        }
        else if (command == "clear")
        {
            myVector.clear();
        }
    }
}