#include "Test.h"
#include "TidyFactory.h"
#include "TidyTest.h"

TEST_CASE("OneStatementPerLine: Basic independent violation") {
    auto result = runCheckTest("OneStatementPerLine", R"(
module top();
    logic a, b, c, d;
    initial begin
        a = b; c = d;
    end
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("OneStatementPerLine: If else one line") {
    auto result = runCheckTest("OneStatementPerLine", R"(
module top();
    logic a, b, c, d;
    initial begin
        if (a) b = c; else b = d;
    end
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("OneStatementPerLine: Case one line") {
    auto result = runCheckTest("OneStatementPerLine", R"(
module top();
    logic a, b, c, d;
    initial begin
        case (a) 1: b = c; 2: b = d; endcase
    end
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("OneStatementPerLine: Declaraion exception") {
    auto result = runCheckTest("OneStatementPerLine", R"(
module top();
    logic a = 1'b1;
    int b = 2;
    reg b, c, d;
endmodule
)");
    CHECK(result);
}

TEST_CASE("OneStatementPerLine: Descending chain") {
    auto result = runCheckTest("OneStatementPerLine", R"(
module top();
    logic a, b;
    initial begin
        @(posedge a) for (int i=0; i<1; i++) case (a) 1: if (a) a = b; endcase
    end
endmodule
)");
    CHECK(result);
}

TEST_CASE("OneStatementPerLine: Error macros handling, fully in") {
    auto result = runCheckTest("OneStatementPerLine", R"(
`define ASSIGN_TWO b = a; a = b;

module top();
    logic a, b;
    initial begin
        `ASSIGN_TWO
    end
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("OneStatementPerLine: Error macros handling, particialy in") {
    auto result = runCheckTest("OneStatementPerLine", R"(
`define ASSIGN_ONE b = a;

module top();
    logic a, b;
    initial begin
       a = b; `ASSIGN_ONE
    end
endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("OneStatementPerLine: OK macros") {
    auto result = runCheckTest("OneStatementPerLine", R"(
`define ASSIGN_ONE a = b;

module top();
    logic a, b;
    initial begin
       `ASSIGN_ONE
    end
endmodule
)");
    CHECK(result);
}

TEST_CASE("OneStatementPerLine: OK on one line") {
    auto result = runCheckTest("OneStatementPerLine", R"(
module top(); logic a, b; initial begin a = b; end endmodule
)");
    CHECK(result);
}

TEST_CASE("OneStatementPerLine: Error on one line") {
    auto result = runCheckTest("OneStatementPerLine", R"(
module top (input a, input b, output reg c, output reg d);
    always@(*)
        if (b) if (a) c = b;
    endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("OneStatementPerLine: Nested increasing order") {
    auto result = runCheckTest("OneStatementPerLine", R"(
module top();
    logic a;
    always@(*)
        if (a) for (int i = 0; i < 2; ++i) begin
        end
endmodule
)");
    CHECK_FALSE(result);
}
