#pragma once

namespace Scythe
{
    class Singleton
    {
    public:
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;
        Singleton(Singleton&&) = delete;
        Singleton& operator=(Singleton&&) = delete;

        static Singleton& GetInstance();
    private:
        Singleton() = default;
        ~Singleton() = default;
    };
}
