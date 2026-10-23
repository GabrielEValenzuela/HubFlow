#include "bootstrap/application.hpp"

#include <exception>
#include <iostream>

int main()
{
    int status {1};
    try
    {
        hubflow::bootstrap::Application application {hubflow::platform::Settings::fromEnvironment()};
        status = application.run();
    }
    catch (const std::exception& error)
    {
        std::cerr << "[hubflow] no se pudo iniciar: " << error.what() << '\n';
    }
    return status;
}
