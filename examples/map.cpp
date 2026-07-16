#include <cstddef>
#include <stdexcept>
#include <string>
#include <unordered_map>

using std::unordered_map, std::string;

int main(void)
{
    unordered_map<string, size_t> map {};

    for (size_t i = 0; i < 1024 * 1024 * 4; i++) {
        char K[16] = { 0 };
        snprintf(K, sizeof K, "%zu", i << 2);

        string key { K };
        map.insert_or_assign(key, i);
    }

    char K[16] = { 0 };
    snprintf(K, sizeof K, "%llu", ((1024ULL * 698ULL) - 1ULL) << 2);
    string key { K };
    map.erase(key);

    try {
        map.at(key);
    } catch (std::out_of_range err) {
        printf("Deletion success\n");
    }

    return 0;
}
