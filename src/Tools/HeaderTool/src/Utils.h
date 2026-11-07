#pragma once
#include <filesystem>
#include <fstream>

namespace Scythe::HeaderTool
{
    inline std::string FindRepoRoot()
    {
        llvm::SmallString<256> Dir(llvm::sys::fs::getMainExecutable(nullptr, nullptr));
        llvm::sys::path::remove_filename(Dir);

        while (true)
        {
            llvm::SmallString<256> Sentinel(Dir);
            llvm::sys::path::append(Sentinel, "src", "Scythe", "Main.cpp");
            if (llvm::sys::fs::exists(Sentinel))
                return std::string(Dir.str());

            llvm::SmallString<256> Parent(Dir);
            llvm::sys::path::remove_filename(Parent);   // one level up
            if (Parent == Dir)
                break;                                  // reached filesystem root
            Dir = Parent;
        }
        return {};
    }
    
    // Returns the path to compile_commands.json (empty string if none found), trying release first, then debug.
    inline std::string FindCompileCommands(const std::string& repoRoot)
    {
#ifdef SCYTHE_COMPILE_COMMANDS
        if (llvm::sys::fs::exists(SCYTHE_COMPILE_COMMANDS))
            return SCYTHE_COMPILE_COMMANDS;
#endif
        
        static const char* Builds[] = { "cmake-build-release", "cmake-build-debug" };
        for (const char* buildDir : Builds)
        {
            llvm::SmallString<256> path(repoRoot);
            llvm::sys::path::append(path, buildDir, "compile_commands.json");
            if (llvm::sys::fs::exists(path))
                return std::string(path.str());
        }
        llvm::errs() << "[SHT] no compile_commands.json found in cmake-build-release/ or cmake-build-debug/\n";
        return {};  // neither found — let the caller emit a warning
    }
    
    // Finds SCYTHE macro markers and returns the macro arguments
    inline std::optional<std::string> FindMarkerAbove(clang::ASTContext& context, const clang::Decl* decl, llvm::StringRef markerName)
    {
        const clang::SourceManager& sourceManager = context.getSourceManager();
        const clang::SourceLocation location = sourceManager.getSpellingLoc(decl->getLocation());
        if (!sourceManager.isInMainFile(location))
            return std::nullopt;

        const llvm::StringRef text = sourceManager.getBufferData(sourceManager.getFileID(location));

        const unsigned offset = sourceManager.getFileOffset(location);
        size_t lineStart = text.rfind('\n', offset);
        lineStart = (lineStart == llvm::StringRef::npos) ? 0 : lineStart + 1;

        size_t cursor = lineStart;
        while (true)
        {
            if (cursor == 0) break;

            const size_t prevEnd = cursor - 1;
            size_t prevStart = 0;
            if (prevEnd > 0)
            {
                const size_t p = text.rfind('\n', prevEnd - 1);
                prevStart = (p == llvm::StringRef::npos) ? 0 : p + 1;
            }
            const llvm::StringRef line = text.slice(prevStart, prevEnd).trim(" \t\r");
            cursor = prevStart;

            if (line.empty() || line.starts_with("//"))
                continue;

            if (line.size() < markerName.size() || !line.starts_with(markerName) || line[markerName.size()] != '(')
                break;

            const size_t open = line.find('(');
            const size_t close = line.rfind(')');
            if (close == llvm::StringRef::npos || close < open)
            {
                llvm::errs() << "error: malformed " << markerName << " marker: " << line << "\n";
                return std::nullopt;
            }
            return std::string(line.slice(open + 1, close).trim(" \t"));
        }
        return std::nullopt;
    }

    inline bool HasAnyMarker(llvm::StringRef path)
    {
        auto Buffer = llvm::MemoryBuffer::getFile(path);
        if (!Buffer)
            return false;
        const llvm::StringRef text = Buffer.get()->getBuffer();
        return text.contains("SCYTHE_CLASS(")    || text.contains("SCYTHE_COMPONENT(")
            || text.contains("SCYTHE_PROPERTY(") || text.contains("SCYTHE_FUNCTION(");
    }

    // Which PCH strategy feeds the shared std/third-party header into a parse:
    //   None         - no precompiled header at all. Used when building the PCH.
    //   ForceInclude - textual /FI<PCH.h>; fallback while no .pch exists yet.
    //   UsePch       - uses the compiled PCH.
    enum class PCHMode { None, ForceInclude, UsePch };
    
    inline std::optional<std::vector<std::string>> BuildFlagsFromCompileCommands(
        const std::string& dbPath,
        const PCHMode mode,
        const std::string& preludeHeader,
        const std::string& preludePch)
    {
        std::string err;
        auto db = clang::tooling::JSONCompilationDatabase::loadFromFile(
            dbPath, err, clang::tooling::JSONCommandLineSyntax::AutoDetect);
        if (!db)
        {
            if (dbPath.empty()) return std::nullopt;  // caller already warned
            llvm::errs() << "warning: no compilation database at " << dbPath
                         << " (" << err << "); using builtin fallback flags\n";
            return std::nullopt;
        }

        for (const clang::tooling::CompileCommand& cmd : db->getAllCompileCommands())
        {
            const llvm::StringRef file(cmd.Filename);
            if (!file.ends_with(".cpp") && !file.ends_with(".cxx")) continue;
            if (!file.contains("src/Scythe/") && !file.contains("src\\Scythe\\")) continue;

            // The compile DB holds MSVC-style flags (/I, /D, /FI, ...), so we stay in
            // clang-cl mode. Every element of this vector is ONE argv entry.
            std::vector<std::string> args;
            args.push_back("--driver-mode=cl");
            args.push_back("/TP"); // treat inputs (.h) as C++
            args.push_back("/std:c++20");
            args.push_back("/EHsc"); // sources use throw; clang-cl errors without it
            args.push_back("/D__SCYTHE_HEADER_TOOL__");
            args.push_back("-Wno-pragma-once-outside-header");

            // Skip index 0 (the compiler executable itself)
            for (size_t i = 1; i < cmd.CommandLine.size(); ++i)
            {
                const std::string& arg = cmd.CommandLine[i];

                if (arg == cmd.Filename) continue;                           // the TU itself
                if (arg == "/c" || arg == "-c") continue;                      // compile-only

                // MSVC binary PCH: unusable by clang
                if (arg.starts_with("/Yu") || arg.starts_with("/Yc") ||
                    arg.starts_with("/Fp")) continue;

                // Output / diagnostics-to-file flags
                if (arg.starts_with("/Fo") || arg.starts_with("/Fd") ||
                    arg.starts_with("/FS") || arg.starts_with("/showIncludes")) continue;

                // CMake's generated PCH wrapper (cmake_pch.hxx) force-include.
                // It lives in the build dir and pulls in MSVC-PCH machinery; we
                // force-include our own PCH header below instead.
                if (arg.starts_with("/FI") && arg.find("cmake_pch") != std::string::npos) continue;

                // We already set the standard explicitly above
                if (arg.starts_with("/std:") || arg.starts_with("-std:") ||
                    arg.starts_with("-std=")) continue;

                // Parse-relevant flags that clang-cl understands
                if (arg.starts_with("/I")  || arg.starts_with("-I")  ||
                    arg.starts_with("/D")  || arg.starts_with("-D")  ||
                    arg.starts_with("/FI") ||
                    arg.starts_with("/utf-8") ||
                    arg.starts_with("/Zc:") ||
                    arg.starts_with("/external"))
                {
                    args.push_back(arg);
                }
            }
            
            switch (mode)
            {
            case PCHMode::None:
                break;                                    // header being frozen IS the prelude
            case PCHMode::ForceInclude:
                if (!preludeHeader.empty())
                    args.push_back("/FI" + preludeHeader);
                break;
            case PCHMode::UsePch:
                //   /Yu + /Fp : "this PCH stands in for this header"
                //   /FI       : the include point where the substitution triggers
                // (If the PCH is already loaded unconditionally, the /FI is skipped by
                //  the pragma-once state serialized inside the PCH - harmless either way.)
                args.push_back("/Yu" + preludeHeader);
                args.push_back("/Fp" + preludePch);
                args.push_back("/FI" + preludeHeader);
                break;
            }
            return args;
        }

        llvm::errs() << "[SHT] warning: no engine TU found in " << dbPath << "\n";
        return std::nullopt;
    }
    
    inline std::string GetCompiledPCHPath(const std::string& root)
    {
#ifdef SCYTHE_HEADER_TOOL_PCH
        if (llvm::sys::fs::exists(SCYTHE_HEADER_TOOL_PCH))
            return SCYTHE_HEADER_TOOL_PCH;
#endif
        
        static const char* Builds[] = { "cmake-build-release", "cmake-build-debug" };
        for (const char* buildDir : Builds)
        {
            llvm::SmallString<256> path(root);
            llvm::sys::path::append(path, buildDir, "headerTool", "SHT.pch");
            if (llvm::sys::fs::exists(path))
                return std::string(path.str());
        }
        llvm::errs() << "[SHT] no SHT.pch found in cmake-build-release/ or cmake-build-debug/\n";
        return {};
    }
    
    inline std::string MakeRepoRelative(const std::string& absolute, const std::string& repoRoot)
    {
        std::error_code ec;   
        std::filesystem::path path = std::filesystem::weakly_canonical(absolute, ec);
        if (ec) path = absolute;
        
        std::filesystem::path relative = std::filesystem::weakly_canonical(repoRoot, ec);
        if (ec) relative = repoRoot;
        
        std::string pathStr = path.generic_string();
        std::string relativeStr = relative.generic_string();
        
        while (relativeStr.size() > 1 && (relativeStr.back() == '/' || relativeStr.back() == '\\'))
            relativeStr.pop_back();
        
        std::string pathLower = pathStr;
        std::string relativeLower = relativeStr;
        std::ranges::transform(pathLower, pathLower.begin(), tolower);
        std::ranges::transform(relativeLower, relativeLower.begin(), tolower);
        
        if (pathLower.starts_with(relativeLower) &&
            (pathLower.size() == relativeLower.size() || pathLower[relativeLower.size()] == '/'))
        {
            std::string rel = pathStr.substr(relativeStr.size());
            if (!rel.empty() && (rel[0] == '/' || rel[0] == '\\'))
                rel.erase(0, 1);
            return rel;
        }
        return pathStr;
    }
    
    inline std::map<std::string, std::string> ParseKeyValueArgs(const std::string& args)
    {
        std::map<std::string, std::string> out;
        std::string key, value;
        bool inQuotes = false;
        bool hasKey = false;
        
        auto trim = [](std::string string) -> std::string
        {
            constexpr const char* ws = " \t\r\n";
            string.erase(0, string.find_first_not_of(ws));
            string.erase(string.find_last_not_of(ws) + 1);
            return string;
        };
        auto saveAndFlush = [&]
        {
            if (!key.empty() || !value.empty())
                out[trim(key)] = trim(value);
            key.clear();
            value.clear();
            hasKey = false;
        };
        
        for (const char c : args)
        {
            if (c == '"') { inQuotes = !inQuotes; continue;}
            if (c == ',' && !inQuotes) { saveAndFlush(); continue;}
            if (c == '=' && !inQuotes && !hasKey) { hasKey = true; continue; }
            (hasKey ? value : key).push_back(c);
        }
        saveAndFlush();
        return out;
    }

    inline bool WriteIfChanged(const std::string& path, const std::string& text)
    {
        std::ifstream in(path, std::ios::binary);
        if (in)
        {
            const std::string current((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            if (current == text) return false;
        }
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        out << text;
        return true;
    }

    static const char* ReadTextFile(const std::string& filePath)
    {
        std::ifstream file(filePath);

        if (!file.is_open())
            throw std::runtime_error("Failed to open file: " + filePath);

        std::stringstream buffer;
        buffer << file.rdbuf();

        return buffer.str().c_str();
    }
}
