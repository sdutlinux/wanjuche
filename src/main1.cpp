#include "master.h"

#include <iostream>

int main(int argc, char * argv[])
{
    Master master(2);
    std::cout << "----Slighttpd----" << std::endl;
    if (!master.start_master())
        return -1;
    std::cout << "----Goodbye----" << std::endl;
    return 0;
}
