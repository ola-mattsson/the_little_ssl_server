#include <compression.h>
#include <iostream>

void test(int size) {
    static uint32_t CHUNK_ = 19;

    uint32_t left = size, read = 0;
    do {
        uint32_t delta = std::min(CHUNK_, size - read);
        left -= delta;
        read += delta;
        assert(size == read + left);
    } while (left > CHUNK_);

    std::cout << "done\n";
}

int main() {
//    test(1007);
    char raw[] = "<HejHejHejHejHejHejHejHejHejHejHejHejHejHejHej>";
    std::vector<char> zipped;
    def(raw, zipped);
    std::string unzipped = unzip(&zipped[0], 4096);
    std::cout << "\n";
}