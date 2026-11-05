#pragma once
#include <string>

#include "ReflectionModel.h"

namespace Scythe::HeaderTool
{
    struct GeneratedFiles
    {
        std::string header;
        std::string source;
    };
    
    GeneratedFiles EmitClass(const ClassInfo& classInfo);
    bool WriteIfChanged(const std::string& path, const std::string& text);
}
