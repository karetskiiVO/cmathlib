#include "cmathlib.h"

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#define isOpr(_node,_op_type) if ((_node)->type == NODE_OPR && (_node)->nodeop == (_op_type))
#define tex_breket(_node,_side,_brek)   if (node->nodeop != OP_div && node->nodeop != OP_pow && (_node)->_side->type == NODE_OPR && (int)((_node)->_side->nodeop / 10) < (int)((_node)->nodeop / 10)) \
                                    fprintf(file, "%s", _brek)

static void skipSpaces (const char* *ptr);

static Func_node* newNode    (Func_node n_node);
static Func_node* newNodePrt (OP_TYPE type, Func_node* node1, Func_node* node2);
static Func_node* newNodeOp  (OP_TYPE type);
static Func_node* newNodeCst (double val);

static Func_node* getN (const char* *ptr, List* varlist);
static Func_node* getP (const char* *ptr, List* varlist);
static Func_node* getL (const char* *ptr, List* varlist);
static Func_node* getT (const char* *ptr, List* varlist);
static Func_node* getE (const char* *ptr, List* varlist);
static Func_node* getG (const char* str , List* varlist);

static double eqValue (const Func_node* node, const List* varlist);

static void graph_dumpEq_node (const Func_node* node, List* varlist);
static void graph_dumpEq_edge (const Func_node* node);

static void latex_dumpEq (const Func_node* node, List* varlist);

static void postordDump  (const Func_node* node);

static Func_node* Eqcopy (const Func_node* origin_node);
static Func_node* difFunc_eq (const Func_node* func, const size_t varind);



static const char* opPrint (OP_TYPE op);

Function* getFunc (const char* str) {
    Function* val = (Function*)calloc(1, sizeof(Function));
    val->variables = (List*)calloc(1, sizeof(List));
    listNew(val->variables);
    val->equation = getG(str, val->variables);
    return val;
}

void FunctionDump (Function* func, const char* format) {
    static size_t texcall = 0;
    if (!strcmp(format, "graphviz")) {
        FILE* file = fopen("eq.dot", "w");

        fprintf(file, "digraph g {\n{\n\t\tnode [shape=record];\n");
        fclose(file);
        
        graph_dumpEq_node(func->equation, func->variables);
        file = fopen("eq.dot", "a");
        fprintf(file, "\t}\n");
        fclose(file);
        graph_dumpEq_edge(func->equation);

        file = fopen("eq.dot", "a");
        fprintf(file, "}\n");
        fclose(file);

        system("dot -Tpng eq.dot > pic.png");
    }
    if (!strcmp(format, "tex")) {
        FILE* file = fopen("eq.tex", "a");
        if (texcall == 0) {
            fclose(file);
            file = fopen("eq.tex", "w");
            fprintf(file, "\\documentclass[12pt]{article} %%size of characters\n");
            fprintf(file, "\\usepackage{cmap} %%for copying Russian text\n");
            fprintf(file, "\\usepackage{amssymb}\n");
            fprintf(file, "\\usepackage[utf8]{inputenc} %%use UTF-8\n");
            fprintf(file, "\\usepackage[english, russian]{babel} %%use Russian\n");
            fprintf(file, "\\usepackage[a4paper, total={7in, 10in}]{geometry}\n");
            fprintf(file, "\\usepackage{mathtext}\n");
            fprintf(file, "\\usepackage{multirow}\n");
            fprintf(file, "\\usepackage{amsmath}\n");
            fprintf(file, "\\usepackage{amsfonts}\n");
            fprintf(file, "\\begin{document}\n");
        }
        texcall++;

        fprintf(file, "$$");
        fclose(file);

        latex_dumpEq(func->equation, func->variables);

        file = fopen("eq.tex", "a");
        fprintf(file, "$$\n");
        fclose(file);
    }
    if (!strcmp(format, "texcompile")) {
        FILE* file = fopen("eq.tex", "a");
        fprintf(file, "\\end{document}\n");
        fclose(file);
        system("pdflatex eq.tex");
    }

    if (!strcmp(format, "postord")) {
        postordDump(func->equation);

        FILE* file = fopen("pstord.txt", "a");
        fprintf(file, "\n");
        fclose(file);
    }
    ///////////////

}

static void graph_dumpEq_node (const Func_node* node, List* varlist) {
    FILE* file = fopen("eq.dot", "a"); /// change

    fprintf(file, "\t\tstruct%p", node);
    switch (node->type) {
        case NODE_OPR: 
            fprintf(file, "[label=\" %s \"",  opPrint(node->nodeop));
            break;
        case NODE_CST:
            fprintf(file, "[label=\" %lg \"", node->value);
            break;
        case NODE_VAR:
            fprintf(file, "[label=\" var_%ld: %s \"", node->varind,
                            varlist->arr[node->varind].value.name);
            break;
        case NODE_EMPTY:
            break;
    }
    fprintf(file, " color=\"olivedrab1\"]\n");
    fclose(file);

    if (node->left ) graph_dumpEq_node(node->left , varlist); 
    if (node->right) graph_dumpEq_node(node->right, varlist); 
}

static void graph_dumpEq_edge (const Func_node* node) {
    FILE* file = fopen("eq.dot", "a"); /// change

    if (node->left ) fprintf(file, "\tstruct%p -> struct%p\n", node, node->left);
    if (node->right) fprintf(file, "\tstruct%p -> struct%p\n", node, node->right);

    fclose(file);

    if (node->left ) graph_dumpEq_edge(node->left); 
    if (node->right) graph_dumpEq_edge(node->right); 
}

static const char* opPrint (OP_TYPE op) {
    switch (op) {
        case OP_add:  return "+";
        case OP_sub:  return "-";
        case OP_mul:  return "*";
        case OP_div:  return "/";
        case OP_pow:  return "^";
        case OP_cos:  return "cos";
        case OP_sin:  return "sin";
        case OP_log:  return "log";
        case OP_tan:  return "tan";
        case OP_sinh: return "sinh";
        case OP_cosh: return "cosh";
        case OP_tanh: return "cosh";
        default:     return NULL;
    }
}

static Func_node* getN (const char* *ptr, List* varlist) {
    skipSpaces(ptr);
    Func_node* node = newNode(F_EMPTY);

    char var[100] = "";
    double val = 0;
    int    pos = 0;
    if (sscanf(*ptr, "%lf%n", &val, &pos)) {
        node->type  = NODE_CST;
        node->value = val;
    } else if (sscanf(*ptr, "%[^^()+-=/* \t\n]",  var)) {
        node->type  = NODE_VAR;
        var_t buf;
        pos = strlen(var);
        buf.name = (char*)calloc(pos + 1, sizeof(char));
        buf.val  = NAN;
        strcpy(buf.name, var);

        node->varind = listSearch(varlist, buf.name);
        if (node->varind == EMPTY) {
            node->varind = listAdd(varlist, 0, buf);
        }
    }

    assert(pos);
    *ptr += pos;

    return node;
}

#define isfunc(_fnc_str, _fnc)          \
else if (!strcmp(_fnc_str, #_fnc)) {     \
    val = newNodeOp(OP_##_fnc);    \
    *ptr += pos;                \
    val->right = getP(ptr, varlist);    \
}

static Func_node* getP (const char* *ptr, List* varlist) {
    skipSpaces(ptr);
    Func_node* val = NULL;
    size_t pos = 0;
    char fnc[100] = "";

    sscanf(*ptr, "%[^ ()]%ln", fnc, &pos);
    if (false);
    isfunc(fnc, sin)
    isfunc(fnc, cos)
    isfunc(fnc, log)
    isfunc(fnc, tan)
    isfunc(fnc, sinh)
    isfunc(fnc, cosh)
    isfunc(fnc, tanh)
    else {
        if (**ptr == '(') {
            (*ptr)++;
            val = getE(ptr, varlist);
            skipSpaces(ptr);
            assert(**ptr == ')');
            (*ptr)++;
        } else {
            val = getN(ptr, varlist);
        }
    }
    return val;
}

#undef isfunc

static Func_node* getL (const char* *ptr, List* varlist) {
    Func_node* val = getP(ptr, varlist);
    Func_node* buf = val;

    skipSpaces(ptr);
    while (**ptr == '^') {
        (*ptr)++;

        val = newNode(F_EMPTY);
        val->left  = buf;
        val->right = getP(ptr, varlist);

        val->type   = NODE_OPR;
        val->value  = NAN;
        val->varind = 0;
        val->nodeop = OP_pow;

        buf = val;
    }
    return val;
}

static Func_node* getT (const char* *ptr, List* varlist) {
    Func_node* val = getL(ptr, varlist);
    Func_node* buf = val;

    skipSpaces(ptr);
    while (**ptr == '*' || **ptr == '/') {
        char com = **ptr;
        (*ptr)++;

        val = newNode(F_EMPTY);
        val->left  = buf;
        val->right = getL(ptr, varlist);

        val->type   = NODE_OPR;
        val->value  = NAN;
        val->varind = 0;

        if (com == '*') {
            val->nodeop = OP_mul;
        } else {
            val->nodeop = OP_div;
        }

        buf = val;
    }
    return val;
}

static Func_node* getE (const char* *ptr, List* varlist) {
    Func_node* val = getT(ptr, varlist);
    Func_node* buf = val;

    skipSpaces(ptr);
    while (**ptr == '+' || **ptr == '-') {
        char com = **ptr;
        (*ptr)++;

        val = newNode(F_EMPTY);
        val->left  = buf;
        val->right = getT(ptr, varlist);

        val->type   = NODE_OPR;
        val->value  = NAN;
        val->varind = 0;

        if (com == '+') {
            val->nodeop = OP_add;
        } else {
            val->nodeop = OP_sub;
        }

        buf = val;
    }
    return val;
}

static Func_node* getG (const char* str,  List* varlist) {
    const char* *ptr = &str;
    Func_node* val = getE(ptr, varlist);
    skipSpaces(ptr);
    return val;
} 

static Func_node* newNode (Func_node n_node) {
    Func_node* node = (Func_node*)calloc(1, sizeof(Func_node));
    *node = n_node;
    node->left  = NULL;
    node->right = NULL;
    return node;
}

static Func_node* newNodePrt (OP_TYPE type, Func_node* node1, Func_node* node2) {
    Func_node* val = (Func_node*)calloc(1, sizeof(Func_node));
    
    *val = F_EMPTY;

    val->left  = node1;
    val->right = node2;
    val->type  = NODE_OPR;
    val->nodeop = type;

    return val;
}

static Func_node* newNodeOp (OP_TYPE type) {
    Func_node* node = newNode(F_EMPTY);

    node->type   = NODE_OPR;
    node->nodeop = type;

    return node;
}

static Func_node* newNodeCst (double val) {
    Func_node* node = newNode(F_EMPTY);

    node->type  = NODE_CST;
    node->value = val;

    return node;
}

static void skipSpaces (const char* *ptr) {
    while (strchr(" \t\n\r", **ptr)) (*ptr)++;
}

void setVarValue_ (List* varlist, const char* var_name, double val) {
    size_t pos = listSearch(varlist, var_name);
    if (pos == EMPTY) {
        var_t buf = {0};
        size_t len = strlen(var_name);
        buf.name = (char*)calloc(len + 1, sizeof(char));
        strcpy(buf.name, var_name);
        buf.val = val;

        listAdd(varlist, 0, buf);
        return;
    }

    varlist->arr[pos].value.val = val;
}

static Func_node* Eqcopy (const Func_node* origin_node) {
    Func_node* val = newNode(*origin_node);

    if (origin_node->left)  val->left  = Eqcopy(origin_node->left);
    if (origin_node->right) val->right = Eqcopy(origin_node->right);

    return val;
}

static void latex_dumpEq (const Func_node* node, List* varlist) {
    FILE* file = fopen("eq.tex", "a");
    if (node->type == NODE_VAR) {
        fprintf(file, " %s ", varlist->arr[node->varind].value.name);
        fclose(file);
        return;
    }
    if (node->type == NODE_CST) {
        if (node->value < 0) fprintf(file, "(");
        fprintf(file, " %lg ", node->value);
        if (node->value < 0) fprintf(file, ")");
        fclose(file);
        return;
    }

    
    isOpr (node, OP_div) {
        fprintf(file, "\\frac");
    }
    if (node->left)  {
        tex_breket(node, left, "\\left(");
        fprintf(file, "{");
        fclose(file);
        latex_dumpEq(node->left, varlist);
        file = fopen("eq.tex", "a");
        fprintf(file, "}");
        tex_breket(node, left, "\\right)");
    }

    switch (node->nodeop) {
        case OP_add:
            fprintf(file, " + ");
            break;
        case OP_sub:
            fprintf(file, " - ");
            break;
        case OP_mul:
            fprintf(file, " \\cdot ");
            break;
        case OP_pow:
            fprintf(file, "^");
            break;
        case OP_cos:
            fprintf(file, "cos \\left(");
            break;
        case OP_sin:
            fprintf(file, "sin \\left(");
            break;
        case OP_log:
            fprintf(file, "log \\left(");
            break;
        case OP_tan:
            fprintf(file, "tan \\left(");
            break;
        case OP_sinh:
            fprintf(file, "sinh \\left(");
            break;
        case OP_cosh:
            fprintf(file, "cosh \\left(");
            break;
        case OP_tanh:
            fprintf(file, "tanh \\left(");
            break;
        default:
            break;
    }

    if (node->right) {
        tex_breket(node, right, "\\left(");
        fprintf(file, "{");
        fclose(file);
        latex_dumpEq(node->right, varlist);
        file = fopen("eq.tex", "a");
        fprintf(file, "}");
        tex_breket(node, right, "\\right)");
    }

    switch (node->nodeop) {
        case OP_cos:
            fprintf(file, "\\right)");
            break;
        case OP_sin:
            fprintf(file, "\\right)");
            break;
        case OP_log:
            fprintf(file, "\\right)");
            break;
        case OP_tan:
            fprintf(file, "\\right)");
            break;
        case OP_sinh:
            fprintf(file, "\\right)");
            break;
        case OP_cosh:
            fprintf(file, "\\right)");
            break;
        case OP_tanh:
            fprintf(file, "\\right)");
            break;
        default:
            break;
    }

    fclose(file);
}

static void postordDump  (const Func_node* node) {
    if (node->left)  postordDump(node->left);
    if (node->right) postordDump(node->right);

    FILE* file = fopen("pstord.txt", "a");

    switch (node->type) {
        case NODE_OPR:
            fprintf(file, "%s", opPrint(node->nodeop));
            break;
        case NODE_CST:
            fprintf(file, "%d", (int)round(node->value));
            break;
        default:
            break;
    }

    fclose(file);
}

static double eqValue (const Func_node* node, const List* varlist) {
    #define L eqValue(node->left,  varlist)
    #define R eqValue(node->right, varlist)

    switch (node->type) {
    case NODE_CST:
        return node->value;
    case NODE_VAR:
        return varlist->arr[node->varind].value.val;
    case NODE_OPR:
        switch (node->nodeop) {
            case OP_add:  return L + R;
            case OP_sub:  return L - R;
            case OP_div:  return L / R;
            case OP_mul:  return L * R;
            case OP_pow:  return pow(L, R);
            case OP_log:  return log(R);
            case OP_sin:  return sin(R);
            case OP_cos:  return cos(R);
            case OP_tan:  return tan(R);
            case OP_sinh: return sinh(R);
            case OP_cosh: return cosh(R);
            case OP_tanh: return tanh(R);
            case OP_EMPTY:return NAN;
        }
    case NODE_EMPTY:
        return NAN;
    }

    #undef L
    #undef R
}

double FuncValue (const Function* func) {
    return eqValue(func->equation, func->variables);
}

static Func_node* difFunc_eq (const Func_node* func, const size_t varind) {
    #define D(_side) difFunc_eq(func->_side, varind)
    #define C(_side) Eqcopy(func->_side)
    #define N(_op, _l, _r) newNodePrt(OP_##_op, _l, _r)
    #define Cst(_num) newNodeCst(_num)
    #define l left
    #define r right

    switch (func->type) {
        case NODE_CST: return newNodeCst(0);
        case NODE_VAR: 
            if (varind == func->varind) {
                return newNodeCst(1);
            } 
            return newNodeCst(0);
        case NODE_EMPTY:
            return NULL;
        case NODE_OPR:
        switch (func->nodeop) {
            case OP_add:
                return N(add, D(l), D(r));
            case OP_sub:
                return N(sub, D(l), D(r));
            case OP_mul:
                return N(add, N(mul, D(l), C(r)), N(mul, C(l), D(r)));
            case OP_div:
                return N(div, N(sub, N(mul, D(l), C(r)), N(mul, C(l), D(r))), N(pow, C(r), Cst(2)));
            case OP_pow:
                return N(mul, N(pow, C(l), C(r)), N(add, N(mul, N(div, D(l), C(l)), C(r)), N(mul, D(r), N(log, NULL, C(l)))));
            case OP_log:
                return N(div, D(r), C(r));
            case OP_sin:
                return N(mul, D(r), N(cos, NULL, C(r)));
            case OP_cos:
                return N(mul, D(r), N(mul, Cst(-1), N(cos, NULL, C(r))));
            case OP_tan:
                return N(div, D(r), N(pow, N(cos, NULL, C(r)), Cst(2)));
            case OP_sinh:
                return N(mul, D(r), N(cosh, NULL, C(r)));
            case OP_cosh:
                return N(mul, D(r), N(sinh, NULL, C(r)));
            case OP_tanh:
                return N(div, D(r), N(pow, N(cosh, NULL, C(r)), Cst(2)));
            case OP_EMPTY:
                return NULL;
        }
    }

    #undef Cst
    #undef D
    #undef C
    #undef N
    #undef l
    #undef r
}

Function* difFunc_ (const Function* func, const char* var_name) {
    Function* derivative = (Function*)calloc(1, sizeof(Function));
    //!!!!!!!!
    derivative->variables = func->variables;
    derivative->equation  = difFunc_eq(func->equation,
                                listSearch(func->variables, var_name));
    
    return derivative;    
}

