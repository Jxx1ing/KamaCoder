#include <iostream>
#include <string>
#include "KLruCache.h" // 假设你的头文件名为 KLruKCache.h

using namespace std;
using namespace PerCache;

int main()
{
    // 测试 KHashLruCaches
    cout << "Testing KHashLruCaches..." << endl;

    // 创建一个总容量为100，分片数为4的缓存
    KHashLruCaches<int, string> cache(100, 4);

    // 测试 put 和 get
    cache.put(1, "Apple");
    cache.put(2, "Banana");
    cache.put(3, "Cherry");

    string value;
    if (cache.get(1, value))
    {
        cout << "Key 1: " << value << endl; // 应该输出 "Apple"
    }
    else
    {
        cout << "Key 1 not found" << endl;
    }

    if (cache.get(2, value))
    {
        cout << "Key 2: " << value << endl; // 应该输出 "Banana"
    }
    else
    {
        cout << "Key 2 not found" << endl;
    }

    if (cache.get(3, value))
    {
        cout << "Key 3: " << value << endl; // 应该输出 "Cherry"
    }
    else
    {
        cout << "Key 3 not found" << endl;
    }

    // 测试不存在的键
    if (cache.get(4, value))
    {
        cout << "Key 4: " << value << endl;
    }
    else
    {
        cout << "Key 4 not found (as expected)" << endl;
    }

    // 测试容量淘汰
    // 添加更多数据，超过容量限制
    for (int i = 5; i < 150; ++i)
    {
        cache.put(i, "Value" + to_string(i));
    }

    // 检查一些较早的键是否被淘汰
    if (cache.get(1, value))
    {
        cout << "Key 1 still exists: " << value << endl;
    }
    else
    {
        cout << "Key 1 was evicted (as expected)" << endl;
    }

    // 检查一些较新的键是否存在
    if (cache.get(149, value))
    {
        cout << "Key 149 exists: " << value << endl;
    }
    else
    {
        cout << "Key 149 not found" << endl;
    }

    cout << "KHashLruCaches test completed." << endl;

    return 0;
}

/*测试结果
Testing KHashLruCaches...
Key 1: Apple
Key 2: Banana
Key 3: Cherry
Key 4 not found (as expected)
Key 1 was evicted (as expected)
Key 149 exists: Value149
KHashLruCaches test completed.
*/