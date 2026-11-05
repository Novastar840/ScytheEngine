#pragma once
#include <string>
#include <unordered_map>

namespace Scythe::Reflection
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
    
    class TypeRegistry
    {
    public:
        static TypeRegistry& Get()
        {
            static TypeRegistry instance;
            return instance;
        }
        
        void Register(const ClassInfo& info)
        {
            m_Classes.push_back(info);
        }
        
        const ClassInfo* FindByName(const std::string& name) const
        {
            for (const ClassInfo& c : m_Classes)
            {
                if (c.name == name)
                {
                    return &c;
                }
            }
            return nullptr;
        }

        const std::vector<ClassInfo>& GetAllClasses() const { return m_Classes; }
    private:
        std::vector<ClassInfo> m_Classes;
    };
}
