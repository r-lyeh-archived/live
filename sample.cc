#include <iostream>
#include <string>
#include "live.hpp"

int main() {
    // feel free to modify following constants during runtime
    for(;;) {
        int number = $live(-1234);
        double real = $live(3.14159);
        const char *string = $live("hello world");
        std::string string2 = $live("abcdef");
        std::cout << number << ',' << real << ',' << string << ',' << string2 << std::endl;
    }
}
