#include "gtest/gtest.h"
#include "parser/parser.h"
#include <iostream>
#include <sstream>
using std::cout;
using std::endl;
using std::stringstream;
using std::string;

/*
 * Helper methods for Empty Statement
 */
void TraverseASTBLock(AST_Block *block);
void TraverseASTSt(AST_Statement *st);

void TraverseASTBLock(AST_Block *block) {
    cout << "{";
    for (AST_Statement *st : block->statements) {
        TraverseASTSt(st);
    }
    cout << "}";
}

void TraverseASTSt(AST_Statement *st) {
    if (!st) {
        cout << ";";
        return;
    }
    if (st->type == ST_BLOCK)
        TraverseASTBLock((AST_Block*)st);
}

/* Print out the AST_EXPRESSION */
void PrintASTExp(AST_Expression *exp);
void PrintASTExpOperand(Operand *o);

void PrintASTExpOperand(Operand *o) {
    if (!o->GetPrefixOP().empty() || !o->GetSuffixOP().empty()) {
        cout << "(";
        for (OP op : o->GetPrefixOP()) {
            switch(op) {
                case OP_ADD:
                    cout << "+";
                    break;
                case OP_SUB:
                    cout << "-";
                    break;
                case OP_INC:
                    cout << "++";
                    break;
                case OP_DEC:
                    cout << "--";
                    break;
                case OP_NOT:
                    cout << "!";
                    break;
                default: ;
            }
        }
    }
    if (o->GetType() == OPRD_LITERAL) {
        Literal *l = (Literal*) o;
        l->GetConst()->Print();
    }
    else {
        Reference* r = (Reference*)o;
        cout << r->GetID();
    }
    if (!o->GetPrefixOP().empty() || !o->GetSuffixOP().empty()) {
        for (OP op : o->GetSuffixOP()) {
            switch(op) {
                case OP_INC:
                    cout << "++";
                    break;
                case OP_DEC:
                    cout << "--";
                    break;
                default: ;
            }
        }
        cout << ")";
    }
}
void PrintASTExp(AST_Expression *exp) {
    if (exp->is_leaf) {
        PrintASTExpOperand(exp->o);
        return;
    }
    if (!exp->e1->is_leaf)
        cout << "(";
    PrintASTExp(exp->e1);
    if (!exp->e1->is_leaf)
        cout << ")";
    switch(exp->op) {
    case OP_ADD:
        cout << "+";
        break;
    case OP_SUB:
        cout << "-";
        break;
    case OP_MUL:
        cout << "*";
        break;
    case OP_DIV:
        cout << "/";
        break;
    default:
        break;
    }
    if (!exp->e2->is_leaf)
        cout << "(";
    PrintASTExp(exp->e2);
    if (!exp->e2->is_leaf)
        cout << ")";
}


/*
 * Test Case
 */
TEST(test_parser, empty_block) {
    Parser parser("../test/test_parser/parser_test1.txt");
    stringstream ss;
    auto buf = cout.rdbuf();
    cout.rdbuf(ss.rdbuf());

    int ns = 16;
    for (int i = 0; i<ns; i++) {
        AST_Statement *st = parser.ParseStatement();
        TraverseASTSt(st);
    }

    cout.rdbuf(buf);
    EXPECT_EQ(";{};{;}{;};;;{}{}{};;;{;}{;{{;};}}", ss.str());
}

TEST(test_parser, declaration) {
    Parser parser("../test/test_parser/parser_test2.txt");

    std::vector<ST> vtest = {ST_FUNC, ST_VAR, ST_VAR, ST_FUNC, ST_FUNC};
    int itest = 0;
    while (true) {
        AST_Statement *st = parser.ParseStatement();
        if (!st) break;
        EXPECT_EQ(vtest[itest++], st->type);
    }
}

TEST(test_parser, simple_expression) {
    Parser parser("../test/test_parser/parser_test3.txt");
    stringstream ss;
    auto buf = cout.rdbuf();
    cout.rdbuf(ss.rdbuf());

    AST_Statement *st;

    // 10+27-16+2.1;
    st = parser.ParseStatement();
    PrintASTExp((AST_Expression*) st);
    cout << endl;
    // e = TraverseASTExp((AST_Expression*) st);
    // EXPECT_EQ(CONST_DOUBLE, e->o.GetType());
    // EXPECT_EQ(23.1, e->o.val.dv);

    // 2*(2+3);
    st = parser.ParseStatement();
    PrintASTExp((AST_Expression*) st);
    cout << endl;
    // e = TraverseASTExp((AST_Expression*) st);
    // EXPECT_EQ(CONST_INT, e->o.GetType());
    // EXPECT_EQ(10, e->o.val.iv);

    // 128 * 4 / 3;
    st = parser.ParseStatement();
    PrintASTExp((AST_Expression*) st);
    cout << endl;
    // e = TraverseASTExp((AST_Expression*) st);
    // EXPECT_EQ(CONST_INT, e->o.GetType());
    // EXPECT_EQ(170, e->o.val.iv);

    // 10 + 128 * 4 - 16 + 10 / 2;
    st = parser.ParseStatement();
    PrintASTExp((AST_Expression*) st);
    cout << endl;
    // e = TraverseASTExp((AST_Expression*) st);
    // EXPECT_EQ(CONST_INT, e->o.GetType());
    // EXPECT_EQ(511, e->o.val.iv);

    cout.rdbuf(buf);
    string str = "((10+27)-16)+2.1\n"
        "2*(2+3)\n"
        "(128*4)/3\n"
        "10+(((128*4)-16)+(10/2))\n";
    EXPECT_EQ(str, ss.str());
}


TEST(test_parser, expression_with_unary_op) {
    Parser parser("../test/test_parser/parser_test5.txt");

    AST_Statement *st;
    string s("(-7)+8\n"
            "5+(-6)\n"
            "(-3)+(+4)\n"
            "(++1----)-(-2++)\n");
    stringstream ss;
    auto buf_bak = cout.rdbuf();
    cout.rdbuf(ss.rdbuf());

    // -7+8;
    st = parser.ParseStatement();
    PrintASTExp((AST_Expression*) st);
    cout << endl;
    // 5+-6;
    st = parser.ParseStatement();
    PrintASTExp((AST_Expression*) st);
    cout << endl;
    // -3++4;
    st = parser.ParseStatement();
    PrintASTExp((AST_Expression*) st);
    cout << endl;
    // ++1------2++;
    st = parser.ParseStatement();
    PrintASTExp((AST_Expression*) st);
    cout << endl;

    s += "(--x)\n"
         "(x++)\n"
         "(-x)\n"
         "(+x)\n"
         "(!true)\n";
    // --x;
    st = parser.ParseStatement();
    PrintASTExp((AST_Expression*) st);
    cout << endl;
    // x++;
    st = parser.ParseStatement();
    PrintASTExp((AST_Expression*) st);
    cout << endl;
    // -x;
    st = parser.ParseStatement();
    PrintASTExp((AST_Expression*) st);
    cout << endl;
    // +x;
    st = parser.ParseStatement();
    PrintASTExp((AST_Expression*) st);
    cout << endl;
    // !true;
    st = parser.ParseStatement();
    PrintASTExp((AST_Expression*) st);
    cout << endl;

    cout.rdbuf(buf_bak);
    EXPECT_EQ(s, ss.str());
}

TEST(test_parser, if_statement) {
    Parser parser("../test/test_parser/parser_test6.txt");

    AST_Statement *st;
    AST_If *stif;

    st = parser.ParseStatement();
    EXPECT_EQ(ST_IF, st->GetType());
    stif = (AST_If*) st;
    EXPECT_EQ(1, stif->GetConds().size());
    EXPECT_EQ(1, stif->GetStams().size());

    st = parser.ParseStatement();
    EXPECT_EQ(ST_IF, st->GetType());
    stif = (AST_If*) st;
    EXPECT_EQ(2, stif->GetConds().size());
    EXPECT_EQ(2, stif->GetStams().size());
}

TEST(test_parser, while_statement) {
    Parser parser("../test/test_parser/parser_test7.txt");

    AST_Statement *st;
    AST_While *sw;

    st = parser.ParseStatement();
    EXPECT_EQ(ST_WHILE, st->GetType());
    sw = (AST_While*) st;
    EXPECT_EQ(ST_EXP, sw->GetBlock()->GetType());

    st = parser.ParseStatement();
    EXPECT_EQ(ST_WHILE, st->GetType());
    sw = (AST_While*) st;
    EXPECT_EQ(ST_BLOCK, sw->GetBlock()->GetType());

    st = parser.ParseStatement();
    EXPECT_EQ(ST_WHILE, st->GetType());
    sw = (AST_While*) st;
    EXPECT_EQ(ST_BREAK, sw->GetBlock()->GetType());
}
