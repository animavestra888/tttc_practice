#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Sema/Sema.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {
class AddPrefixesVisitor : public RecursiveASTVisitor<AddPrefixesVisitor> {
    ASTContext &Context;
    Rewriter &RW;
    std::map<const VarDecl*, std::string> renameMap;

public:
    AddPrefixesVisitor(ASTContext &Ctx, Rewriter &R) : Context(Ctx), RW(R) {
        RW.setSourceMgr(Ctx.getSourceManager(),
                      Ctx.getLangOpts());
    } 

    bool VisitVarDecl(VarDecl *VD) {
        if (VD->isImplicit() || VD->isInvalidDecl()) return true;

        std::string prefix;
        if (VD->isFileVarDecl()) {
             prefix = "global_";
        }
        else if (VD->getStorageClass() == SC_Static) {
             prefix = "static_";
        }
        else if (isa<ParmVarDecl>(VD)) {
             prefix = "param_";
        }
        else {
             prefix = "local_";
        }

        std::string name = VD->getNameAsString();
        if (name.find(prefix) == 0) return true;

        std::string newName = prefix + name;
        renameMap[VD] = newName;

        SourceLocation loc = VD->getLocation();
        RW.ReplaceText(loc, name.length(), newName);
        return true;
    }

    bool VisitDeclRefExpr(DeclRefExpr *DRE) {
        if (VarDecl *VD = dyn_cast<VarDecl>(DRE->getDecl())) {
            if (renameMap.find(VD) != renameMap.end()) {
                RW.ReplaceText(DRE->getLocation(), VD->getName().size(), renameMap[VD]);
            }
        }
        return true;
    }
};

class AddPrefixesConsumer : public ASTConsumer {
    Rewriter RW;
    AddPrefixesVisitor Visitor;

public:
    AddPrefixesConsumer(ASTContext &Context) : Visitor(Context, RW) {}

    void HandleTranslationUnit(ASTContext &Context) override {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
        RW.getEditBuffer(RW.getSourceMgr().getMainFileID()).write(llvm::outs());	
    }
};

class AddPrefixesAction : public PluginASTAction {
protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
        return std::make_unique<AddPrefixesConsumer>(CI.getASTContext());
    }

    bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &Args) override {
        return true;
    }
};
}
  
static FrontendPluginRegistry::Add<AddPrefixesAction>
X("add-prefixes", "add prefixes to variable names");

