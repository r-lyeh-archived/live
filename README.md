live
====

- Live is an automatic reloader of constants during runtime, featuring type inference. Written in C++11.
- Live is cross-platform. Builds on Windows/Linux/MacosX. Compiles on g++/clang/msvc.
- Live is self-contained. No third party dependencies.
- Live is tiny. Header only.
- BOOST licensed.

### intro
- On debug, `$live(...)` constants are watched, parsed, evaluated, cached, and returned back as needed.
- On release, `$live(...)` constants are just returned back with no modifications.

### sample
```c++
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
```

### cons
- Live requires strict ordering of `$live()` elements during runtime.
