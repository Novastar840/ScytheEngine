#pragma once
#include <string>
#include <unordered_map>
#include <cstddef>

namespace scythe
{
    struct PropertyInfo
    {
        std::string name;
        std::string typeName;
        size_t offset;
        std::unordered_map<std::string, std::string> metadata;
    };

    struct FunctionInfo
    {
        std::string name;
        std::string returnType;
        std::vector<std::string> paramTypes;
        bool isStatic = false;
    };

    struct ClassInfo
    {
        std::string name;
        std::string namespaceQualified;
        std::vector<PropertyInfo> properties;
        std::vector<FunctionInfo> functions;
        std::unordered_map<std::string, std::string> classMetadata;
    };
}
