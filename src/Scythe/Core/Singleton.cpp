#include "Singleton.h"

namespace Scythe
{
    Singleton& Singleton::GetInstance()
    {
        static Singleton instance;
        return instance;
    }
}
