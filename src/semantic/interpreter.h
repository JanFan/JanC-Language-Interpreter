#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <deque>
#include "parser/parser.h"
#include "symbol/symbol.h"
#include "arithmetic.h"
using std::deque;
using std::cerr;

class Interpreter {
public:
    Interpreter() {}

    static void IntrStatement(AST_Statement *st, NestedSymbolTable *sym, FuncTable *fsym);
    static void IntrVar(AST_Statement *st, NestedSymbolTable *sym, FuncTable *fsym);
    static void IntrFunc(AST_Func* func, NestedSymbolTable *sym, FuncTable *fsym);
    static void IntrBlock(AST_Block* block, NestedSymbolTable *sym, FuncTable *fsym);
    static Constant* IntrExpression(AST_Expression* e, NestedSymbolTable *sym, FuncTable *fsym);
    static Constant* UnpackVar(Reference *r, NestedSymbolTable *sym, FuncTable *fsym);
    static Constant* UnpackFunc(RefFunc *r, NestedSymbolTable *sym, FuncTable *fsym);
    static Constant* UnpackArray(RefArray *r, NestedSymbolTable *sym, FuncTable *fsym);
    static Constant* IntrOperand(Operand *o, NestedSymbolTable *sym, FuncTable *fsym);
    static Constant* IntrArrayContent(AST_Array *array, NestedSymbolTable *sym, FuncTable *fsym);

    static Constant* DoBinaryOP(Constant* con1, Constant *con2, OP op);
    static void DoPrefixOP(Operand *o);
    static void DoSuffixOP(Operand *o);
private:
    ;
};

#endif
