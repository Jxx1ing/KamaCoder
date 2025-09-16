#pragma once

#include <mutex>
#include <unordered_map>
#include <memory> //weak_ptr 和 shared_ptr
#include <thread> //KHashLruCaches类的hardware_concurrency
#include <vector>
#include <math.h>     //KHashLruCaches类的ceil函数
#include <functional> ////KHashLruCaches类的hash函数
#include "KICachePolicy.h"
using namespace std;

namespace PerCache
{
    // 前向声明 —— 为了KLruCache 在 LruNode 中声明为友元时可以找到定义
    template <typename Key, typename Value>
    class KLruCache;

    // （1）LruNode类
    template <typename Key, typename Value>
    class LruNode
    {
        /*LruCache中的removeNode/insertNode等函数中需要直接操作LruNode的私有成员_prev、_next
        外部类访问一个类（LruNode）的私有成员，需要(在该类中即LruNode类)把外部类声明为友元
        */
        friend class KLruCache<Key, Value>;

    private:
        Key _key;                              // 键
        Value _value;                          // 值
        weak_ptr<LruNode<Key, Value>> _prev;   //_prev指向双向链表的前一个节点（前驱）
        shared_ptr<LruNode<Key, Value>> _next; //_next指向双向链表的后一个节点（后继）
    public:
        // LruNode类的构造函数
        LruNode(Key key, Value value)
            : _key(key), _value(value)
        {
        }
        Key getKey() const { return _key; }                   // 获取key——加上const表示无法修改对象的成员变量
        void setValue(const Value &value) { _value = value; } // 设置value值
        Value getValue() const { return _value; }             // 获取value值
    };
    // （2）KLruCache类
    template <typename Key, typename Value>
    class KLruCache : public KICachePolicy<Key, Value> // 继承 KICachePolicy类
    {
    public:
        using LruNodeType = LruNode<Key, Value>;     // 节点
        using NodePtr = shared_ptr<LruNodeType>;     // 管理一个节点的指针
        using NodeMap = unordered_map<Key, NodePtr>; // 哈希表，键->指针（即某个节点）

    private:
        int _capacity;      // Lru缓存容量(注意是哈希表而不是双向链表)
        NodeMap _nodeMap;   // Lru哈希表
        mutex _mutex;       // 互斥锁
        NodePtr _dummyHead; // shared_ptr指针管理的哨兵头节点
        NodePtr _dummyTail; // shared_ptr指针管理的哨兵尾节点
    public:
        // KLruCache类的构造函数
        KLruCache(int capacity)
            : _capacity(capacity)
        {
            initializeList(); // 构建双向链表（初始化哨兵头尾节点）
            /*注意：不需要显式初始化：
                std::mutex 是 RAII 类型，声明时已经自动初始化。
                nodeMap_(非指针成员变量会在对象构造时自动调用默认构造函数)自动初始化为空哈希表。
           */
        }

        // put添加缓存(更新哈希表和双向链表)
        void put(Key key, Value value) override
        {
            // 如果容量小于等于0，返回（说明参数错误）
            if (_capacity <= 0)
                return;
            // lock_guard加锁
            lock_guard<mutex> lock(_mutex);
            // 如果查找到key，则更新对应的value
            auto it = _nodeMap.find(key); // it是一个迭代器
            if (it != _nodeMap.end())
            {
                updateExistingNode(it->second, value); // shared_ptr指针，值
                return;
            }

            // 否则（代表没有找到对应的key），直接插入这个节点
            addNewNode(key, value);
            // 离开作用域自动解锁
        }

        // get查询哈希表中是否存在键，并使用输出参数value填充对应的值；若不存在返回fasle
        bool get(Key key, Value &value) override
        {
            lock_guard<mutex> lock(_mutex);
            auto it = _nodeMap.find(key);
            if (it != _nodeMap.end())
            {
                // 查询节点后将该节点移动到最新位置
                moveToMostRecent(it->second);
                // 并把查询结果更新到输出参数value  —— it->second是节点，getValue是调用了LruNode类中的getValue方法
                value = it->second->getValue();
                return true;
            }
            // 否则，如果没有在哈希表中查询到该key,返回false
            return false;
        }

        Value get(Key key) override
        {
            Value value{};
            get(key, value);
            return value; // 如果key不存在，则返回0
        }
        // 删除指定元素（到双向链表和哈希表）
        void remove(Key key)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            auto it = _nodeMap.find(key);
            if (it != _nodeMap.end())
            {
                removeNode(it->second); // 双向链表中删除该节点
                _nodeMap.erase(it);     // 哈希表中删除key-value
            }
        }

    private:
        // 构建双向链表（初始化哨兵头尾节点）
        void initializeList()
        {
            _dummyHead = make_shared<LruNodeType>(Key(), Value());
            _dummyTail = make_shared<LruNodeType>(Key(), Value());
            _dummyHead->_next = _dummyTail;
            _dummyTail->_prev = _dummyHead;
        }

        // 更新节点位置
        void updateExistingNode(NodePtr node, const Value &value)
        {
            // 更新节点的value
            node->setValue(value);
            // 将节点移动到链表尾部（最新位置）
            moveToMostRecent(node);
        }

        // 插入节点(更新哈希表和双向链表)
        void addNewNode(const Key &key, const Value &value)
        {
            // 限制哈希表大小，通过O（1）得到元素数量。（而如果限制双向链表大小，遍历链表需要O（n））
            if (_nodeMap.size() >= _capacity)
            {
                evictLeastRecent(); // 驱逐最少访问，给哈希表留出容量
            }
            NodePtr newNode = make_shared<LruNodeType>(key, value); // 创建节点（这对key-value）
            insertNode(newNode);                                    // 双向链表中插入该节点
            _nodeMap[key] = newNode;                                // 哈希表中插入这个节点
        }

        // 将节点移到最新位置
        void moveToMostRecent(NodePtr node)
        {
            // 移除这个节点
            removeNode(node);
            // 插入这个节点
            insertNode(node);
        }

        //  双向链表中移除节点
        void removeNode(NodePtr node)
        {
            // 如果这个节点的前驱和后继节点都存在
            if (!node->_prev.expired() && node->_next)
            {
                auto prev = node->_prev.lock(); // weak_ptr -> shared_ptr
                prev->_next = node->_next;
                node->_next->_prev = prev;
                node->_next = nullptr;
                /*
                    node->_next = nullptr的目的是把这个节点与链表断开
                    看起来node->_prev没有断开（即node->_prev 没有手动置空），会不会导致内存泄露？
                    实际上，因为没有其他节点指向node, 因此node的强引用计数为1（make_shared创建节点引用计数就是1）
                    当node离开作用域时，节点析构造,所以node->_prev（即一个weak_ptr）也会被销毁
                */
            }
        }

        // 双向链表中插入节点（尾插法）
        void insertNode(NodePtr node)
        {
            auto prev = _dummyTail->_prev; // weak_ptr
            prev.lock()->_next = node;     // 注意：不要写成prev->_next = node,因为weak_ptr不存在operator->。 正确方式是shared_ptr->_next =  shared_ptr
            node->_next = _dummyTail;
            _dummyTail->_prev = node;
            node->_prev = prev;
        }

        // 驱逐最少访问（删除哈希表key和删除链表头节点）
        void evictLeastRecent()
        {
            auto RealHead = _dummyHead->_next;
            removeNode(RealHead);               // 在链表中删除头节点
            _nodeMap.erase(RealHead->getKey()); // 在哈希表中删除key-value(erase)
        }
    };
    // （3）KLruKCache类
    template <typename Key, typename Value>
    class KLruKCache : public KLruCache<Key, Value> // 继承的是KLruCache类
    {
    private:
        int _k;                                             // 访问次数
        unique_ptr<KLruCache<Key, size_t>> _historyCounter; // 指针。指向一个LRU缓存对象（由双向链表和哈希表构成）：一个计数器缓存，保存所有的Key -> 访问次数(注意如果key已经放在主缓存中，计数器不再计算key的访问次数)
        unordered_map<Key, Value> _historyValueMap;         // 哈希表。保存那些访问次数 < k_ 的 Key 对应的真实 Value（如果访问达k次，需要在哈希表中删除这对key-value）
        /*
            主缓存是KLruCache(因为KLruKCache构造时候，先构造出基类。即KLruKCache 本身有一个 KLruCache<Key, Value>缓存)，通过基类的get()/put()存取数据
            历史缓存是哈希表（因为value要么存在于主缓存，要么存在于哈希表中），但是哈希表需要配合KLruCache实例_historyCache一起使用。
        */
    public:
        // KLruKCache构造函数——还调用KLruCache基类构造
        KLruKCache(int capacity, int historyCapacity, int k)
            : KLruCache<Key, Value>(capacity), _k(k), _historyCounter(make_unique<KLruCache<Key, size_t>>(historyCapacity))
        {
        }
        /*
        make_unique<KLruCache<Key, Value>>(historyCapacity)
            等价于 unique_ptr<KLruCache<Key, Value>>(new KLruCache<Key, Value>(historyCapacity));
        historyList 是一个 unique_ptr。它指向一个动态分配的 KLruCache<Key, Value> 对象。创建的时候把 historyCapacity 传给 KLruCache 的构造函数。
        */

        // 查找哈希表_historyValueMap的key，返回对应的value
        Value get(Key key)
        {
            // 首先尝试从主缓存LruCache中获取数据
            Value value{};
            bool inMainCache = KLruCache<Key, Value>::get(key, value);
            // 如果在主缓存中
            if (inMainCache)
            {
                return value;
            }
            // 运行到这里说明数据不在主缓存
            // 获取并更新历史访问计数（put根据LRU算法放入历史缓存_historyCache）
            size_t historyCount = _historyCounter->get(key); // 注意KLruCache的get方法实现中，如果key不存在，返回0
            historyCount++;
            _historyCounter->put(key, historyCount);

            // 假设访问次数达到了k次
            if (historyCount >= _k)
            {
                // 观察历史缓存中（哈希表）是否存在该历史值key-value
                auto it = _historyValueMap.find(key);
                if (it != _historyValueMap.end())
                {
                    // 如果该历史值key-value存在,把这对key-value放入主缓存
                    Value storedValue = it->second;
                    KLruCache<Key, Value>::put(key, storedValue);
                    // 删除哈希表_historyValueMap中的历史值key-value
                    _historyValueMap.erase(it); // 这里通过迭代器删除，也可以写成.erase(key)
                    // 删除KLruCache类型的historyCounter中的 双向链表（节点） 和 哈希表（key-value）
                    _historyCounter->remove(key);
                    // 添加到主缓存
                    KLruCache<Key, Value>::put(key, storedValue);
                    // 找到数据 → 返回主缓存或历史缓存中的真实值value
                    return storedValue;
                }
            }
            // 运行到这里，说明主缓存或历史缓存中都找不到数据 → 返回 Value{} 默认值
            /*
                这里实际上有歧义。如果Value是int类型，默认值是0
                那么，无法区分缓存命中时可能返回值就是0 or 缓存未命中时默认值0
            */
            return value;
        }
        // 添加缓存(到主缓存或者历史缓存)
        void put(Key key, Value value)
        {
            /*
            主缓存是KLruCache类型因此调用put函数有容量限制,历史缓存是哈希表类型没有容量限制（注意historyCapacity是传入historyCounter的参数，KLruCache类型）
            */
            // 检查是否已在主缓存
            Value existingvalue{};
            bool inMainCache = KLruCache<Key, Value>::get(key, existingvalue);
            // 如果在主缓存中
            if (inMainCache)
            {
                // 更新key-value
                KLruCache<Key, Value>::put(key, value);
                return;
            }
            // 运行到这里，说明不存在主缓存中
            // 保存值到历史缓存哈希表中，供后续get操作使用
            _historyValueMap[key] = value;
            // 获取并更新历史访问计数（put根据LRU算法放入历史缓存_historyCache）
            size_t historyCount = _historyCounter->get(key); // 注意KLruCache的get方法实现中，如果key不存在，返回0
            historyCount++;
            _historyCounter->put(key, historyCount);
            // 如果访问次数大于等于k次,从历史缓存总删除，更新到主缓存
            if (historyCount >= _k)
            {
                _historyCounter->remove(key);
                _historyValueMap.erase(key);
                KLruCache<Key, Value>::put(key, value);
            }
        }
    };
    // （4）KLruKCaches类
    template <typename Key, typename Value>
    class KHashLruCaches // 注意KLruKCaches未继承任何类
    {
    private:
        size_t _capacity;                                          // 缓存总容量
        int _sliceNum;                                             // 分片数量
        vector<unique_ptr<KLruCache<Key, Value>>> _lruSliceCaches; // 分片缓存(是一个向量，元素是unique_ptr指针，每个指针指向一个KLruCache类型的缓存)
    public:
        // KHashLruCaches类的构造函数
        KHashLruCaches(size_t capacity, int sliceNum)
            : _capacity(capacity), _sliceNum(sliceNum > 0 ? sliceNum : std::thread::hardware_concurrency()) // 如果slicenum<0使用CPU核心数
        /*
            注意不可以写成hardware_concurrency()（尽管using namespace std 以及#include<thread>。这里需要显式写出来std::thread::
        */
        {
            // 获取每个分片的大小 总大小/分片数 -> 向上取整
            size_t sliceSize = ceil(_capacity / static_cast<double>(_sliceNum)); // static_cast将int类型转换为double类型：因为整数/整数会舍弃小数部分
            // 创建sliceSize个分片（每个分片的类型都是KLruCache)
            for (int i = 0; i < sliceSize; i++)
            {
                _lruSliceCaches.emplace_back(new KLruCache<Key, Value>(sliceSize));
                /*
                如果不使用new,换一种写法：lruSliceCaches_.emplace_back(make_unique<KLruCache<Key, Value>>(sliceSize));
                */
            }
        }
        // put——把key-value放入缓存中
        void put(Key key, Value value)
        {
            size_t index = Hash(key) % _sliceNum; // 获取key的hash值，并计算出对应的分片索引
            _lruSliceCaches[index]->put(key, value);
        }

        // get——key是否存在
        bool get(Key key, Value &value)
        {
            size_t index = Hash(key) % _sliceNum;
            return _lruSliceCaches[index]->get(key, value);
        }

        // get——获取value
        Value get(Key key)
        {
            Value value{};
            get(key, value); // 调用的是KHashLruCaches类的get(key, value)方法
            return value;
        }

    private:
        // 将key转化为对应的哈希值
        size_t Hash(Key key)
        {
            hash<Key> hashFunc;
            return hashFunc(key);
        }
    };
}
