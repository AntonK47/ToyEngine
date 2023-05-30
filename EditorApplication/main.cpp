
#include <EditorApplication.h>
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
    const auto result = toy::editor::EditorApplication::run(arguments);

    return result;
}
