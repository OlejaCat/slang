#include "ASTHelperVisitors.h"
#include "TidyDiags.h"
#include <cstddef>
#include <map>
#include <optional>

using namespace slang;
using namespace slang::ast;

namespace one_statement_per_line {

struct StatementData {
    SourceLocation location;
    int32_t level;
};

struct MainVisitor : public TidyVisitor, ASTVisitor<MainVisitor, VisitFlags::AllCanonical> {
    const SourceManager* source_manager;

    std::map<size_t, std::vector<StatementData>> statementsPerLine;

    MainVisitor(Diagnostics& diagnostics, const SourceManager* source_manager) :
        TidyVisitor(diagnostics), source_manager(source_manager) {}

    template<typename T>
        requires std::derived_from<T, Statement>
    void handle(const T& statement) {
        if constexpr (std::is_same_v<T, StatementList> || std::is_same_v<T, BlockStatement>) {
            visitDefault(statement);
            return;
        }

        if (statement.kind == StatementKind::Invalid || statement.kind == StatementKind::Empty ||
            statement.kind == StatementKind::VariableDeclaration) {
            return;
        }

        SourceLocation startLocation = statement.sourceRange.start();
        if (source_manager->isMacroLoc(startLocation)) {
            startLocation = source_manager->getExpansionLoc(startLocation);
        }

        size_t currentLine = source_manager->getLineNumber(startLocation);

        statementsPerLine[currentLine].push_back(
            {startLocation, getHierarchyLevel(statement.kind)});
        visitDefault(statement);
    }

    void checkStatementsOneLine() {
        for (const auto& [line, statements] : statementsPerLine) {
            if (statements.size() <= 1) {
                continue;
            }

            for (size_t i = 0; i < statements.size() - 1; ++i) {
                if (statements[i].level <= statements[i + 1].level) {
                    diags.add(diag::OneStatementPerLine, statements[i + 1].location);
                    break;
                }
            }
        }
    }

private:
    int32_t getHierarchyLevel(StatementKind kind) const {
        switch (kind) {
            // block statements
            case StatementKind::Block:
                return 5;

            // procedural timing controls
            case StatementKind::Timed:
                return 4;

            // looping statements
            case StatementKind::ForeverLoop:
            case StatementKind::RepeatLoop:
            case StatementKind::WhileLoop:
            case StatementKind::DoWhileLoop:
            case StatementKind::ForLoop:
            case StatementKind::ForeachLoop:
                return 3;

            // case statement
            case StatementKind::Case:
            case StatementKind::PatternCase:
            case StatementKind::RandCase:
                return 2;

            // conditional statements
            case StatementKind::Conditional:
                return 1;

            default:
                return 0;
        }
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

        visitor.checkStatementsOneLine();

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
