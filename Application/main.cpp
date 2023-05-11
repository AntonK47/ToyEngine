//#define _CRTDBG_MAP_ALLOC
//
//#include <Crtdbg.h>
#include <Application.h>

int main()
{
    
    const auto result = Application::run();
    //_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    //_CrtMemDumpAllObjectsSince(NULL);

    return result;
}
