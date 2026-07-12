#include <string>
#include <unordered_map>

using std::unordered_map, std::string;

int main(void)
{
    unordered_map<string, size_t> map {};

    for (size_t i = 0; i < 1024 * 50; i++) {
        char K[16] = { 0 };
        snprintf(K, sizeof K, "%zu", i << 16);
        string key { K };
        map.insert_or_assign(key, i);
        size_t value = map.at(key);
        printf("[%zu] key: \"%s\", value: %zu\n", i, key.c_str(), value);
        map.erase(key);
    }

    return 0;
}
