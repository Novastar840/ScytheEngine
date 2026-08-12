#pragma once


namespace Scythe
{

    class Subsystem
    {
    public:
        virtual ~Subsystem() = default;

        virtual bool Initialize() { return true; }
        virtual void Shutdown() {}
        virtual void Update(float deltaTime) {}

        virtual const char* GetName() const = 0;
    };
}