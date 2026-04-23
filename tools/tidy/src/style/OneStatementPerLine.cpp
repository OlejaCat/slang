#include "ASTHelperVisitors.h"
#include "TidyDiags.h"

using namespace slang;
using namespace slang::ast;

namespace one_statement_per_line {

struct MainVisitor : public TidyVisitor, ASTVisitor<MainVisitor, VisitFlags::AllCanonical> {
    const SourceManager* source_manager;
    size_t lastLine = 0;

    MainVisitor(Diagnostics& diagnostics, const SourceManager* source_manager) :
        TidyVisitor(diagnostics), source_manager(source_manager) {}

    template<typename T>
        requires std::derived_from<T, Statement>
    void handle(const T& statement) {
        if constexpr (std::is_same_v<T, StatementList> || std::is_same_v<T, BlockStatement>) {
            visitDefault(statement);
            return;
        }

        if (statement.kind == StatementKind::Invalid ||
            statement.kind == StatementKind::VariableDeclaration) {
            return;
        }

        SourceLocation startLocation = statement.sourceRange.start();
        if (source_manager->isMacroLoc(startLocation)) {
            startLocation = source_manager->getExpansionLoc(startLocation);
        }

        size_t currentLine = source_manager->getLineNumber(startLocation);

        if (currentLine != 0 && currentLine == lastLine) {
            diags.add(diag::OneStatementPerLine, startLocation);
        }

        size_t previousNeighborLine = currentLine;
        lastLine = 0;

        visitDefault(statement);

        lastLine = previousNeighborLine;
    }
};

} // namespace one_statement_per_line

using namespace one_statement_per_line;
class OneStatementPerLine : public TidyCheck {
public:
    [[maybe_unused]] explicit OneStatementPerLine(
        TidyKind kind, std::optional<slang::DiagnosticSeverity> severity) :
        TidyCheck(kind, severity) {}

    bool check(const ast::RootSymbol& root, const slang::analysis::AnalysisManager&) override {
        const SourceManager* source_manager = root.getCompilation().getSourceManager();

        MainVisitor visitor(diagnostics, source_manager);
        root.visit(visitor);
        return diagnostics.empty();
    }

    DiagCode diagCode() const override { return diag::OneStatementPerLine; }
    DiagnosticSeverity diagDefaultSeverity() const override { return DiagnosticSeverity::Warning; }
    std::string diagString() const override {
        return "Multiple statements are described in the single line. "
               "Describe one statement per line to improce RTL description readability";
    }
    std::string name() const override { return "OneStatementPerLine"; }
    std::string description() const override { return shortDescription(); }
    std::string shortDescription() const override {
        return "Checks that only one statement described on single line";
    }
};

REGISTER(OneStatementPerLine, OneStatementPerLine, TidyKind::Style);
