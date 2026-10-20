#pragma once
#include <map>
#include <string>
#include <vector>

namespace Scythe
{
    struct PropertyInfo
    {
        std::string name;
        std::string cppType;
        std::map<std::string, std::string> metaData;
        unsigned sourceLine = 0;
    };

    struct FunctionInfo
    {
        std::string name;
        std::string returnType;
        std::vector<std::string> parameters;
        bool isStatic = false;
    };

    struct ClassInfo
    {
        std::string name;
        std::string namespaceQualifiedName; // E.g.: "Scythe::ExampleClass"
        std::string headerPath;
        std::string baseClass;
        std::string crtBaseClass; // E.g.: "ComponentImpl"
        bool crtArgMatchesClass = false; // Checks if class T : ExampleImpl<T> is true
        unsigned generatedBodyLine = 0;
        std::map<std::string, std::string> classMetaData;
        std::vector<PropertyInfo> properties;
        std::vector<FunctionInfo> functions;
    };
    
    using ReflectionModel = std::vector<ClassInfo>;
}
