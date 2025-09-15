#include <iostream>
#include "KLruCache.h"

using namespace std;
using PerCache::KLruCache;

int main()
{
    // 创建容量为3的KLruCache
    KLruCache<int, int> cache(3);

    // 插入三个元素
    int v = 0;
    cache.put(1, v = 10);
    cache.put(2, v = 20);
    cache.put(3, v = 30);

    // 打印获取元素
    int value;
    if (cache.get(1, value))
        cout << "get(1) = " << value << endl; // 访问1，变成最近使用
    if (cache.get(2, value))
        cout << "get(2) = " << value << endl;
    if (cache.get(3, value))
        cout << "get(3) = " << value << endl;

    // 插入新元素，触发驱逐最少访问（这里会驱逐 key=1）
    cache.put(4, v = 40);
    cout << "After inserting 4:" << endl;
    if (cache.get(1, value))
        cout << "get(1) = " << value << endl;
    else
        cout << "get(1) = not found (evicted)" << endl;

    if (cache.get(4, value))
        cout << "get(4) = " << value << endl;

    // 更新已有元素
    cache.put(2, v = 22);
    if (cache.get(2, value))
        cout << "After update, get(2) = " << value << endl;

    return 0;
}

/*测试结果：
get(1) = 10
get(2) = 20
get(3) = 30
After inserting 4:
get(1) = not found (evicted)
get(4) = 40
After update, get(2) = 22
*/
