#include <iostream>
#include <string>

#include "/util/function/router.h"


void add(int x, int y, int* z) {
    *z = x + y;
}

void inc(int* y) {
    *y += 1;
}

int main() {

    util::router<std::string>::get().register_handler("add", add);
    util::router<std::string>::get().register_handler("inc", inc);
    int x = 1;
    int y = 2;
    int z;

    util::router<std::string>::get().route("add", 100, 200, &z);
    util::router<std::string>::get().route("inc", &z);

    std::cout <<":" << z << std::endl; 
}
