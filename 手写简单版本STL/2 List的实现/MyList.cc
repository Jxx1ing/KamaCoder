#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

template <typename T>
class List
{
private:
    // 一个节点
    struct Node
    {
        T data;     // 数据
        Node *prev; // 指向前一个节点的指针
        Node *next; // 指向后一个节点的指针

        // 节点构造函数(带参数)     成员初始化列表
        Node(const T &value, Node *prevNode = nullptr, Node *nextNode = nullptr)
            : data(value),
              prev(prevNode),
              next(nextNode)
        {
        }
    };

    // list的变量
    Node *head;  // 头节点指针
    Node *tail;  // 尾节点指针
    size_t size; // 链表中节点的数量

public:
    // 允许这个运算符函数访问 List 的私有成员（这个函数需要访问 pt.head，而 head 是 private 成员。 在类外定义的函数，不能直接访问类的私有成员。）
    /*
     重载<<一般不写成成员函数，而是写成 非成员函数（普通函数）
        List<int> lst;
        lst.operator<<(std::cout);   // ✅ 可以
        lst << std::cout;            // ❌ 不对
    👉 注意：成员运算符函数的左操作数必须是这个类对象。
    也就是说：
        lst << std::cout; ✅
        std::cout << lst; ❌
    但我们平常希望的写法是 std::cout << lst，因为 << 的左边更自然地应该是输出流。
     */
    template <typename L>
    friend std::ostream &operator<<(std::ostream &os, const List<L> &pt);
    /*
    默认的 << 只认识基本类型（int, double, string 等），我们重新定义它，让它也能处理 List<T>。
    两个参数：std::ostream &os, const List<T> &pt)
        左边的流对象：比如 std::cout。传引用是为了避免复制整个流对象（流是不能拷贝的）。
        右边的 List 对象：我们要打印的链表，传 const & 是为了避免拷贝整个链表，又保证不修改它。
    返回值是一个输出流类型的对象
    */

public:
    /*1-基础成员函数*/
    // 构造函数
    List() : head(nullptr),
             tail(nullptr),
             size(0)
    {
    }

    // 析构函数
    ~List()
    {
        clear();
    }

    /*2-核心功能*/
    // 在链表末尾添加元素
    void push_back(const T &value)
    {
        // 创建新节点
        Node *newNode = new Node(value, tail, nullptr);
        if (tail)
        {
            // 如果tail != NULL, 两个节点之间构建双向关系
            tail->next = newNode;
            // 更新尾节点
            tail = newNode;
        }
        else
        {
            // 说明tail == NULL, 这是一个空链表。那么新增的节点既是头节点，也是尾节点
            head = newNode;
            // 更新尾节点
            tail = newNode;
        }
        size++;
    }

    // 在链表开头添加元素
    void push_front(const T &value)
    {
        // 创建新节点
        Node *newNode = new Node(value, nullptr, head);
        if (head)
        {
            // 如果head != NULL, 两个节点之间构建双向关系
            head->prev = newNode;
            // 更新头节点
            head = newNode;
        }
        else
        {
            // 说明head == NULL, 这是一个空链表。那么新增的节点既是头节点，也是尾节点
            tail = newNode;
            // 更新头节点
            head = newNode;
        }
        size++;
    }

    void pop_back()
    {
        if (size > 0)
        {
            // 获取尾节点的前一个节点
            Node *newTail = tail->prev;
            // 删除最后一个节点
            delete tail;
            tail = newTail;
            // 如果链表不为空
            if (tail)
            {
                tail->next = nullptr;
            }
            else
            {
                // 运行到这里，说明链表为空
                head = nullptr;
                /*
                如果head不置nullptr，则会造成悬空指针。
                因为只有一个节点，删除了链表为空，此时tail指向nullptr,但是head指向已经delete的（那一个节点）内存
                */
            }
            size--;
        }
    }

    // 删除链表开头的元素
    void pop_front()
    {
        if (size > 0)
        {
            // 获取头节点的后一个节点
            Node *newHead = head->next;
            // 删除第一个节点
            delete head;
            head = newHead;
            // 如果链表不为空
            if (head)
            {
                head->prev = nullptr;
            }
            else
            {
                // 运行到这里，说明链表为空
                tail = nullptr;
            }
            size--;
        }
    }

    // 删除指定值的节点
    void remove(const T &val)
    {
        Node *node = head;
        while (node != nullptr && node->data != val)
        {
            node = node->next;
        }
        // 运行到这里有两种情况：（1）node->data = val   (2)node == nullptr 没有在List中找到val值
        if (node == nullptr)
            return; // 情况（2）—— 没有找到
        // 情况一 node != nullptr分为3种情况
        // A-node是中间节点，即不是头节点也不是尾节点
        if (node != head && node != tail)
        {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }
        // B-node是头节点
        else if (node == head)
        {
            head = node->next;
            head->prev = nullptr;
        }
        // C-node是尾节点
        else if (node == tail)
        {
            tail = tail->prev;
            tail->next = nullptr;
        }
        // D-如果只有一个节点，即既是头节点也是尾节点，则删除节点后链表置空
        else if (node == head && node == tail)
        {
            head = nullptr;
            tail = nullptr;
        }

        /*
        分为3种情况的原因是： 删除一个节点，涉及3个节点，即要删除的节点，它前一个节点，它后一个节点
        * 比如我要删除第一个节点(head)，如果都用情况A代替，会出现
        node->prev = nullptr,因此node->prev->next 无意义
        * 同理，如果我要删除尾节点（tail）,如果都用情况B代替，会出现
        node->next = nullptr, 因此node->next->prev 无意义

        所以要单独讨论删除的节点是头节点或者尾节点的情况，也就是B/C
        此外，还要考虑如果只有一个节点，即既是头节点也是尾节点，则删除节点后链表置空
        */

        size--;
        delete node;
        node == nullptr; // 防止野指针
    }

    // 获取链表中节点的数量
    size_t getSize() const
    {
        return size;
    }

    /*3-迭代与遍历*/
    // 打印链表中的元素（这里注意：非const对象也可以调用const成员函数  删掉const也可以）
    void printElements() const
    {
        for (Node *current = head; current != nullptr; current = current->next)
        {
            std::cout << current->data << " ";
        }
        std::cout << std::endl;
    }

    /*4-辅助功能*/
    // 重载[]运算符以对链表进行索引访问   operator[]
    T &operator[](size_t index)
    {
        // 从头结点开始遍历，找到第index个节点
        Node *current = head;
        for (size_t i = 0; i < index; i++)
        {
            if (!current)
            {
                throw std::out_of_range("index out of range");
            }
            current = current->next;
        }
        // 返回节点中的数
        return current->data;
    }
    // 重载<<运算符以打印链表            operator<<    -->普通函数形式写在类外

    /*5-一些额外实现的函数功能*/
    // 获得指定值的节点
    Node *getNode(const T &val)
    {
        Node *node = head;
        while (node != nullptr && node->data != val)
        {
            node = node->next;
        }
        // 运行到这里有两种情况：（1）node->data = val   (2)node == nullptr 没有在List中找到val值
        return node;
    }
    // 返回链表中数据的指针（调用者拿到这个指针，可以直接修改链表内的数据）
    T *find(const T &val)
    {
        Node *node = getNode(val); // node->data 是节点内的数据
        if (node == nullptr)
        {
            // 如果没有找到对应的值，返回nullptr
            return nullptr;
        }
        // 运行到这里，说明找到val值
        return &node->data;
    }

    // 清空链表
    void clear()
    {
        while (head)
        {
            // 从头节点开始，依次删除节点
            Node *temp = head;
            head = head->next;
            delete temp;
            temp = nullptr; // 防止野指针
        }
        // 运行到这里，说明链表为空
        // 更新尾节点指针 和 链表大小
        tail = nullptr;
        size = 0;
    }

    // List的迭代器
    Node *begin()
    {
        return head;
    }

    Node *end()
    {
        return nullptr;
    }

    // List的迭代器（const版本）
    const Node *begin() const
    {
        return head;
    }

    const Node *end() const
    {
        return nullptr;
    }
};

// 普通函数——重载<<运算符
template <typename T>
std::ostream &operator<<(std::ostream &os, const List<T> &pt)
{
    /*在模板类中，如果你访问依赖模板参数的嵌套类型，必须写 typename
    如果去掉：error: need 'typename' before 'List<T>::Node' because 'List<T>' is a dependent scope
    模板编译器在第一次解析模板时 无法确定 Node 是类型还是静态成员，为了安全必须显式加上 typename。
    */

    /*
    可不可以直接写成 Node* current 而不加 typename List<T>::Node*？
    这里 Node 是 List<T> 的嵌套类型
    因为 List 是模板，Node 依赖模板参数 T，所以编译器 无法确定 Node 是类型还是成员变量
    所以必须写：typename List<T>::Node* current
    否则编译器根本不知道你指的是什么，会报错：error: ‘Node’ was not declared in this scope
但是在在 List<T> 的 成员函数 里面写：Node* current = head;（因为Node 是类的 嵌套类型， 成员函数里面默认可以访问类的内部类型）
    */
    for (typename List<T>::Node *current = pt.head; current != nullptr; current = current->next)
    {
        os << current->data << " ";
    }
    os << std::endl;
    return os;
}

int main()
{
    // 创建一个 List 对象
    List<int> myList;

    int N;
    std::cin >> N;
    // 读走回车
    getchar();
    std::string line;
    // 接收命令
    for (int i = 0; i < N; i++)
    {
        std::getline(std::cin, line);
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        int value;

        if (command == "push_back")
        {
            iss >> value;
            myList.push_back(value);
        }

        if (command == "push_front")
        {
            iss >> value;
            myList.push_front(value);
        }

        if (command == "pop_back")
        {
            myList.pop_back();
        }

        if (command == "pop_front")
        {
            myList.pop_front();
        }

        if (command == "remove")
        {
            iss >> value;
            myList.remove(value);
        }

        if (command == "clear")
        {
            myList.clear();
        }

        if (command == "size")
        {
            std::cout << myList.getSize() << std::endl;
        }

        if (command == "get")
        {
            iss >> value;
            std::cout << myList[value] << std::endl;
        }

        if (command == "print")
        {
            if (myList.getSize() == 0)
            {
                std::cout << "empty" << std::endl;
            }
            else
            {
                myList.printElements();
            }
        }
    }
    return 0;
}