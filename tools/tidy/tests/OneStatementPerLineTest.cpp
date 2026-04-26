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

TEST_CASE("OneStatementPerLine: Hierarchical dependent") {
    auto result = runCheckTest("OneStatementPerLine", R"(
module top();
    logic a, b, c;
    initial begin
        if (a) b = c;
        if (a) begin b = c; end
        case (a) 1: b = c; endcase
        for (int i=0; i<1; i++) a = b;
        while (a) a = b;
        repeat (10) a = b;
        forever a = b;
        #(10) a = b;
        @(posedge c) a = b;
    end

    task t; a = b; endtask
    function f; a = b; endfunction
endmodule
)");
    CHECK(result);
}

TEST_CASE("OneStatementPerLine: Descending chain") {
    auto result = runCheckTest("OneStatementPerLine", R"(
module top();
    logic a, b;
    initial begin @(posedge a) #10 if (a) for (int i=0; i<1; i++) while (a) repeat (1) forever case (a) 1: a = b; endcase end
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
module top(); logic a, b; initial begin a = b; b = a; end endmodule
)");
    CHECK_FALSE(result);
}

TEST_CASE("OneStatementPerLine: Sibling after nested statement on same line") {
    auto result = runCheckTest("OneStatementPerLine", R"(
module top();
    logic a, b, c;
    initial begin
        if (a)
            b = c; c = b;
    end
endmodule
)");
    CHECK_FALSE(result);
}
