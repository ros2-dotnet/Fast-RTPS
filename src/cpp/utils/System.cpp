#include <fastrtps/utils/System.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace eprosima {
namespace fastrtps {

int System::GetPID()
{
#if defined(_WIN32)
    return (int)GetCurrentProcessId();
#else
    return (int)getpid();
#endif
}

}
}