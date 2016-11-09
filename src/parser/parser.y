%{
#include "analyzer/node.h"
NBlock *programBlock; /* the top level root node of our final AST */

extern int yylex();
extern int yyget_lineno();
void yyerror(const char *s) { printf("ERROR: %sn", s); }
%}
/* for more error verbose */
%error-verbose
%debug

/* Represents the many different ways we can access our data */
%union {
    Node *node;
    NBlock *block;
    NExpression *expr;
    NStatement *stmt;
    NIdentifier *ident;
    NVariableDeclaration *var_decl;
    std::vector<NIdentifier*> *varvec;
    std::vector<NExpression*> *exprvec;
    std::string *string;
    int token;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */
%token <string> TIDENTIFIER TINTEGER
%token <token> TCEQ TCNE TEQUAL RETURN FUNC
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TCOMMA TDOT

/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (NIdentifier*). It makes the compiler happy.
 */
%type <ident> ident
%type <expr> numeric expr
%type <varvec> func_decl_args
%type <exprvec> call_args
%type <block> program stmts block
%type <stmt> stmt var_decl func_decl
%type <token> comparison

/* Operator precedence for mathematical operators */
%left TPLUS TMINUS
%left TMUL TDIV

%start program

%%

program : stmts { programBlock = $1; }
;

stmts : stmt { $$ = new NBlock(); $$->statements.push_back($<stmt>1); }
| stmts stmt { $1->statements.push_back($<stmt>2); }
;


stmt : var_decl | func_decl
| expr { $$ = new NExpressionStatement(*$1); }
| RETURN ident {$$ = new NReturnStatement(*$2); }
;

block : TLBRACE stmts TRBRACE { $$ = $2; }
| TLBRACE TRBRACE { $$ = new NBlock(); }
;

var_decl : ident { $$ = new NVariableDeclaration(*$1); }
| ident TEQUAL expr { $$ = new NVariableDeclaration(*$1, $3); }
;

func_decl : FUNC ident TLPAREN func_decl_args TRPAREN block
{ $$ = new NFunctionDeclaration(*$2, *$4, *$6); delete $4; }
;

func_decl_args : /*blank*/  { $$ = new VariableList(); }
| ident { $$ = new VariableList(); $$->push_back($<ident>1); }
| func_decl_args TCOMMA ident { $1->push_back($<ident>3); }
;

ident : TIDENTIFIER { $$ = new NIdentifier(*$1); delete $1; }
| TIDENTIFIER TDOT TIDENTIFIER { $$ = new NIdentifier(*$1, *$3); delete $1; delete $3;}
;

numeric : TINTEGER { $$ = new NInteger(atol($1->c_str())); delete $1; }
;

/* TODO: some issues with lineno */
expr : ident TEQUAL expr { $$ = new NAssignment(*$<ident>1, *$3); }
| ident TLPAREN call_args TRPAREN { $$ = new NMethodCall(*$1, *$3); delete $3; }
| ident { $<ident>$ = $1; }
| numeric
| ident comparison ident { $$ = new NBinaryOperator(*$1, $2==TCEQ, *$3, yyget_lineno()); }
| TLPAREN expr TRPAREN { $$ = $2; }
;

call_args : /*blank*/  { $$ = new ExpressionList(); }
| expr { $$ = new ExpressionList(); $$->push_back($1); }
| call_args TCOMMA expr  { $1->push_back($3); }
;

comparison : TCEQ | TCNE
;

%%