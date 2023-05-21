//#define _CRTDBG_MAP_ALLOC
//
//#include <Crtdbg.h>
#include <Application.h>
#include <string>
#include <vector>
#include <ranges>

int main(int argc, char* argv[])
{
    auto arguments = std::vector<std::string>{};
    arguments.resize(argc);
    
    for (int i = 0; i < argc; i++)
    {
        arguments[i] = std::string{ argv[i] };
    }
    const auto result = Application::run(arguments);
    //_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    //_CrtMemDumpAllObjectsSince(NULL);

    return result;
}
