#include "interpreter.h"

/* Interpreter Start Point */
void Interpreter::IntrStatement (AST_Statement *st, NestedSymbolTable *sym, FuncTable *fsym) {
    if (!st)
        return;
    switch(st->type) {
    case ST_BLOCK:
        IntrBlock((AST_Block*)st, sym, fsym);
        break;
    case ST_EXP:
        {
        Constant *con = IntrExpression((AST_Expression*)st, sym, fsym);
        if (con) {
            con->Print();
            cout << endl;
        }
        }
        break;
    case ST_FUNC:
        IntrFunc((AST_Func *)st, sym, fsym);
        break;
    case ST_VAR:
    case ST_ARRAY:
        IntrVar(st, sym, fsym);
        break;
    default:
        cout << "Error in IntrStatement: wrong type for default" << endl;
        exit(0);
    };
}

void Interpreter::IntrBlock(AST_Block* block, NestedSymbolTable *sym, FuncTable *fsym) {
    sym->NewSymbolTable();
    for (AST_Statement *st : block->statements) {
        IntrStatement(st, sym, fsym);
    }
    sym->DelSymbolTable();
}

Constant* Interpreter::IntrArrayContent(AST_Array *array, NestedSymbolTable *sym, FuncTable *fsym) {
    if (array->ve.empty())
        return NULL;

    Array *arr = new Array(array->sz_array);
    for (AST_Expression* e : array->ve) {
        Constant *_con = IntrExpression(e, sym, fsym);
        arr->AddElement(_con);
    }
    return arr;
}

void Interpreter::IntrVar(AST_Statement *st, NestedSymbolTable *sym, FuncTable *fsym) {
    if (st->GetType() == ST_ARRAY) {
        AST_Array *array = (AST_Array*) st;
        Constant* val = IntrArrayContent(array, sym, fsym);
        if (sym->GetCurSymbolTable()->IsSymbolDefined(array->id)) {
            cerr << "Error in IntrVar: symbol " << array->id << " has been defined" << endl;
            exit(0);
        }
        sym->GetCurSymbolTable()->AddSymbol(array->id, val);
    }
    else {
        AST_Var *var = (AST_Var*) st;
        if (sym->GetCurSymbolTable()->IsSymbolDefined(var->id)) {
            cerr << "Error in IntrVar: symbol " << var->id << " has been defined" << endl;
            exit(0);
        }
        Constant *val = NULL;
        if (var->val) {
            val = IntrExpression(var->val, sym, fsym);
        }
        else {
            cerr << "Error in IntrVar: symbol " << var->id << " not initialized" << endl;
            exit(0);
        }
        sym->GetCurSymbolTable()->AddSymbol(var->id, val);
    }
}

void Interpreter::IntrFunc(AST_Func* func, NestedSymbolTable *sym, FuncTable *fsym) {
    fsym->AddSymbol(func->id, func);
}

Constant* Interpreter::DoBinaryOP(Constant* con1, Constant *con2, OP op) {
    switch(op) {
    /* arithmetic */
    case OP_ADD:
        return ((Arithmetic*)con1)->Add((Arithmetic*)con2);
    case OP_SUB:
        return ((Arithmetic*)con1)->Sub((Arithmetic*)con2);
    case OP_MUL:
        return ((Arithmetic*)con1)->Mul((Arithmetic*)con2);
    case OP_DIV:
        return ((Arithmetic*)con1)->Div((Arithmetic*)con2);
    /* relational */
    case OP_EQ:
        return ((Arithmetic*)con1)->EQ((Arithmetic*)con2);
    case OP_NOT_EQ:
        return ((Arithmetic*)con1)->NEQ((Arithmetic*)con2);
    case OP_GT:
        return ((Arithmetic*)con1)->GT((Arithmetic*)con2);
    case OP_LT:
        return ((Arithmetic*)con1)->LT((Arithmetic*)con2);
    case OP_GT_EQ:
        return ((Arithmetic*)con1)->GTEQ((Arithmetic*)con2);
    case OP_LT_EQ:
        return ((Arithmetic*)con1)->LTEQ((Arithmetic*)con2);
    /* logical */
    case OP_AND:
        return ((Bool*)con1)->And((Bool*)con2);
    case OP_OR:
        return ((Bool*)con1)->Or((Bool*)con2);
    default: ;
    }
    return NULL;
}

Constant* Interpreter::IntrExpression(AST_Expression* exp, NestedSymbolTable *sym, FuncTable *fsym) {
    if (exp->IsLeaf()) {
        return IntrOperand(exp->o, sym, fsym);
    }
    // For assgiment
    if (exp->op == OP_ASSIGN) {
        AST_Expression *lv = exp->e1;
        if (!lv->IsLeaf() || lv->o->GetType() != OPRD_REF) {
            cerr << "Error in IntrExpression: l-value must be assignable" << endl;
            exit(0);
        }
        Reference *r = (Reference*)(lv->o);
        Constant* rv = IntrExpression(exp->e2, sym, fsym);
        sym->GetCurSymbolTable()->ChangeSymbol(r->GetID(), rv);
        return rv;
    }
    // Normal recursive process
    Constant* con1 = IntrExpression(exp->e1, sym, fsym);
    Constant* con2 = IntrExpression(exp->e2, sym, fsym);
    return DoBinaryOP(con1, con2, exp->op);
}

Constant* Interpreter::IntrOperand(Operand *o, NestedSymbolTable *sym, FuncTable *fsym) {
    if (o->GetType() == OPRD_LITERAL) {
        Literal *l = (Literal*) o;
        return l->GetConst();
    }
    else {
        Reference *r = (Reference*) o;
        // var
        if (sym->GetCurSymbolTable()->IsSymbolDefinedRecursively(r->GetID()))
            return UnpackVar(r, sym, fsym);
        // func
        else if (fsym->LookupSymbol(r->GetID()))
            return UnpackFunc((RefFunc*)r, sym, fsym);
        else {
            cerr << "Error in IntrOperand: symbol not defined- " << r->GetID() << endl;
            exit(0);
        }
    }
    return NULL;    /* never been here */
}

Constant* Interpreter::UnpackArray(RefArray *r, NestedSymbolTable *sym, FuncTable *fsym) {
    AST_Expression *e = r->GetIndex();
    Constant *cindex = IntrExpression(e, sym, fsym);
    if (cindex->GetType() != CONST_INT) {
        cerr << "Error in UnpackVar: array index must be Int- " << cindex->GetType() << endl;
        exit(0);
    }
    int index = ((Int*)cindex)->GetInt();
    return ((Array*)cindex)->At(index);
}

Constant* Interpreter::UnpackVar(Reference *r, NestedSymbolTable *sym, FuncTable *fsym) {
    Constant* con = sym->GetCurSymbolTable()->LookupSymbol(r->GetID());
    if (r->GetType() == OPRD_REFARRAY)
        return UnpackArray((RefArray*)r, sym, fsym);
    // single var
    return con;
}

Constant* Interpreter::UnpackFunc(RefFunc *r, NestedSymbolTable *sym, FuncTable *fsym) {
    if (!fsym->IsSymbolDefined(r->GetID())) {
        cerr << "Error in UnpackFunc: symbol " << r->GetID() << " not defined" << endl;
        exit(0);
    }
    // sym->NewFuncSymbolTable();
    sym->NewSymbolTable();
    Constant *res = NULL;
    AST_Func *func = (AST_Func*)(fsym->LookupSymbol(r->GetID()));
    for (AST_Statement *st : func->block->statements) {
        if (st->GetType() != ST_RETURN) {
            IntrStatement(st, sym, fsym);
            continue;
        }
        // return
        AST_Return *rt = (AST_Return*)st;
        res = IntrExpression(rt->e, sym, fsym);
        break;
    }
    // sym->DelFuncSymbolTable();
    sym->DelSymbolTable();
    return res;
}

