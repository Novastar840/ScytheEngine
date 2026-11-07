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
    
    GeneratedFiles EmitHeader(const std::string& headerPath, const std::vector<const ClassInfo*>& classes);
    bool WriteIfChanged(const std::string& path, const std::string& text);
}
