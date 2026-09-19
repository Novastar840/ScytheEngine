#pragma once

#ifdef __clang__
#define SCYTHE_CLASS(...)    __attribute__((annotate("scythe-class:" #__VA_ARGS__)))
    #define SCYTHE_PROPERTY(...) __attribute__((annotate("scythe-property:" #__VA_ARGS__)))
    #define SCYTHE_FUNCTION(...) __attribute__((annotate("scythe-function:" #__VA_ARGS__)))
#else
    // MSVC and standard GCC do not use the annotate attribute in the same way
    #define SCYTHE_CLASS(...)
    #define SCYTHE_PROPERTY(...)
    #define SCYTHE_FUNCTION(...)
#endif

#define GENERATED_BODY() \
    SCYTHE_INCLUDE_GENERATED_BODY(__LINE__)

#define SCYTHE_INCLUDE_GENERATED_BODY(line) SCYTHE_INCLUDE_GENERATED_BODY_IMPL(line)
#define SCYTHE_INCLUDE_GENERATED_BODY_IMPL(line) SCYTHE_GENERATED_BODY_##line

// IDE Fallback (IntelliSense/Clangd)
// Prevents red squiggles on a clean checkout before the tool has run.
#if defined(__INTELLISENSE__) || defined(__clangd__)
    #undef GENERATED_BODY
    #define GENERATED_BODY() public:
#endif