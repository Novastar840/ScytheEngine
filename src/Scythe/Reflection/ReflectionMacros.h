#pragma once

#define SCYTHE_CLASS(...)
#define SCYTHE_PROPERTY(...)
#define SCYTHE_FUNCTION(...)

#ifdef __SCYTHE_HEADER_TOOL__
    #define GENERATED_BODY()
#else
    #define GENERATED_BODY() \
        SCYTHE_INCLUDE_GENERATED_BODY(__LINE__)

    #define SCYTHE_INCLUDE_GENERATED_BODY(line) SCYTHE_INCLUDE_GENERATED_BODY_IMPL(line)
    #define SCYTHE_INCLUDE_GENERATED_BODY_IMPL(line) SCYTHE_GENERATED_BODY_##line
#endif

// IDE Fallback (IntelliSense/Clangd)
// Prevents red squiggles on a clean checkout before the tool has run.
#if defined(__INTELLISENSE__) || defined(__clangd__)
    #undef GENERATED_BODY
    #define GENERATED_BODY() public:
#endif