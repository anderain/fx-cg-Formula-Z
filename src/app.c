#include "platform.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "graphProvider.h"
#include "inputProvider.h"
#include "fontProvider.h"

#define STREQ(s1, s2) (((s1) != NULL) && ((s2) != NULL) && strcmp((s1), (s2)) == 0)
#define DEBUG_MSG(s) // puts(s)

#define is_digit(c) ((c) >= '0' && (c) <= '9')
#define is_alpha(c) ((c) >= 'a' && (c) <= 'z' || (c) >= 'A' && (c) <= 'Z')
#define is_alnum(c) ((is_digit(c)) || (is_alpha(c)))
#define is_space(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == 'r')

#ifdef APP_FXCG
#   define puts //
#   define putchar //
#   define printf //
#   define max(a, b) ((a) > (b) ? (a) : (b))
#endif

typedef enum {
    ERR, END, NUM, SYM, OPR, FNC, BKT, UDF
} token_type_t;

const char *DBG_TOKEN_NAME[] = {
    "ERR", "END", "NUM", "SYM", "OPR", "FNC", "BKT", "UDF"
};

typedef struct tag_token_t {
    token_type_t type;
    char content[20];
} token_t;

typedef struct tag_expr_node_t {
    token_type_t type;
    char content[20];
    struct tag_expr_node_t *lchild, *rchild;
} expr_node_t;

token_t* set_token(token_t *token, token_type_t type, const char *content) {
    token->type = type;
    strcpy(token->content, content);
    return token;
}

typedef struct tag_analyzer_t {
    const char *expr;
    const char *eptr;
    token_t token;
} analyzer_t;

typedef struct tag_vstack_t {
    int max_size;
    int size;
    void **data;
} vstack_t;

vstack_t* vs_new_stack(int max_size) {
    vstack_t *s = (vstack_t *)malloc(sizeof(vstack_t));

    s->max_size = max_size;
    s->size = 0;
    s->data = (void **)malloc(max_size * sizeof(void *));

    return s;
}

void vs_free(vstack_t *s) {
    if (s == NULL)
        return;
    if (s->data != NULL)
        free(s->data);
    free(s);
}

void vs_push(vstack_t *_self, void *data) {
    if (_self->size >= _self->max_size) {
        DEBUG_MSG("vstack : push overflow");
        return;
    }
    _self->data[_self->size++] = data;
}

void* vs_pop(vstack_t *_self) {
    if (_self->size <= 0) {
        DEBUG_MSG("vstack : pop overflow");
        return NULL;
    }
    return _self->data[--_self->size];
}

void* vs_top(vstack_t *_self) {
    if (_self->size <= 0) {
        DEBUG_MSG("vstack : top overflow");
        return NULL;
    }
    return _self->data[_self->size - 1];
}

int vs_empty(vstack_t *_self) {
    return _self->size <= 0;
}

typedef struct tag_vlist_node_t {
    struct tag_vlist_node_t *prev, *next;
    void *data;
} vlist_node_t;

typedef struct {
    vlist_node_t *head, *tail;
    int size;
} vlist_t;

vlist_node_t* vln_new_node(void *data) {
    vlist_node_t* n = (vlist_node_t *)malloc(sizeof(vlist_node_t));
    n->prev = n->next = NULL;
    n->data = data;
    return n;
}

vlist_t* vl_new_list() {
    vlist_t *l = (vlist_t *)malloc(sizeof(vlist_t));
    l->head = l->tail = NULL;
    l->size = 0;
    return l;
}

vlist_t* vl_push_back(vlist_t* _self, void *data) {
    vlist_node_t *new_node = vln_new_node(data);

    if (_self->head == NULL) {
        _self->head = _self->tail = new_node;
    }
    else {
        vlist_node_t * tail = _self->tail;
        tail->next = new_node;
        new_node->prev = tail;
        _self->tail = new_node;
    }

    _self->size++;

    return _self;
}

void* vl_pop_front(vlist_t* _self) {
    vlist_node_t *head;
    void *data;

    head = _self->head;

    if (head == NULL) return NULL;

    data = head->data;

    _self->head = head->next;
    if (_self->head) {
        _self->head->prev = NULL;
    }
    else {
        _self->tail = NULL;
    }

    free(head);
    _self->size--;

    return data;
}

void vl_destory(vlist_t* _self) {
    vlist_node_t *n1, *n2;

    n1 = _self->head;
    while (n1) {
        n2 = n1->next;
        free(n1);
        n1 = n2;
    }

    free(_self);
}

typedef vlist_t vqueue_t;
#define vq_new_queue vl_new_list
#define vq_push vl_push_back
#define vq_pop vl_pop_front
#define vq_empty(q) ((q)->size <= 0)
#define vq_destory(q) vl_destory(q)

char *STR_DUMP(const char *str) {
    char *buffer = (char *)malloc(strlen(str) + 1);
    strcpy(buffer, str);
    return buffer;
}

token_t* get_token(analyzer_t *_self) {
    char first_char;
    char buffer[100];
    char *pbuffer = buffer;

    while (is_space(*_self->eptr)) {
        _self->eptr++;
    }

    first_char = *_self->eptr;

    // operator 
    switch (first_char) {
    case '+':   case '-':   case '*':   case '/': case '^':   case '%':
        buffer[0] = first_char;
        buffer[1] = '\0';
        _self->eptr++;
        return set_token(&_self->token, OPR, buffer);
    case '(':   case ')':
        buffer[0] = first_char;
        buffer[1] = '\0';
        _self->eptr++;
        return set_token(&_self->token, BKT, buffer);
    }

    // expr end here 
    if (first_char == '\0') {
        return set_token(&_self->token, END, "");
    }
    // numberic
    else if (is_digit(first_char)) {
        while (is_digit(*_self->eptr)) {
            *pbuffer++ = *_self->eptr++;
        }
        if (*_self->eptr != '.') {
            *pbuffer = '\0';
            return set_token(&_self->token, NUM, buffer);
        }
        while (is_digit(*_self->eptr)) {
            *pbuffer++ = *_self->eptr++;
        }
        *pbuffer = '\0';
        return set_token(&_self->token, NUM, buffer);
    }
    // symbol
    else if (is_alpha(first_char)) {
        while (is_alnum(*_self->eptr)) {
            *pbuffer++ = *_self->eptr++;
        }
        *pbuffer = '\0';
        return set_token(&_self->token, SYM, buffer);
    }
    // undefined
    else {
        _self->eptr++;
        buffer[0] = first_char;
        buffer[1] = '\0';
        return set_token(&_self->token, UDF, buffer);
    }
    return NULL;
}

void rewind_token(analyzer_t *_self) {
    _self->eptr -= strlen(_self->token.content);
}

void reset_token(analyzer_t *_self) {
    _self->eptr = _self->expr;
}

int match_expr(analyzer_t *_self) {
    token_t *token = get_token(_self);
    // printf("[%s] %s\n", DBG_TOKEN_NAME[token->type], token->content);
    if (token->type == SYM || token->type == NUM) {
        token_t *next = get_token(_self);
        if (next->type == OPR) {
            return match_expr(_self);
        }
        else {
            rewind_token(_self);
        }
        return 1;
    }
    else if (token->type == OPR && STREQ(token->content, "-")) {
        return match_expr(_self);
    }
    else if (token->type == BKT && STREQ(token->content, "(")) {
        int success = match_expr(_self);
        if (!success) return 0;
        token = get_token(_self);
        // printf("A[%s] %s\n", DBG_TOKEN_NAME[token->type], token->content);
        if (token->type == BKT && STREQ(token->content, ")")) {
            token_t *next = get_token(_self);
            if (next->type == OPR) {
                return match_expr(_self);
            }
            else {
                rewind_token(_self);
            }
            return 1;
        }
        return 0;
    }
    return 0;
}

void display_syntax_error(analyzer_t *_self) {
    int i;
    rewind_token(_self);
    puts("[Syntax error]");
    puts(_self->expr);
    for (i = 0; i < _self->eptr - _self->expr; ++i) {
        printf(" ");
    }
    printf("^\n");
}

int check_expr(analyzer_t *_self) {
    int result = match_expr(_self);
    // syntax error
    if (!result) {
        // display error info
        display_syntax_error(_self);
        return 0;
    }
    else {
        // checkout not end
        token_t *token = get_token(_self);
        if (token->type != END) {
            display_syntax_error(_self);
            return 0;
        }
    }
    reset_token(_self);
    return 1;
}

void travel(expr_node_t *node) {
    if (node == NULL)
        return;
    printf("[%s] %s\n", DBG_TOKEN_NAME[node->type], node->content);
    travel(node->lchild);
    travel(node->rchild);
}

expr_node_t * expr_node_by_token(const token_t * token) {
    expr_node_t *node = (expr_node_t *)malloc(sizeof(expr_node_t));

    node->type = token->type;
    strcpy(node->content, token->content);
    node->lchild = node->rchild = NULL;

    return node;
}

void destory_expr(expr_node_t *node) {
    if (node == NULL) return;
    destory_expr(node->lchild);
    destory_expr(node->rchild);
    free(node);
}

int operator_priority(const expr_node_t *node) {
    const char *opr = node->content;
    if (STREQ(opr, "+") || STREQ(opr, "-")) return 1;
    if (STREQ(opr, "*") || STREQ(opr, "/") || STREQ(opr, "^") || STREQ(opr, "%")) return 2;
    return 0;
}

expr_node_t* build_expr_tree(analyzer_t *_self) {
    token_t     *token;
    expr_node_t *node;
    vstack_t    *opr_stack = vs_new_stack(100);
    vstack_t    *sym_stack = vs_new_stack(100);

    while (1) {
        token = get_token(_self);
        if (*token->content != ')')
            node = expr_node_by_token(token);

        if (token->type == SYM || token->type == NUM) {
            vs_push(sym_stack, node);
        }
        else if (token->type == OPR) {
            while (!vs_empty(opr_stack)) {
                expr_node_t *top = (expr_node_t *)vs_top(opr_stack);
                // printf("PRI = %s(%d), %s(%d)\n", top->content, operator_priority(top), node->content, operator_priority(node));
                if (operator_priority(top) >= operator_priority(node)) {
                    vs_pop(opr_stack);
                    // printf("POP = %d,%s\n", top->type, top->content);
                    top->rchild = vs_pop(sym_stack);
                    top->lchild = vs_pop(sym_stack);
                    vs_push(sym_stack, top);
                }
                else {
                    break;
                }
            }
            vs_push(opr_stack, node);
        }
        else if (token->type == BKT) {
            if (*token->content == '(') {
                vs_push(opr_stack, node);
            }
            else if (*token->content == ')') {
                while (!vs_empty(opr_stack)) {
                    expr_node_t *top = (expr_node_t *)vs_pop(opr_stack);
                    if (top->type == BKT) break;
                    top->rchild = vs_pop(sym_stack);
                    top->lchild = vs_pop(sym_stack);
                    vs_push(sym_stack, top);
                }
            }
        }
        else if (token->type == END) {
            break;
        }
    }

    while (!vs_empty(opr_stack)) {
        expr_node_t *top = (expr_node_t *)vs_pop(opr_stack);
        top->rchild = vs_pop(sym_stack);
        top->lchild = vs_pop(sym_stack);
        vs_push(sym_stack, top);
    }

    // travel((expr_node_t *)vs_top(sym_stack));
    node = vs_top(sym_stack);

    vs_free(opr_stack);
    vs_free(sym_stack);

    return node;
}

#define RNTYP_NODE 1
#define RNTYP_TEXT 2

typedef struct tag_renderer_node_t {
    int type;
    char *content;
    vlist_t *children;
    int width;
    int height;
} renderer_node_t;

renderer_node_t * rn_new(int type, const char *content) {
    renderer_node_t * renderer = (renderer_node_t *)malloc(sizeof(renderer_node_t));
    renderer->type = type;
    renderer->content = content ? STR_DUMP(content) : NULL;
    renderer->children = type == RNTYP_NODE ? vl_new_list() : NULL;
    renderer->width = 0;
    renderer->height = 0;
    return renderer;
}

void rn_destory(renderer_node_t *_self) {
    if (_self->children)
        vl_destory(_self->children);
    if (_self->content)
        free(_self->content);
    free(_self);
}

void build_renderer(expr_node_t *node, expr_node_t *parent, renderer_node_t *renderer) {
    int need_bkt = 0;
    int parent_is_div = parent && parent->type == OPR && STREQ(parent->content, "/");

    if (node->type == SYM || node->type == NUM) {
        vl_push_back(renderer->children, rn_new(RNTYP_TEXT, node->content));
        return;
    }

    if (parent != NULL) {
        if (!parent_is_div && operator_priority(node) < operator_priority(parent)) {
            need_bkt = 1;
        }
    }

    if (node->type == OPR && (STREQ(node->content, "/") || STREQ(node->content, "^"))) {
        renderer_node_t * rn_self = rn_new(RNTYP_NODE, node->content);
        renderer_node_t * rn_left = rn_new(RNTYP_NODE, NULL);
        renderer_node_t * rn_right = rn_new(RNTYP_NODE, NULL);

        vl_push_back(rn_self->children, rn_left);
        vl_push_back(rn_self->children, rn_right);

        if (node->lchild) build_renderer(node->lchild, node, rn_left);

        if (node->rchild) build_renderer(node->rchild, node, rn_right);

        vl_push_back(renderer->children, rn_self);
    }
    else {
        if (need_bkt) vl_push_back(renderer->children, rn_new(RNTYP_TEXT, "("));

        if (node->lchild) build_renderer(node->lchild, node, renderer);

        vl_push_back(renderer->children, rn_new(RNTYP_TEXT, node->content));

        if (node->rchild) build_renderer(node->rchild, node, renderer);

        if (need_bkt) vl_push_back(renderer->children, rn_new(RNTYP_TEXT, ")"));
    }

}

void test_size(renderer_node_t * rn) {
    if (rn->type == RNTYP_TEXT) {
        rn->width = strlen(rn->content) * FONT_WIDTH_PX;
        rn->height = FONT_HEIGHT_PX;
    }
    else if (rn->type == RNTYP_NODE) {
        if (STREQ(rn->content, "/")) {
            renderer_node_t *num = (renderer_node_t *)rn->children->head->data;
            renderer_node_t *den = (renderer_node_t *)rn->children->tail->data;

            test_size(num);
            test_size(den);

            rn->width = max(num->width, den->width) + 2;
            rn->height = num->height + den->height + 1;
        }
        else if (STREQ(rn->content, "^")) {
            renderer_node_t *left = (renderer_node_t *)rn->children->head->data;
            renderer_node_t *right = (renderer_node_t *)rn->children->tail->data;

            test_size(left);
            test_size(right);

            rn->width = left->width + right->width;
            rn->height = left->height + right->height - FONT_HEIGHT_PX / 2;
        }
        else {
            int width = 0;
            int top = 0, bottom = FONT_HEIGHT_PX;

            vlist_node_t *node = rn->children->head;

            for (; node; node = node->next) {
                renderer_node_t *child = (renderer_node_t *)node->data;
                test_size(child);
                width += child->width;
                if (child->type == RNTYP_NODE) {
                    if (STREQ(child->content, "/")) {
                        renderer_node_t *num = (renderer_node_t *)child->children->head->data;
                        renderer_node_t *den = (renderer_node_t *)child->children->tail->data;
                        int test_top = FONT_HEIGHT_PX / 2 - 1 - num->height;
                        int test_bottom = FONT_HEIGHT_PX / 2 + 1 + den->height;
                        if (test_top < top) top = test_top;
                        if (test_bottom > bottom) bottom = test_bottom;
                    }
                    else if (STREQ(child->content, "^")) {
                        renderer_node_t *left = (renderer_node_t *)child->children->head->data;
                        renderer_node_t *right = (renderer_node_t *)child->children->tail->data;
                        int test_top = FONT_HEIGHT_PX / 2 - left->height / 2 - right->height / 2;
                        int test_bottom = FONT_HEIGHT_PX / 2 + right->height / 2;
                        if (test_top < top) top = test_top;
                        if (test_bottom > bottom) bottom = test_bottom;
                    }
                }
            }
            rn->width = width;
            rn->height = bottom - top;
        }
    }
}

void test_travel_renderer(renderer_node_t *rn) {
    vlist_node_t *ln = rn->children->head;

    printf("<node tag='%s'>", rn->content ? rn->content : "");

    for (; ln; ln = ln->next) {
        renderer_node_t *rn = (renderer_node_t *)ln->data;
        if (rn->type == RNTYP_TEXT) {
            printf("%s ", rn->content);
        }
        else if (rn->type == RNTYP_NODE) {
            test_travel_renderer(rn);
        }
    }

    printf("</node>");
}

void render(int *x, int *y, renderer_node_t *rn) {

    if (rn->type == RNTYP_TEXT) {
        int i;
        for (i = 0; rn->content[i]; ++i) {
            disp_char(*x, *y, rn->content[i]);
            *x += FONT_WIDTH_PX;
        }
    }
    else if (rn->type == RNTYP_NODE) {
        int ox = *x;
        int oy = *y;

        if (STREQ(rn->content, "/")) {
            renderer_node_t *num = (renderer_node_t *)rn->children->head->data;
            renderer_node_t *den = (renderer_node_t *)rn->children->tail->data;

            *x = ox + (rn->width - num->width) / 2;
            *y = oy - num->height + FONT_HEIGHT_PX / 2 - 1;
            render(x, y, num);

            disp_line(ox + 1, oy + FONT_HEIGHT_PX / 2 - 1, ox + rn->width - 1, oy + FONT_HEIGHT_PX / 2 - 1);

            *x = ox + (rn->width - den->width) / 2;
            *y = oy + FONT_HEIGHT_PX / 2 + 1;
            render(x, y, den);

            *x = ox + rn->width;
            *y = oy;
        }
        else if (STREQ(rn->content, "^")) {
            renderer_node_t *left = (renderer_node_t *)rn->children->head->data;
            renderer_node_t *right = (renderer_node_t *)rn->children->tail->data;

            *x = ox;
            *y = oy + FONT_HEIGHT_PX / 2 - left->height / 2;
            render(x, y, left);

            *x = ox + left->width;
            *y = oy + FONT_HEIGHT_PX / 2 - left->height / 2 - right->height / 2;
            render(x, y, right);

            *x = ox + rn->width;
            *y = oy;
        }
        else {
            int top = 0;
            //int bottom = FONT_HEIGHT_PX;

            vlist_node_t *node;

            for (node = rn->children->head; node; node = node->next) {
                renderer_node_t *child = (renderer_node_t *)node->data;

                if (child->type == RNTYP_NODE) {
                    if (STREQ(child->content, "/")) {
                        renderer_node_t *num = (renderer_node_t *)child->children->head->data;
                        int test_top = FONT_HEIGHT_PX / 2 - 1 - num->height;
                        if (test_top < top) top = test_top;
                    }
                    else if (STREQ(child->content, "^")) {
                        renderer_node_t *left = (renderer_node_t *)child->children->head->data;
                        renderer_node_t *right = (renderer_node_t *)child->children->tail->data;
                        int test_top = FONT_HEIGHT_PX / 2 - left->height / 2 - right->height / 2;
                        // int test_bottom = FONT_HEIGHT_PX / 2 + right->height / 2;
                        if (test_top < top) top = test_top;
                    }
                }
            }

            *y = oy - top;

            for (node = rn->children->head; node; node = node->next) {
                renderer_node_t *child = (renderer_node_t *)node->data;
                render(x, y, child);
            }

            *y = oy;
        }
    }
}

#if defined(APP_MSVC)
int app() {
#else
int main() {
#endif
    analyzer_t analyzer;
    expr_node_t *expr_root;
    renderer_node_t *renderer_root = rn_new(RNTYP_NODE, "root");

    analyzer.expr = "A+(1/2)^(3/4)+5*(1+(2+C/D)/3)";
    reset_token(&analyzer);

    printf("SYNTAX CHECK = %s\n", check_expr(&analyzer) ? "SUCCESS" : "FAIL");
    expr_root = build_expr_tree(&analyzer);
    travel(expr_root);
    build_renderer(expr_root, NULL, renderer_root);

    test_size(renderer_root);
    printf("size=%d,%d\n", renderer_root->width, renderer_root->height);
    test_travel_renderer(renderer_root);

    init_graph_app();

    all_clr();
    {
        int x = 9 * 6;
        int y = 16;
        disp_string(0, 0, "INPUT = ");
        disp_string(8 * 6, 0, analyzer.expr);
        disp_string(0, 8, "====================================");
        disp_string(0, 16 + (renderer_root->height - FONT_HEIGHT_PX) / 2, "OUTPUT = ");
        render(&x, &y, renderer_root);
    }
    

    put_disp();
    
    while (wait_key()) {
    }

    destory_expr(expr_root);

    return 0;
}

