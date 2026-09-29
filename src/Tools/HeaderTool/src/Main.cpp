#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

using namespace clang;

namespace Scythe
{
    std::string FindRepoRoot()
    {
        if (const char* Env = std::getenv("SCYTHE_REPO_ROOT"))
            if (llvm::sys::fs::exists(Env))
                return Env;
        
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
    
    // Finds SCYTHE macro markers and returns the macro arguments
    std::optional<std::string> FindMarkerAbove(ASTContext& context, const Decl* decl, llvm::StringRef markerName)
    {
        const SourceManager& sourceManager = context.getSourceManager();
        const SourceLocation location = sourceManager.getSpellingLoc(decl->getLocation());
        if (!sourceManager.isInMainFile(location))
            return std::nullopt;
        
        const StringRef text = sourceManager.getBufferData(sourceManager.getFileID(location));
        
        const unsigned offset = sourceManager.getFileOffset(location);
        size_t lineStart = text.rfind('\n', offset);
        lineStart = (lineStart == StringRef::npos) ? 0 : lineStart + 1;
        
        size_t cursor = lineStart;
        while (true)
        {
            if (cursor == 0) break;
            
            const size_t prevEnd = cursor - 1;
            size_t prevStart = 0;
            if (prevEnd > 0)
            {
                const size_t p = text.rfind('\n', prevEnd - 1);
                prevStart = (p == StringRef::npos) ? 0 : p + 1;
            }
            const StringRef line = text.slice(prevStart, prevEnd).trim(" \t\r");
            cursor = prevStart;
            
            if (line.empty() || line.starts_with("//"))
                continue;
            
            if (line.size() < markerName.size() || !line.starts_with(markerName) || line[markerName.size()] != '(')
                break;
            
            const size_t open = line.find('(');
            const size_t close = line.rfind(')');
            if (close == StringRef::npos || close < open)
            {
                llvm::errs() << "error: malformed " << markerName << " marker: " << line << "\n";
                return std::nullopt;
            }
            return std::string(line.slice(open + 1, close).trim(" \t"));
        }
        return std::nullopt;
    }
    
    class ReflectionVisitor : public RecursiveASTVisitor<ReflectionVisitor>
    {
    public:
        explicit ReflectionVisitor(ASTContext& context) : m_Ctx(context) {}

        bool VisitCXXRecordDecl(CXXRecordDecl* decl)
        {
            if (!IsInMainFile(decl)) return true;
            if (!decl->isCompleteDefinition()) return true; // skip forward declarations
            
            if (auto Args = FindMarkerAbove(m_Ctx, decl, "SCYTHE_CLASS"))
                llvm::outs() << "SCYTHE_CLASS(" << *Args << ")\n";
            llvm::outs() << "class: " << decl->getQualifiedNameAsString() << "\n";
            return true;
        }

        bool VisitFieldDecl(FieldDecl* decl)
        {
            if (!IsInMainFile(decl)) return true;
            
            if (auto Args = FindMarkerAbove(m_Ctx, decl, "SCYTHE_PROPERTY"))
                llvm::outs() << "   SCYTHE_PROPERTY(" << *Args << ")\n";
            llvm::outs() << "   field: " << decl->getNameAsString()
                << " : " << decl->getType().getAsString() << "\n";
            return true;
        }

    private:
        ASTContext& m_Ctx;

        bool IsInMainFile(const Decl* decl) const
        {
            return m_Ctx.getSourceManager().isInMainFile(decl->getLocation());
        }
    };

    class ReflectionConsumer : public ASTConsumer
    {
    public:
        void HandleTranslationUnit(ASTContext& context) override
        {
            ReflectionVisitor visitor(context);
            visitor.TraverseDecl(context.getTranslationUnitDecl());
        }
    };

    class ReflectionAction : public ASTFrontendAction
    {
    public:
        std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& compilerInstance, StringRef File) override
        {
            return std::make_unique<ReflectionConsumer>();
        }
    };
}

int main()
{
    const std::string repoRoot = Scythe::FindRepoRoot();
    if (repoRoot.empty())
    {
        llvm::errs() << "error: ScytheEngine repo root not found above the executable.\n"
                     << "        Set SCYTHE_REPO_ROOT or run the tool from a checkout.\n";
        return 1;
    }
    
    const std::vector<std::string> headers = {
        repoRoot + "/src/Tools/HeaderTool/tests/Fixtures/SampleComponent.h",
    };
    
    const std::vector<std::string> clangArgs = {
        "-x", "c++",                       // .h is ambiguous: declare the language explicitly
        "-std=c++20",
        "-I" + repoRoot + "/src/Scythe",
        "-D__SCYTHE_HEADER_TOOL__",          // macro header's tool branch
        "-Wno-pragma-once-outside-header",   // parsing a .h as a TU is intentional
    };
    
    tooling::FixedCompilationDatabase compilations(repoRoot, clangArgs);
    tooling::ClangTool tool(compilations, headers);
    
    return tool.run(tooling::newFrontendActionFactory<Scythe::ReflectionAction>().get());
}

