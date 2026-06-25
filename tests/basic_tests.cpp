#include <iostream>

int main() {
    if (2 + 2 != 4) {
        std::cerr << "Sanity check failed\n";
        return 1;
    }

    return 0;
}
