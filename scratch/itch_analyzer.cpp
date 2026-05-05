#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <cstdint>

#if defined(_MSC_VER)
#define BSWAP16(x) _byteswap_ushort(x)
#else
#define BSWAP16(x) __builtin_bswap16(x)
#endif

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    std::ifstream file(argv[1], std::ios::binary);
    if (!file) return 1;

    std::map<char, int> counts;
    uint16_t msg_len;
    while (file.read(reinterpret_cast<char*>(&msg_len), 2)) {
        msg_len = BSWAP16(msg_len);
        std::vector<uint8_t> buffer(msg_len);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), msg_len)) break;
        
        char type = buffer[0];
        counts[type]++;
    }

    std::cout << "--- ITCH File Analysis ---" << std::endl;
    for (auto const& [type, count] : counts) {
        std::cout << "Msg '" << type << "': " << count << "\n";
    }
    return 0;
}
