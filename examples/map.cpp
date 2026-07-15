#include <cstddef>
#include <string>
#include <unordered_map>

using std::unordered_map, std::string;

int main(void)
{
    unordered_map<string, size_t> map {};

    for (size_t i = 0; i < 1024 * 100; i++) {
        char K[16] = { 0 };
        snprintf(K, sizeof K, "%zu", i << 8);

        string key { K };
        map.insert_or_assign(key, i);

        (void)map.at(key);
        // ::printf("[%zu] key: \"%s\", value: %zu\n", i, key.c_str(), value);
    }

    char K[16] = { 0 };
    snprintf(K, sizeof K, "%llu", ((1024ULL * 100ULL) - 1ULL) << 8);
    string key { K };
    (void)map.at(key);

    return 0;
}
