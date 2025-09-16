#include <iostream>
#include "KLruCache.h"

using namespace PerCache;

int main()
{
    // 创建一个容量为2的LRU缓存
    KLruCache<int, std::string> cache(2);

    // 测试1：插入和查询
    cache.put(1, "One");
    cache.put(2, "Two");

    std::string value;
    if (cache.get(1, value))
    {
        std::cout << "Key 1: " << value << std::endl; // 应输出 "One"
    }

    if (cache.get(2, value))
    {
        std::cout << "Key 2: " << value << std::endl; // 应输出 "Two"
    }

    // 测试2：触发LRU淘汰（插入第三个元素，应淘汰最久未使用的键1）
    cache.put(3, "Three");

    if (!cache.get(1, value))
    {
        std::cout << "Key 1 has been evicted." << std::endl; // 应输出
    }

    if (cache.get(2, value))
    {
        std::cout << "Key 2: " << value << std::endl; // 应输出 "Two"
    }

    if (cache.get(3, value))
    {
        std::cout << "Key 3: " << value << std::endl; // 应输出 "Three"
    }

    // 测试3：更新已有键的值
    cache.put(2, "Two Updated");
    if (cache.get(2, value))
    {
        std::cout << "Key 2 updated: " << value << std::endl; // 应输出 "Two Updated"
    }

    // 测试4：使用另一个get重载
    std::string val = cache.get(3);
    std::cout << "Key 3 via get(key): " << val << std::endl; // 应输出 "Three"

    // 测试5：删除键
    cache.remove(2);
    if (!cache.get(2, value))
    {
        std::cout << "Key 2 has been removed." << std::endl; // 应输出
    }

    return 0;
}

/*测试结果
get(1) = 10
get(2) = 20
get(3) = 30
After inserting 4:
get(1) = not found (evicted)
get(4) = 40
After update, get(2) = 22
*/