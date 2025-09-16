#include <iostream>
#include "KLruCache.h"

using namespace PerCache;
using namespace std;

int main()
{
    // 创建一个 KLruKCache 实例：主缓存容量为2，_historyCounter缓存容量（KLruCache类型，_historyCounter指针指向的对象）为3，k=2
    KLruKCache<int, int> cache(2, 3, 2);

    // 测试1：插入数据，但未达到k次访问，应保留在历史缓存中
    cache.put(1, 100);
    cache.put(2, 200);

    // 此时主缓存中应无数据（因为尚未达到k次访问）
    cout << "Key 1 in main cache? " << (cache.get(1) == 100 ? "Yes" : "No") << endl; // 应输出 No（实际返回0，因为未命中）
    cout << "Key 2 in main cache? " << (cache.get(2) == 200 ? "Yes" : "No") << endl; // 应输出 No

    // 测试2：对key=1再次访问，使其达到k=2次，应晋升到主缓存
    cout << "Get key=1 again: " << cache.get(1) << endl;                                 // 应返回100（实际会触发晋升）
    cout << "Now key=1 in main cache? " << (cache.get(1) == 100 ? "Yes" : "No") << endl; // 应输出 Yes

    // 测试3：插入新数据，触发主缓存淘汰（LRU策略）
    cache.put(3, 300);
    cache.put(4, 400);

    // 此时key=2应已被淘汰（_historyCounter缓存中可能还有，但主缓存中无）
    cout << "Key 2 in main cache? " << (cache.get(2) == 200 ? "Yes" : "No") << endl; // 应输出 No

    // 测试4：key=3和4应在主缓存中(前面put()1次，这里get()1次)
    cout << "Key 3 in main cache? " << (cache.get(3) == 300 ? "Yes" : "No") << endl; // 应输出 Yes
    cout << "Key 4 in main cache? " << (cache.get(4) == 400 ? "Yes" : "No") << endl; // 应输出 Yes

    return 0;
}