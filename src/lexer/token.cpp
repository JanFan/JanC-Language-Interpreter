#include "token.h"
using namespace std;

Lexer::Lexer(string path_to_file) {
    in.open(path_to_file);
    cur = in.get();

    for (auto p : Key_Words) {
        mkey.insert(p);
    }
}

Lexer::~Lexer() {
    in.close();
}

OP Lexer::GetCurOP() {
    return ope.op;
}

TOKEN Lexer::GetNextToken() {
    lexem_buf[0] = '\0';
    while(cur != EOF && isspace(cur)) cur = in.get();
    if (cur == EOF) return TOK_END;

    TOKEN t = TOK_START;
    int op_table_index = 0;
    int ibuf = 0;

    while(true) {
        if (cur == EOF) goto done;

        switch(t) {
        case TOK_START: /* start state */
            if (isdigit(cur))
                t = TOK_INT;
            else if (cur == '.')
                t = TOK_DOUBLE;
            else if (isalpha(cur) || cur == '_')
                t = TOK_ID;
            else if (Delim.find(cur) != string::npos) {
                lexem_buf[ibuf++] = cur;
                cur = in.get();
                goto key_word;
            }
            else if (cur == '"') {
                t = TOK_STRING;
                cur = in.get();
                continue;
            }
            else if (strchr(OP_Char_Set, cur)) {
                t = TOK_OP;
                for (OPEntry e : OP_Action_Table[op_table_index]) {
                    if (e.c == OPEntry::EMPTY_OP) break;
                    if (cur == e.c) {
                        ope = e;
                        break;
                    }
                }
                op_table_index++;
            }
            else ;
            break;
        case TOK_ID:
            if (isdigit(cur) || isalpha(cur) || cur == '_')
                ;
            else if (isspace(cur) || Delim.find(cur) != string::npos)
                goto key_word;
            else goto bad_syntax;
            break;
        case TOK_INT:
            if (cur == '.')
                t = TOK_DOUBLE;
            else if (isdigit(cur))
                ;
            else if (isspace(cur) || Delim.find(cur) != string::npos)
                goto done;
            else goto bad_syntax;
            break;
        case TOK_DOUBLE:
            if (isdigit(cur))
                ;
            else if (isspace(cur) || Delim.find(cur) != string::npos)
                goto done;
            else goto bad_syntax;
            break;
        case TOK_STRING:
            if (cur == '\\') { // For escape characters
                cur = in.get();
                lexem_buf[ibuf++] = cur;
                cur = in.get();
                continue;
            }
            else if (cur == '"') {
                cur = in.get(); // eat the ending '"'
                goto done;
            }
            else ;
            break;
        case TOK_OP:
            {
                bool is_match = false;
                for (int ind : ope.nxt) {
                    OPEntry e = OP_Action_Table[op_table_index][ind];
                    if (e.c == OPEntry::EMPTY_OP) break;
                    if (cur == e.c) {
                        ope = e;
                        is_match = true;
                        cur = in.get(); // don't forget to eat the op char
                        break;
                    }
                }
                if (ope.op != OP_NONE && (ope.nxt.empty() || !is_match))
                    goto done;
                else if (!ope.nxt.empty() && is_match)
                    op_table_index++;
                else
                    goto bad_syntax;
            }
            break;
        default:
            break;
        }
        lexem_buf[ibuf++] = cur;
        cur = in.get();
    }

done:
    lexem_buf[ibuf] = '\0';
    return t;
key_word:
    lexem_buf[ibuf] = '\0';
    if (mkey.find(lexem_buf) != mkey.end())
        return mkey[lexem_buf];
    else return t;
bad_syntax:
    cout << "bad syntax in state " << t << " : " << (char)cur << endl;
    return TOK_BAD_TOKEN;
}

string Lexer::GetCurLexem() {
    return string(lexem_buf);
}

TOKEN Lexer::NextNum() {
    static const char *deli = " \t\n";
    TOKEN t = TOK_INT;
    int ibuf = 0;
    while(NULL != strchr(deli, cur)) {
        if (cur == '.')
            t = TOK_DOUBLE;
        lexem_buf[ibuf++] = cur;
        cur = in.get();
    }

    return t;
}

bool Lexer::Eat(char c) {
    if (c == cur)
        cur = in.get();

    return false;
}

void Lexer::Eat() {
    cur = in.get();
}
