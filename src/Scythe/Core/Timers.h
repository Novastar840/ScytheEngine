#pragma once
#include <chrono>
#include <spdlog/spdlog.h>

namespace Scythe
{
    // Measures the duration of a callable and logs the elapsed time.
    // Returns the timed time.
    template <typename Func, typename... Args>
        requires std::invocable<Func, Args...>
    double TimeFunction(std::string_view label, Func&& func, Args&&... args)
    {
        const auto start = std::chrono::steady_clock::now();
        
        // casting to void to avoid possible [[nodiscard]] warnings.
        static_cast<void>(std::invoke(std::forward<Func>(func), std::forward<Args>(args)...));
        
        const auto end = std::chrono::steady_clock::now();
        
        const double ms = std::chrono::duration<double, std::milli>(end - start).count();
        spdlog::info("Timer [{}]: {:.5f} ms", label, ms);
        return ms;
    }

    enum TimingGuarPrintFormat : int8_t
    {
        milliSeconds,
        seconds,
        minutes,
        hours
    };
    
    // Measures how long a span of work takes and logs the elapsed time.
    class TimingGuard
    {
    public:
        explicit TimingGuard(std::string_view label)
                : m_Label(label)
        {
        }
        
        TimingGuard() = default;

        void SetNow()
        {
            m_Start = std::chrono::steady_clock::now();
        }
        
        void SetNow(std::string_view label)
        {
            m_Label = label;
            m_Start = std::chrono::steady_clock::now();
        }
        
        void PrintTime(TimingGuarPrintFormat format = milliSeconds)
        {
            const auto end = std::chrono::steady_clock::now();
            const double time = std::chrono::duration<double, std::milli>(end - m_Start).count();

            const auto [scaled, unit] = [&]() -> std::pair<double, std::string_view> {
                switch (format)
                {
                case TimingGuarPrintFormat::hours:   return {time / (60. * 60. * 1000.), "hours"};
                case TimingGuarPrintFormat::minutes: return {time / (60. * 1000.),       "minutes"};
                case TimingGuarPrintFormat::seconds: return {time / 1000.,               "seconds"};
                default:                             return {time,                       "ms"};
                }
            }();

            spdlog::info("Timer [{}]: {:.5f} {}", m_Label, scaled, unit);
        }
        
        double GetTime() const
        {
            const auto end = std::chrono::steady_clock::now();
            const double ms = std::chrono::duration<double, std::milli>(end - m_Start).count();
            return ms;
        }
        
        void SetLabel(std::string_view label)
        {
            m_Label = label;
        }
        
        void PrintAndSetNow()
        {
            PrintTime();
            SetNow();
        }
        
        void PrintAndSetNow(TimingGuarPrintFormat format)
        {
            PrintTime(format);
            SetNow();
        }
        
        void PrintAndSetNow(std::string_view label, TimingGuarPrintFormat format = milliSeconds)
        {
            PrintTime(format);
            SetNow(label);
        }
        
        double GetAndSetNow()
        {
            double ms = GetTime();
            SetNow();
            return ms;
        }
        
        double GetAndSetNow(std::string_view label)
        {
            double ms = GetTime();
            SetNow(label);
            return ms;
        }

        TimingGuard(const TimingGuard&) = delete;
        TimingGuard& operator=(const TimingGuard&) = delete;

    private:
        
        std::string m_Label;
        std::chrono::steady_clock::time_point m_Start = std::chrono::steady_clock::now();
    };
}
