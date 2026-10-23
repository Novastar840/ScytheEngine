#include "Utils.h"
#include "ReflectionModel.h"

using namespace clang;

namespace Scythe
{
    struct VisitorContext
    {
        HeaderTool::ReflectionModel reflectionModel;
        std::string repoRoot;
    };    
    
    class ReflectionVisitor : public RecursiveASTVisitor<ReflectionVisitor>
    {
    public:
        explicit ReflectionVisitor(ASTContext& context, VisitorContext& vContext) 
            : m_Ctx(context), m_ReflectionModel(vContext.reflectionModel), m_RepoRoot(vContext.repoRoot)  
        {}
        
        bool VisitCXXRecordDecl(CXXRecordDecl* decl)
        {
            if (!IsInMainFile(decl) || !decl->isCompleteDefinition())
                return true;
            
            const auto args = HeaderTool::FindMarkerAbove(m_Ctx, decl, "SCYTHE_CLASS");
            
            if (args == std::nullopt) // no marker found
            {
                if (ClassBodyContains(decl, "SCYTHE_COMPONENT("))
                {
                    llvm::outs() << "[SHT] legacy marker, not reflected yet: " 
                                 << decl->getQualifiedNameAsString() << '\n';
                }
                return true;
            }

            HeaderTool::ClassInfo info {
                .name = decl->getNameAsString(),
                .namespaceQualifiedName = decl->getQualifiedNameAsString(),
                .headerPath = GetRepoRelativePath(decl),
                .generatedBodyLine = FindGeneratedBodyLine(decl),
                .classMetaData = HeaderTool::ParseKeyValueArgs(*args),
            };
            
            if (info.generatedBodyLine == 0)
                llvm::errs() << "[SHT] error: " << info.namespaceQualifiedName 
                             << "marked as SCYTHE_CLASS but no GENERATED_BODY() was found\n";
            
            ExtractBase(decl, info);
            ExtractProperties(decl, info);
            
            m_ReflectionModel.push_back(std::move(info));
            
            return true;
        }

    private:
        void ExtractBase(const CXXRecordDecl* decl, HeaderTool::ClassInfo& info) const
        {
            for (const auto& base : decl->bases())
            {
                QualType baseType = base.getType();
                if (info.baseClass.empty())
                    info.baseClass = baseType.getAsString(GetPrintingPolicy());
                
                if (const auto* spec = baseType->getAs<TemplateSpecializationType>())
                {
                    const std::string tmpl = spec->getTemplateName().getAsTemplateDecl()->getNameAsString();
                    if (tmpl == "ComponentImpl" || tmpl == "compImpl")
                    {
                        info.crtBaseClass = tmpl;
                        if (spec->template_arguments().size() == 1 && spec->template_arguments()[0].getKind() == TemplateArgument::Type)
                        {
                            const QualType arg = spec->template_arguments()[0].getAsType();
                            info.crtArgMatchesClass = (arg->getAsCXXRecordDecl() == decl);
                            if (!info.crtArgMatchesClass)
                                llvm::errs() << "error: " << info.namespaceQualifiedName
                                             << ": CRTP base " << tmpl << "<"
                                             << arg.getAsString(GetPrintingPolicy())
                                             << "> does not name the class itself\n";
                        }
                    }
                }
                break;
            }
        }
        
        void ExtractProperties(const CXXRecordDecl* decl, HeaderTool::ClassInfo& info) const
        {
            for (const auto& field : decl->fields())
            {
                const auto args = HeaderTool::FindMarkerAbove(m_Ctx, field, "SCYTHE_PROPERTY");
                if (args == std::nullopt) continue;

                HeaderTool::PropertyInfo pInfo{
                    .name = field->getNameAsString(),
                    .cppType = field->getType().getAsString(GetPrintingPolicy()),
                    .metaData = HeaderTool::ParseKeyValueArgs(*args), 
                    .sourceLine = m_Ctx.getSourceManager().getSpellingLineNumber(field->getLocation())
                };
                info.properties.push_back(std::move(pInfo));
            }
        }
        
        PrintingPolicy GetPrintingPolicy() const
        {
            PrintingPolicy pPolicy = m_Ctx.getPrintingPolicy();
            pPolicy.Bool = true;
            pPolicy.SuppressTagKeyword = true;
            return pPolicy;
        }
        
        unsigned FindGeneratedBodyLine(const CXXRecordDecl* decl) const
        {
            const SourceManager& sourceManager = m_Ctx.getSourceManager();
            const FileID fileId = sourceManager.getFileID(decl->getLocation());
            const StringRef text = sourceManager.getBufferData(fileId);
            const unsigned first = sourceManager.getSpellingLineNumber(decl->getBeginLoc());
            const unsigned last = sourceManager.getSpellingLineNumber(decl->getEndLoc());
            
            for (unsigned line = first; line < last; line++)
            {
                const SourceLocation loc = sourceManager.translateLineCol(fileId, line, 1);
                if (loc.isInvalid()) continue;
                
                const size_t offset = sourceManager.getFileOffset(loc);
                if (offset >= text.size()) break;
                
                size_t endOfLine = text.find("\n", offset);
                if (endOfLine == StringRef::npos) endOfLine = text.size();
                
                const StringRef lineRef = text.slice(offset, endOfLine).trim(" \t\r");
                
                if (lineRef.starts_with("GENERATED_BODY()"))
                    return line;
            }
            return 0;
        }
        
        bool ClassBodyContains(const CXXRecordDecl* decl, const std::string& searchString) const
        {
            SourceManager& sourceManager = m_Ctx.getSourceManager();
            StringRef text = sourceManager.getBufferData(sourceManager.getFileID(decl->getLocation()));
            const unsigned first = sourceManager.getFileOffset(decl->getBeginLoc());
            const unsigned last = sourceManager.getFileOffset(decl->getEndLoc());
            return text.slice(first, last).contains(searchString);
        }
        
        // Gets the relative path of the file that contains the declaration
        std::string GetRepoRelativePath(const CXXRecordDecl* decl) const
        {
            const SourceManager& sourceManager = m_Ctx.getSourceManager();
            const FileID fileID = sourceManager.getFileID(sourceManager.getSpellingLoc(decl->getLocation()));
            if (const auto file = sourceManager.getFileEntryRefForID(fileID))
            {
                // FileEntryRef only carries the accessed name; the canonical-path
                // query lives on the FileEntry it refers to.
                const FileEntry& entry = file->getFileEntry();

                std::string canonical = file->getName().str();          // accessed path
                if (const std::optional<StringRef> real = entry.tryGetRealPathName())
                    canonical = std::string(*real);                     // symlink/junction-resolved

                return HeaderTool::MakeRepoRelative(canonical, m_RepoRoot);
            }
            return std::string();
        }
        
        bool IsInMainFile(const Decl* decl) const
        {
            return m_Ctx.getSourceManager().isInMainFile(decl->getLocation());
        }
        
        ASTContext& m_Ctx;
        HeaderTool::ReflectionModel& m_ReflectionModel;
        std::string m_RepoRoot;
    };

    class ReflectionConsumer : public ASTConsumer
    {
    public:
        explicit ReflectionConsumer(VisitorContext& visitorContext) : m_VisitorContext(visitorContext) {}
        void HandleTranslationUnit(ASTContext& context) override
        {
            ReflectionVisitor visitor(context, m_VisitorContext);
            visitor.TraverseDecl(context.getTranslationUnitDecl());
        }
    private:
        VisitorContext& m_VisitorContext;
    };

    class ReflectionAction : public ASTFrontendAction
    {
    public:
        explicit ReflectionAction(VisitorContext& visitorContext) : m_VisitorContext(visitorContext) {}
    protected:
        std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& compilerInstance, StringRef File) override
        {
            compilerInstance.getFrontendOpts().SkipFunctionBodies = true;
            return std::make_unique<ReflectionConsumer>(m_VisitorContext);
        }
    private:
        VisitorContext& m_VisitorContext;
    };
    
    class ScythePCHAction : public GeneratePCHAction
    {
    public:
        explicit ScythePCHAction(const std::string& out) : m_Out(out) {}
        bool BeginSourceFileAction(CompilerInstance& CI) override
        {
            CI.getFrontendOpts().OutputFile = m_Out;
            return GeneratePCHAction::BeginSourceFileAction(CI);
        }
    private:
        const std::string& m_Out;
    };
    
    class ScythePCHActionFactory : public tooling::FrontendActionFactory
    {
    public:
        explicit ScythePCHActionFactory(const std::string& out) : m_Out(out) {}

        std::unique_ptr<FrontendAction> create() override
        {
            return std::make_unique<ScythePCHAction>(m_Out);
        }

    private:
        const std::string& m_Out;
    };
    
    class ReflectionActionFactory : public tooling::FrontendActionFactory
    {
    public:
        explicit ReflectionActionFactory(VisitorContext& vContext) : m_VisitorContext(vContext) {}
        std::unique_ptr<FrontendAction> create() override
        {
            return std::make_unique<ReflectionAction>(m_VisitorContext);
        }
    private:
        VisitorContext& m_VisitorContext;
    };
}

int main(int argc, const char** argv)
{
    using namespace Scythe::HeaderTool;
    
    const std::string repoRoot = FindRepoRoot();
    if (repoRoot.empty())
    {
        llvm::errs() << "[SHT] error: ScytheEngine repo root not found above the executable.\n"
                     << "        Set SCYTHE_REPO_ROOT or run the tool from a checkout.\n";
        return 1;
    }

    const std::string compileCommandsPath = FindCompileCommands(repoRoot);
    const std::string pchHeader = repoRoot + "/src/Scythe/ScytheEnginePCH.h";
    const std::string Pch       = GetCompiledPCHPath(repoRoot);

    // --- PCH production ---
    // ScytheHeaderTool --build-pch <out.pch> <PCH.h>
    if (argc > 3 && std::string(argv[1]) == "--build-pch")
    {
        const std::string out    = argv[2];
        const std::string header = argv[3];
        
        auto flags = BuildFlagsFromCompileCommands(
            compileCommandsPath, PCHMode::None, pchHeader, Pch);
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
        if (HasAnyMarker(h)) marked.push_back(h);
    if (marked.empty())
        return 0;

    const bool havePch = !Pch.empty() && llvm::sys::fs::exists(Pch);
    auto flags = BuildFlagsFromCompileCommands(
        compileCommandsPath,
        havePch ? PCHMode::UsePch : PCHMode::ForceInclude,
        pchHeader, Pch);
    if (!flags)
        flags = std::vector<std::string>{
            "--driver-mode=cl", "/TP", "/std:c++20",
            "/I" + repoRoot + "/src/Scythe",
            "/D__SCYTHE_HEADER_TOOL__",
            "-Wno-pragma-once-outside-header",
            "/FI" + pchHeader };

    Scythe::VisitorContext visitorContext { .repoRoot = repoRoot };
    
    tooling::FixedCompilationDatabase compilations(repoRoot, *flags);
    tooling::ClangTool tool(compilations, marked);
    Scythe::ReflectionActionFactory factory(visitorContext);
    const int result = tool.run(&factory);
    if (result != 0) return result; // failed parse
    
    // --- temp. model dump ---
    for (const auto& classInfo : visitorContext.reflectionModel)
    {
        llvm::outs() << "class " << classInfo.namespaceQualifiedName
                     << "  [" << classInfo.headerPath << ":" << classInfo.generatedBodyLine << "]\n";
        if (!classInfo.baseClass.empty())
        {
            llvm::outs() << "    base: " << classInfo.baseClass;
            if (!classInfo.crtBaseClass.empty())
                llvm::outs() << "  crtp=" << classInfo.crtBaseClass
                             << (classInfo.crtArgMatchesClass ? " (ok)" : " (MISMATCH)");
            llvm::outs() << "\n";
        }
        for (const auto& [k, v] : classInfo.classMetaData)
            llvm::outs() << "    meta: " << k << " = " << v << "\n";
        for (const auto& p : classInfo.properties)
            llvm::outs() << "    prop: " << p.name << " : " << p.cppType
                         << "  @" << p.sourceLine << "\n";
    }
    return 0;
}
