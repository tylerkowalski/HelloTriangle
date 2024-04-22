#include "app.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main()
{
    ht::App myApp{};

    try
    {
        myApp.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}