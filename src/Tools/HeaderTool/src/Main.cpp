#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/JSONCompilationDatabase.h"
#include "clang/Frontend/FrontendActions.h"
#include "Utils.h"

using namespace clang;

namespace Scythe
{
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
    
    class ScythePCHAction : public GeneratePCHAction
    {
    public:
        explicit ScythePCHAction(std::string out) : m_Out(std::move(out)) {}
    protected:
        bool BeginSourceFileAction(CompilerInstance& CI) override
        {
            CI.getFrontendOpts().OutputFile = m_Out;   // before base reads it
            return GeneratePCHAction::BeginSourceFileAction(CI);
        }
    private:
        std::string m_Out;
    };
    
    class ScythePCHActionFactory : public tooling::FrontendActionFactory
    {
    public:
        explicit ScythePCHActionFactory(std::string out) : m_Out(std::move(out)) {}

        std::unique_ptr<FrontendAction> create() override
        {
            return std::make_unique<ScythePCHAction>(m_Out);
        }

    private:
        std::string m_Out;
    };
}

int main(int argc, const char** argv)
{
    const std::string repoRoot = Scythe::FindRepoRoot();
    if (repoRoot.empty())
    {
        llvm::errs() << "[SHT] error: ScytheEngine repo root not found above the executable.\n"
                     << "        Set SCYTHE_REPO_ROOT or run the tool from a checkout.\n";
        return 1;
    }

    const std::string compileCommandsPath = Scythe::FindCompileCommands(repoRoot);
    const std::string preludeHeader = repoRoot + "/src/Scythe/ScytheEnginePCH.h";
    const std::string preludePch    = Scythe::GetCompiledPCHPath(repoRoot);

    // --- PCH production ---
    // ScytheHeaderTool --build-pch <out.pch> <PCH.h>
    if (argc > 3 && std::string(argv[1]) == "--build-pch")
    {
        const std::string out    = argv[2];
        const std::string header = argv[3];
        
        auto flags = Scythe::BuildFlagsFromCompileCommands(
            compileCommandsPath, Scythe::PCHMode::None, preludeHeader, preludePch);
        if (!flags)
            flags = std::vector<std::string>{
                "--driver-mode=cl", "/TP", "/std:c++20",
                "/D__SCYTHE_HEADER_TOOL__",
                "-Wno-pragma-once-outside-header" };

        tooling::FixedCompilationDatabase db(repoRoot, *flags);
        tooling::ClangTool tool(db, { header });
        Scythe::ScythePCHActionFactory factory(out);
        return tool.run(&factory);
    }
    
    std::vector<std::string> headers;
    if (argc > 1)
        headers.assign(argv + 1, argv + argc);
    else
        headers = { repoRoot + "/src/Tools/HeaderTool/tests/Fixtures/SampleComponent.h" };

    std::vector<std::string> marked;
    for (const auto& h : headers)
        if (Scythe::HasAnyMarker(h)) marked.push_back(h);
    if (marked.empty())
        return 0;

    const bool havePch = !preludePch.empty() && llvm::sys::fs::exists(preludePch);
    auto flags = Scythe::BuildFlagsFromCompileCommands(
        compileCommandsPath,
        havePch ? Scythe::PCHMode::UsePch : Scythe::PCHMode::ForceInclude,
        preludeHeader, preludePch);
    if (!flags)
        flags = std::vector<std::string>{
            "--driver-mode=cl", "/TP", "/std:c++20",
            "/I" + repoRoot + "/src/Scythe",
            "/D__SCYTHE_HEADER_TOOL__",
            "-Wno-pragma-once-outside-header",
            "/FI" + preludeHeader };

    tooling::FixedCompilationDatabase compilations(repoRoot, *flags);
    tooling::ClangTool tool(compilations, marked);
    return tool.run(tooling::newFrontendActionFactory<Scythe::ReflectionAction>().get());
}
