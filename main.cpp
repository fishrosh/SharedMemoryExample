#include "tests.h"
#include <iostream>

int main(int count, char* args[])
{
    //::tests::singleStatic();
    // ::tests::multiStatic();

    for (auto i = 0ull; i < count; ++i)
        std::cout << args[i] << std::endl;

    if (count > 1) tests::example(true);
    else tests::example();
}