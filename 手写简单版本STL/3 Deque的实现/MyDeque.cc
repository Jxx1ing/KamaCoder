#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>

template <typename T>
class Deque
{
private:
    T *elements;       // 循环一维数组
    size_t capacity;   // 数组的总容量
    size_t frontIndex; // deque的前端索引
    size_t backIndex;  // deque的后端索引(指向的位置是当前末尾元素的下一个位置, 也就是还没有有效的数据(如果容量满了, 其会指)
    size_t size;       // 数组长度(已使用的容量)

public:
    // 1、基础成员函数
    // 构造函数
    Deque() : elements(nullptr),
              capacity(0),
              frontIndex(0),
              backIndex(0),
              size(0)
    {
    }

    // 析构函数
    ~Deque()
    {
        clear();
        delete[] elements;
    }

    // 2、核心功能
    // 在 Deque 末尾添加元素
    void push_back(const T &value)
    {
        // 检查是否需要扩张数组容量
        if (size == capacity)
        {
            resize();
        }

        // 在当前后端索引插入新的元素
        elements[backIndex] = value;
        // 计算新的后端索引
        backIndex = (backIndex + 1) % capacity;
        // 增加deque的元素数量
        size++;
    }

    // 在 Deque 开头添加元素
    void push_front(const T &value)
    {
        // 检查是否需要扩张数组容量
        if (size == capacity)
        {
            resize();
        }

        // 计算新的前端索引
        frontIndex = (frontIndex - 1 + capacity) % capacity; // 防止新的前端索引为负数。比如当前frontIndex = 0,不可能新的索引值为-1。应该是逻辑上的最后一个索引
        // 插入新的元素
        elements[frontIndex] = value;
        // 增加deque的元素数量
        size++;
    }

    // 删除 Deque 末尾的元素
    void pop_back()
    {
        // 检查deque是否为空
        if (0 == size)
        {
            throw std::out_of_range("Deque is empty");
        }
        // 注意：这里删除元素不需要显示删除，以后新追加元素会自动覆盖
        backIndex = (backIndex - 1 + capacity) % capacity; // 防止新的后端索引为负数。比如当前backIndex = 0, 不可能新的索引值为 - 1。应该是逻辑上的最后一个索引
        // 减少deque的元素数量
        size--;
    }

    // 删除 Deque 开头的元素
    void pop_front()
    {
        // 检查deque是否为空
        if (0 == size)
        {
            throw std::out_of_range("Deque is empty");
        }
        // 注意：这里删除元素不需要显示删除，以后新追加元素会自动覆盖
        frontIndex = (frontIndex + 1) % capacity;
        // 减少deque的元素数量
        size--;
    }

    // 获取 Deque 中节点的数量
    size_t getSize() const
    {
        return size;
    }

    // 删除 Deque 中所有的元素
    void clear()
    {
        while (size > 0)
        {
            pop_front();
        }
    }

    // 3、迭代与遍历
    // 打印队列
    void printElements() const
    {
        size_t index = frontIndex;
        for (size_t i = 0; i < size; i++)
        {
            std::cout << elements[index] << " ";
            index = (index + 1) % capacity;
        }
        std::cout << std::endl;
    }

    // 4、辅助功能（重载[]运算符以对 Deque 进行索引访问）
    T &operator[](int index)
    {
        if (index < 0 || index >= size)
        {
            throw std::out_of_range("index out of range");
        }
        return elements[(frontIndex + index) % capacity]; // index指的是当前前端索引开始（相当于0），往后index个位置（包含当前前端索引）
    }

private:
    // 扩展数组容量（这里以扩容一倍为例）
    void resize()
    {
        // 计算新的容量
        size_t newCapacity = (capacity == 0) ? 1 : capacity * 2;

        // 深拷贝
        T *newElements = new T[newCapacity];
        size_t index = frontIndex;
        for (size_t i = 0; i < size; i++)
        {
            newElements[i] = elements[index]; // 不管原来的队列前端在哪里，按顺序复制到新循环数组中（新数组中从索引0开始）
            index = (index + 1) % capacity;
        }

        // 释放旧内存
        delete[] elements;

        // 更新
        elements = newElements;
        capacity = newCapacity;
        frontIndex = 0;
        backIndex = size;
    }
};

int main()
{
    // 创建一个 Deque 对象
    Deque<int> myDeque;

    int N;
    std::cin >> N;
    // 读走回车
    getchar();
    std::string line;
    // 接收命令
    for (int i = 0; i < N; i++)
    {
        std::getline(std::cin, line); // 读取一整行输入
        std::istringstream iss(line); // 创建一个输入字符串流来解析这行内容
        std::string command;          // 存储命令
        iss >> command;               // 解析第一个单词作为命令
        int value;                    // 存储接下来的整数值

        if (command == "push_back")
        {
            iss >> value;
            myDeque.push_back(value);
        }

        if (command == "push_front")
        {
            iss >> value;
            myDeque.push_front(value);
        }

        if (command == "pop_back")
        {
            if (myDeque.getSize() == 0)
            {
                continue;
            }
            myDeque.pop_back();
        }

        if (command == "pop_front")
        {
            if (myDeque.getSize() == 0)
            {
                continue;
            }
            myDeque.pop_front();
        }

        if (command == "clear")
        {
            myDeque.clear();
        }

        if (command == "size")
        {
            std::cout << myDeque.getSize() << std::endl;
        }

        if (command == "get")
        {
            iss >> value;
            std::cout << myDeque[value] << std::endl;
        }

        if (command == "print")
        {
            if (myDeque.getSize() == 0)
            {
                std::cout << "empty" << std::endl;
            }
            else
            {
                myDeque.printElements();
            }
        }
    }
    return 0;
}
