#include "platform.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
// #include "eigenmath/defs.h"

// #define CONSOLE_ONLY

#if !defined(CONSOLE_ONLY)
#   include "graphProvider.h"
#   include "inputProvider.h"
#   include "fontProvider.h"
#   ifdef APP_FXCG
#       define puts // 
#       define putchar // 
#       define printf // 
#       define max(a, b) ((a) > (b) ? (a) : (b))
#   endif
#endif

#define STR_EQUAL(s1, s2) (((s1) != NULL) && ((s2) != NULL) && strcmp((s1), (s2)) == 0)
#define DEBUG_MSG(s) // puts(s)

#define is_digit(c) ((c) >= '0' && (c) <= '9')
#define is_alpha(c) ((c) >= 'a' && (c) <= 'z' || (c) >= 'A' && (c) <= 'Z')
#define is_alnum(c) ((is_digit(c)) || (is_alpha(c)))
#define is_space(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == 'r')

#define TOKEN_LENGTH_MAX 20

typedef enum {
    TOKEN_ERR, TOKEN_END, TOKEN_NUM,
    TOKEN_SYM, TOKEN_OPR, TOKEN_FNC,
    TOKEN_BKT, TOKEN_CMA, TOKEN_UDF
} token_type_t;

const char *DBG_TOKEN_NAME[] = {
    "TOKEN_ERR", "TOKEN_END", "TOKEN_NUM", "TOKEN_SYM", "TOKEN_OPR", "TOKEN_FNC", "TOKEN_BKT", "TOKEN_CMA", "TOKEN_UDF"
};

const struct {
    const char *opr;
    int priority;
}
OPR_PRIORITY[] = {
    { "+",  1 }, { "-",  1 },
    { "*", 10 }, { "/", 10 },
    { "^", 20 }, { "%", 20 },
    { NULL, 0 }
};

typedef struct tag_token_t {
    token_type_t type;
    char content[TOKEN_LENGTH_MAX];
} token_t;

typedef struct tag_expr_node_t {
    token_type_t type;
    char content[TOKEN_LENGTH_MAX];
    struct tag_expr_node_t **children;
    int child_num;
} expr_node_t;

#define EN_LCHILD(en) ((en)->children[0])
#define EN_RCHILD(en) ((en)->children[1])

char *STR_COPY(char *dest, int max, const char *src) {
    int i;
    for (i = 0; i < max - 1 && src[i]; ++i) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    return dest;
}

char *STR_DUMP(const char *str) {
    int length = strlen(str) + 1;
    char *buffer = (char *)malloc(length);
    STR_COPY(buffer, length + 1, str);
    return buffer;
}

token_t* set_token(token_t *token, token_type_t type, const char *content) {
    token->type = type;
    STR_COPY(token->content, TOKEN_LENGTH_MAX, content);
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

token_t* next_token(analyzer_t *_self) {
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
        return set_token(&_self->token, TOKEN_OPR, buffer);
    case '(':   case ')':
        buffer[0] = first_char;
        buffer[1] = '\0';
        _self->eptr++;
        return set_token(&_self->token, TOKEN_BKT, buffer);
    case ',':
        buffer[0] = first_char;
        buffer[1] = '\0';
        _self->eptr++;
        return set_token(&_self->token, TOKEN_CMA, buffer);
    }

    // expr end here 
    if (first_char == '\0') {
        return set_token(&_self->token, TOKEN_END, "");
    }
    // numberic
    else if (is_digit(first_char)) {
        while (is_digit(*_self->eptr)) {
            *pbuffer++ = *_self->eptr++;
        }
        if (*_self->eptr != '.') {
            *pbuffer = '\0';
            return set_token(&_self->token, TOKEN_NUM, buffer);
        }
        while (is_digit(*_self->eptr)) {
            *pbuffer++ = *_self->eptr++;
        }
        *pbuffer = '\0';
        return set_token(&_self->token, TOKEN_NUM, buffer);
    }
    // symbol
    else if (is_alpha(first_char)) {
        int is_func = 0;

        while (is_alnum(*_self->eptr)) {
            *pbuffer++ = *_self->eptr++;
        }
        // func
        if (*_self->eptr == '(') {
            _self->eptr++;
            is_func = 1;
        }
        *pbuffer = '\0';

        if (is_func) {
            return set_token(&_self->token, TOKEN_FNC, buffer);
        }
        else {
            return set_token(&_self->token, TOKEN_SYM, buffer);
        }
    }
    // undefined
    else {
        _self->eptr++;
        buffer[0] = first_char;
        buffer[1] = '\0';
        return set_token(&_self->token, TOKEN_UDF, buffer);
    }
    return NULL;
}

void rewind_token(analyzer_t *_self) {
    _self->eptr -= strlen(_self->token.content);
}

void reset_token(analyzer_t *_self) {
    _self->eptr = _self->expr;
}

int match_expr(analyzer_t *_self);

int match_try_next(analyzer_t *_self) {
    token_t *next = next_token(_self);
    if (next->type == TOKEN_OPR) {
        return match_expr(_self);
    }
    else {
        rewind_token(_self);
    }
    return 1;
}

int match_expr(analyzer_t *_self) {
    token_t *token = next_token(_self);
    // printf("[%s] %s\n", DBG_TOKEN_NAME[token->type], token->content);
    if (token->type == TOKEN_SYM || token->type == TOKEN_NUM) {
        return match_try_next(_self);
    }
    else if (token->type == TOKEN_OPR && STR_EQUAL(token->content, "-")) {
        return match_expr(_self) && match_try_next(_self);
    }
    else if (token->type == TOKEN_FNC) {
        while (1) {
            int result = match_expr(_self);
            if (!result)
                return 0;
            token = next_token(_self);
            if (token->type == TOKEN_CMA)
                continue;
            else if (token->type == TOKEN_BKT && *token->content == ')')
                break;
            else
                return 0;
        }
        return match_try_next(_self);
    }
    else if (token->type == TOKEN_BKT && STR_EQUAL(token->content, "(")) {
        int success = match_expr(_self);
        if (!success) return 0;
        token = next_token(_self);
        // printf("A[%s] %s\n", DBG_TOKEN_NAME[token->type], token->content);
        if (token->type == TOKEN_BKT && STR_EQUAL(token->content, ")")) {
            token_t *next = next_token(_self);
            if (next->type == TOKEN_OPR) {
                return match_expr(_self);
            }
            else {
                rewind_token(_self);
            }
            return 1;
        }
        return 0; //return match_try_next(_self);
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
        token_t *token = next_token(_self);
        if (token->type != TOKEN_END) {
            display_syntax_error(_self);
            return 0;
        }
    }
    reset_token(_self);
    return 1;
}

expr_node_t * en_new(const int type, const char *content, int child_num) {
    int i;

    expr_node_t *node = (expr_node_t *)malloc(sizeof(expr_node_t));

    node->type = type;
    STR_COPY(node->content, TOKEN_LENGTH_MAX, content);

    node->child_num = child_num;
    node->children = (expr_node_t **)malloc(child_num * (sizeof(expr_node_t *)));
    for (i = 0; i < node->child_num; ++i) {
        node->children[i] = NULL;
    }

    return node;
}


expr_node_t * en_create_by_token(const token_t * token, int child_num) {
    return en_new(token->type, token->content, child_num);
}

void en_destory(expr_node_t *node) {
    int i;

    if (node == NULL) return;

    for (i = 0; i < node->child_num; ++i) {
        if (node->children[i]) free(node->children[i]);
    }
    free(node->children);

    free(node);
}

expr_node_t* build_expr_tree(analyzer_t *_self);

expr_node_t* build_try_next(analyzer_t *_self, expr_node_t *left_node) {
    token_t *token = next_token(_self);

    if (token->type == TOKEN_OPR) {
        expr_node_t *opr_node = en_create_by_token(token, 2);
        expr_node_t *right_node = build_expr_tree(_self);

        EN_LCHILD(opr_node) = left_node;
        EN_RCHILD(opr_node) = right_node;

        return opr_node;
    }
    else {
        rewind_token(_self);
        return left_node;
    }

    return NULL;
}

expr_node_t* build_expr_tree(analyzer_t *_self) {
    token_t *token;

    token = next_token(_self);
    // printf("token = %s,%s\n", DBG_TOKEN_NAME[token->type], token->content);

    if (token->type == TOKEN_SYM || token->type == TOKEN_NUM) {
        expr_node_t *left_node = en_create_by_token(token, 0);
        return build_try_next(_self, left_node);
    }
    else if (token->type == TOKEN_OPR) { // '-'
        expr_node_t* opr_node = en_create_by_token(token, 1);
        EN_LCHILD(opr_node) = build_expr_tree(_self);
        return build_try_next(_self, opr_node);
    }
    else if (token->type == TOKEN_FNC) {
        char *func_name = STR_DUMP(token->content);
        expr_node_t *func_node = NULL;
        expr_node_t *child_buf[100];
        int child_num = 0, i;

        while (1) {
            expr_node_t *child = build_expr_tree(_self);

            child_buf[child_num++] = child;

            token = next_token(_self);
            if (token->type == TOKEN_CMA) {
                continue;
            }
            else if (token->type == TOKEN_BKT && *token->content == ')') {
                break;
            }
        }

        func_node = en_new(TOKEN_FNC, func_name, child_num);
        for (i = 0; i < child_num; ++i) {
            func_node->children[i] = child_buf[i];
        }
        free(func_name);

        return build_try_next(_self, func_node);
    }
    else if (token->type == TOKEN_BKT && *token->content == '(') {
        expr_node_t *bkt_node = en_new(TOKEN_BKT, "()", 1);
        bkt_node->children[0] = build_expr_tree(_self);
        next_token(_self); // ignore ')'

        return build_try_next(_self, bkt_node);
    }

    return NULL;
}

expr_node_t* reduce_bkt(expr_node_t * en) {
    expr_node_t* content;

    if (en == NULL || en->type != TOKEN_BKT) return en;

    content = en->children[0];
    en->children[0] = NULL;

    en_destory(en);

    return content;
}

void reduce_all_bkt(expr_node_t * en) {
    int i;
    if (en == NULL || en->type == TOKEN_SYM || en->type == TOKEN_NUM) {
        return;
    }
    if (STR_EQUAL("/", en->content)) {
        EN_LCHILD(en) = reduce_bkt(EN_LCHILD(en));
        EN_RCHILD(en) = reduce_bkt(EN_RCHILD(en));
    }
    else if (STR_EQUAL("^", en->content)) {
        EN_RCHILD(en) = reduce_bkt(EN_RCHILD(en));
    }
    for (i = 0; i < en->child_num; ++i) {
        reduce_all_bkt(en->children[i]);
    }
}

void reduce_sqrt(expr_node_t * en) {
    int i;
    if (en == NULL || en->type == TOKEN_SYM || en->type == TOKEN_NUM) {
        return;
    }
    for (i = 0; i < en->child_num; ++i) {
        reduce_sqrt(en->children[i]);
    }
    if (STR_EQUAL("^", en->content)) {
        expr_node_t * power = EN_RCHILD(en);
        if (power->type == TOKEN_OPR && STR_EQUAL("/", power->content)) {
            if (EN_LCHILD(power)->type == TOKEN_NUM && STR_EQUAL("1", EN_LCHILD(power)->content)
             && EN_RCHILD(power)->type == TOKEN_NUM && STR_EQUAL("2", EN_RCHILD(power)->content)) {
                expr_node_t *base = EN_LCHILD(en);
                en_destory(power);
                free(en->children);
                en->child_num = 1;
                en->children = (expr_node_t **)malloc(1 * sizeof(expr_node_t *));
                en->children[0] = base;
                en->type = TOKEN_FNC;
                STR_COPY(en->content, TOKEN_LENGTH_MAX, "sqrt");
            }
        }
    }
}

int opr_priority(const expr_node_t *node) {
    int i;
    for (i = 0; OPR_PRIORITY[i].opr; ++i) {
        if (STR_EQUAL(OPR_PRIORITY[i].opr, node->content)) {
            return OPR_PRIORITY[i].priority;
        }
    }
    return -1;
}

expr_node_t* sort_expr_tree(expr_node_t *en, int *ptr_change_flag) {
    if (en == NULL) {
        return NULL;
    }
    else if (en->type == TOKEN_FNC || en->type == TOKEN_BKT) {
        int i;
        for (i = 0; i < en->child_num; ++i) {
            en->children[i] = sort_expr_tree(en->children[i], ptr_change_flag);
        }
        return en;
    }
    else if (en->type == TOKEN_OPR) {
        expr_node_t *enr, *a, *b;
        /*
        上一步中 1/2+3 会被解析成如下的表达式树
            (/) en
            / \
           /   \
         (1)  (+) enr
         c    / \
             /   \
           (2)   (3)
            a     b

        '/'的优先级比'+'高，所以进行排序
            (+) enr
            / \
           /   \
      en (/)   (3) b
         / \
        /   \
      (1)   (2) a
        */
        EN_LCHILD(en) = sort_expr_tree(EN_LCHILD(en), ptr_change_flag);
        EN_RCHILD(en) = sort_expr_tree(EN_RCHILD(en), ptr_change_flag);
        enr = EN_RCHILD(en);

        if (enr->type == TOKEN_OPR && opr_priority(en) >= opr_priority(enr)) {
            *ptr_change_flag = 1;
            
            a = EN_LCHILD(enr);
            b = EN_RCHILD(enr);
            EN_RCHILD(en) = a;
            EN_LCHILD(enr) = en;

            EN_RCHILD(enr) = sort_expr_tree(EN_RCHILD(enr), ptr_change_flag);

            return enr;
        }
    }
    return en;
}

void sort_expr(expr_node_t ** ptr_en) {
    int change_flag;

    do {
        change_flag = 0;
        *ptr_en = sort_expr_tree(*ptr_en, &change_flag);
    } while (change_flag);
}

#define RNTYP_NODE 1
#define RNTYP_TEXT 2
#define RNTYP_FUNC 3

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
    renderer->children = type == RNTYP_NODE || type == RNTYP_FUNC ? vl_new_list() : NULL;
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

void build_renderer(expr_node_t *en, renderer_node_t *renderer) {
    if (en->type == TOKEN_SYM || en->type == TOKEN_NUM) {
        vl_push_back(renderer->children, rn_new(RNTYP_TEXT, en->content));
    }
    else if (en->type == TOKEN_OPR) {
        if (STR_EQUAL(en->content, "/") || STR_EQUAL(en->content, "^")) {
            renderer_node_t *l = rn_new(RNTYP_NODE, "up");
            renderer_node_t *r = rn_new(RNTYP_NODE, "down");
            renderer_node_t *opr = rn_new(RNTYP_NODE, en->content);

            build_renderer(EN_LCHILD(en), l);
            build_renderer(EN_RCHILD(en), r);

            vl_push_back(opr->children, l);
            vl_push_back(opr->children, r);
            vl_push_back(renderer->children, opr);
        }
        else {
            build_renderer(EN_LCHILD(en), renderer);
            vl_push_back(renderer->children, rn_new(RNTYP_TEXT, en->content));
            build_renderer(EN_RCHILD(en), renderer);
        }
    }
    else if (en->type == TOKEN_BKT) {
        renderer_node_t *bkt = rn_new(RNTYP_NODE, "(bkt)");
        renderer_node_t *inner = rn_new(RNTYP_NODE, "inner");
        int i;
        for (i = 0; i < en->child_num; ++i) {
            build_renderer(en->children[i], inner);
        }
        vl_push_back(bkt->children, inner);
        vl_push_back(renderer->children, bkt);
    }
    else if (en->type == TOKEN_FNC) {
        renderer_node_t *func = rn_new(RNTYP_FUNC, en->content);
        renderer_node_t *inner = rn_new(RNTYP_NODE, "inner");
        int i;
        for (i = 0; i < en->child_num; ++i) {
            build_renderer(en->children[i], inner);
            if (i < en->child_num - 1)
                vl_push_back(inner->children, rn_new(RNTYP_TEXT, ","));
        }
        vl_push_back(func->children, inner);
        vl_push_back(renderer->children, func);
    }
}

void travel_expr(expr_node_t *en, int tab) {
    int i;
    for (i = 0; i < tab; ++i) printf(" ");
    if (en->child_num > 0) {
        printf("<en text=\"%s\">\n", en->content);

        for (i = 0; i < en->child_num; ++i) {
            travel_expr(en->children[i], tab + 2);
        }

        for (i = 0; i < tab; ++i) printf(" "); printf("</en>\n");
    }
    else {
        printf("%s\n", en->content);
    }
}

void travel_renderer(renderer_node_t *rn, int tab) {
    int i;
    for (i = 0; i < tab; ++i) printf(" ");
    if (rn->type != RNTYP_TEXT) {
        vlist_node_t * node;
        printf("<rn text=\"%s\" type=\"%d\">\n", rn->content ? rn->content : "NULL", rn->type);

        for (node = rn->children->head; node; node = node->next) {
            travel_renderer((renderer_node_t *)node->data, tab + 2);
        }

        for (i = 0; i < tab; ++i) printf(" "); printf("</rn>\n");
    }
    else {
        printf("%s\n", rn->content);
    }
}

#if !defined(CONSOLE_ONLY)

void test_size(renderer_node_t * rn) {
    if (rn->type == RNTYP_TEXT) {
        rn->width = strlen(rn->content) * FONT_WIDTH_PX;
        rn->height = FONT_HEIGHT_PX;
    }
    else if (rn->type == RNTYP_FUNC) {
        if (STR_EQUAL("sqrt", rn->content)) {
            renderer_node_t *content = (renderer_node_t *)rn->children->head->data;

            test_size(content);

            rn->width = content->width + 2 + 3;
            rn->height = content->height + 2;
        }
        else {
            renderer_node_t *content = (renderer_node_t *)rn->children->head->data;

            test_size(content);

            rn->width = content->width + 3 + 3 + 1 + strlen(rn->content) * FONT_WIDTH_PX;
            rn->height = content->height;
        }
    }
    else if (rn->type == RNTYP_NODE) {
        if (STR_EQUAL(rn->content, "/")) {
            renderer_node_t *num = (renderer_node_t *)rn->children->head->data;
            renderer_node_t *den = (renderer_node_t *)rn->children->tail->data;

            test_size(num);
            test_size(den);

            rn->width = max(num->width, den->width) + 2;
            rn->height = num->height + den->height + 2;
        }
        else if (STR_EQUAL(rn->content, "^")) {
            renderer_node_t *left = (renderer_node_t *)rn->children->head->data;
            renderer_node_t *right = (renderer_node_t *)rn->children->tail->data;

            test_size(left);
            test_size(right);

            rn->width = left->width + right->width;
            rn->height = left->height + right->height / 2;
        }
        else if (STR_EQUAL(rn->content, "(bkt)")) {
            renderer_node_t *content = (renderer_node_t *)rn->children->head->data;
            
            test_size(content);

            rn->width = content->width + 3 + 3;
            rn->height = content->height;

            printf("test inner %d, %d\n", rn->width, rn->height);
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
                    if (STR_EQUAL(child->content, "/")) {
                        renderer_node_t *num = (renderer_node_t *)child->children->head->data;
                        renderer_node_t *den = (renderer_node_t *)child->children->tail->data;
                        int test_top = FONT_HEIGHT_PX / 2 - 1 - num->height;
                        int test_bottom = FONT_HEIGHT_PX / 2 + 1 + den->height;
                        if (test_top < top) top = test_top;
                        if (test_bottom > bottom) bottom = test_bottom;
                    }
                    else if (STR_EQUAL(child->content, "^")) {
                        renderer_node_t *left = (renderer_node_t *)child->children->head->data;
                        renderer_node_t *right = (renderer_node_t *)child->children->tail->data;
                        int test_top = FONT_HEIGHT_PX / 2 - left->height / 2 - right->height / 2;
                        int test_bottom = FONT_HEIGHT_PX / 2 + left->height / 2;
                        if (test_top < top) top = test_top;
                        if (test_bottom > bottom) bottom = test_bottom;
                    }
                    else if (STR_EQUAL(child->content, "(bkt)")) {
                        renderer_node_t *one = (renderer_node_t *)child->children->head->data;
                        int test_top = FONT_HEIGHT_PX / 2 - one->height / 2;
                        int test_bottom = FONT_HEIGHT_PX / 2 + one->height / 2;
                        if (test_top < top) top = test_top;
                        if (test_bottom > bottom) bottom = test_bottom;
                        printf("test %s %d, %d\n", child->content, test_top, test_bottom);
                    }
                }
                else if (child->type == RNTYP_FUNC) {
                    if (STR_EQUAL("sqrt", child->content)) {
                        int test_top = FONT_HEIGHT_PX / 2 - child->height / 2;
                        int test_bottom = FONT_HEIGHT_PX / 2 + child->height / 2;
                        if (test_top < top) top = test_top;
                        if (test_bottom > bottom) bottom = test_bottom;
                    }
                    else {
                        renderer_node_t *one = (renderer_node_t *)child->children->head->data;
                        int test_top = FONT_HEIGHT_PX / 2 - one->height / 2;
                        int test_bottom = FONT_HEIGHT_PX / 2 + one->height / 2;
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

void render(int *x, int *y, renderer_node_t *rn) {

    if (rn->type == RNTYP_TEXT) {
        int i;
        for (i = 0; rn->content[i]; ++i) {
            disp_char(*x, *y, rn->content[i]);
            *x += FONT_WIDTH_PX;
        }
    }
    else if (rn->type == RNTYP_FUNC) {
        int ox = *x;
        int oy = *y;
        if (STR_EQUAL("sqrt", rn->content)) {
            int fy;
            renderer_node_t *one = (renderer_node_t *)rn->children->head->data;

            *x += 5;
            *y = oy + FONT_HEIGHT_PX / 2 - one->height / 2 + 1;
            fy = *y - 2;
            render(x, y, one);

            disp_line(ox, fy + rn->height - 5, ox + 2, fy + rn->height - 3);
            disp_line(ox + 2, fy + rn->height - 3, ox + 4, fy);
            disp_line(ox + 4, fy, ox + rn->width, fy);

            *x = ox + rn->width;
            *y = oy;
        }
        else {
            int i;

            renderer_node_t *one = (renderer_node_t *)rn->children->head->data;

            for (i = 0; rn->content[i]; ++i) {
                disp_char(*x, *y, rn->content[i]);
                *x += FONT_WIDTH_PX;
            }

            *x += 1;
            *y = oy + FONT_HEIGHT_PX / 2 - one->height / 2;
            disp_bkt(*x, *y, one->height, 1);

            *x += 3;
            *y = oy + FONT_HEIGHT_PX / 2 - one->height / 2;
            render(x, y, one);

            *x = ox + rn->width;
            *y = oy + FONT_HEIGHT_PX / 2 - one->height / 2;

            disp_bkt(*x, *y, one->height, 0);

            *y = oy;
        }
    }
    else if (rn->type == RNTYP_NODE) {
        int ox = *x;
        int oy = *y;

        if (STR_EQUAL(rn->content, "/")) {
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
        else if (STR_EQUAL(rn->content, "^")) {
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
        else if (STR_EQUAL(rn->content, "(bkt)")) {
            renderer_node_t *one = (renderer_node_t *)rn->children->head->data;

            *x = ox + 3;
            *y = oy + FONT_HEIGHT_PX / 2 - one->height / 2;
            render(x, y, one);

            *x = ox;
            *y = oy + FONT_HEIGHT_PX / 2 - one->height / 2;

            disp_bkt(*x, *y, one->height, 1);
            disp_bkt(*x + rn->width, *y, one->height, 0);

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
                    if (STR_EQUAL(child->content, "/")) {
                        renderer_node_t *num = (renderer_node_t *)child->children->head->data;
                        int test_top = FONT_HEIGHT_PX / 2 - 1 - num->height;
                        if (test_top < top) top = test_top;
                    }
                    else if (STR_EQUAL(child->content, "^")) {
                        renderer_node_t *left = (renderer_node_t *)child->children->head->data;
                        renderer_node_t *right = (renderer_node_t *)child->children->tail->data;
                        int test_top = FONT_HEIGHT_PX / 2 - left->height / 2 - right->height / 2;
                        // int test_bottom = FONT_HEIGHT_PX / 2 + right->height / 2;
                        if (test_top < top) top = test_top;
                    }
                    else if (STR_EQUAL(child->content, "(bkt)")) {
                        renderer_node_t *one = (renderer_node_t *)child->children->head->data;
                        int test_top = FONT_HEIGHT_PX / 2 - one->height / 2;

                        if (test_top < top) top = test_top;
                    }
                }
                else if (child->type == RNTYP_FUNC) {
                    if (STR_EQUAL("sqrt", child->content)) {
                        int test_top = FONT_HEIGHT_PX / 2 - child->height / 2;

                        if (test_top < top) top = test_top;
                    }
                    else {
                        renderer_node_t *one = (renderer_node_t *)child->children->head->data;
                        int test_top = FONT_HEIGHT_PX / 2 - one->height / 2;

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
#endif

char key2char(int key) {
    if (key >= KEY_CHAR_0 && key <= KEY_CHAR_9) {
        return '0' + (key - KEY_CHAR_0);
    }
    if (key >= KEY_CHAR_A && key <= KEY_CHAR_Z) {
        return 'a' + (key - KEY_CHAR_A);
    }
    switch (key) {
        case KEY_CHAR_PLUS:     return '+';
        case KEY_CHAR_MINUS:    return '-';
        case KEY_CHAR_MULT:     return '*';
        case KEY_CHAR_DIV:      return '/';
        case KEY_CHAR_LPAR:     return '(';
        case KEY_CHAR_RPAR:     return ')';
        case KEY_CHAR_COMMA:    return ',';
        case KEY_CHAR_DP:       return '.';
        case KEY_CHAR_POW:      return '^';
        case KEY_CHAR_EQUAL:    return '=';
    }
    return 0;
}

void get_string(const int x, const int y, const int char_width, char *buffer, const int max) {
    int length = strlen(buffer);
    
    while (1) {
        unsigned int key;
        // redraw
        {
            int left = 0, i;
            int tx, ty;
            
            for (tx = x; tx < x + char_width * FONT_WIDTH_PX; ++tx) {
                for (ty = y; ty < y + FONT_HEIGHT_PX; ++ty) {
                    set_pixel(tx, ty, 0xffff);
                }
            }

            if (length > char_width - 1) {
                left = length - char_width + 1;
            }

            for (i = left; i < length; ++i) {
                disp_char(x + FONT_WIDTH_PX * (i - left), y, buffer[i]);
            }
            disp_char(x + FONT_WIDTH_PX * (i - left), y, '_');
        }
#ifdef APP_MSVC
        put_disp();
#endif
        key = wait_key();
        if (key == KEY_CTRL_EXE) {
            buffer[length] = '\0';
            return;
        }
        else if (key == KEY_CTRL_DEL) {
            if (length >= 0) length--;
        }
#ifdef APP_MSVC
        else if (key == 0) {
            exit(0);
        }
#endif
        else {
            char c = key2char(key);
            if (c) {
                buffer[length++] = c;
            }
        }
    }
}

void draw_expr(int x, int y, int *p_width, int *p_height, const char *str_expr) {
    analyzer_t analyzer;
    expr_node_t *expr_root;
    renderer_node_t *renderer_root = rn_new(RNTYP_NODE, "root");
    renderer_node_t *rn = renderer_root;

    analyzer.expr = str_expr;
    reset_token(&analyzer);

    expr_root = build_expr_tree(&analyzer);
    travel_expr(expr_root, 0);
    sort_expr(&expr_root);
    reduce_all_bkt(expr_root);
    reduce_sqrt(expr_root);
    travel_expr(expr_root, 0);
    build_renderer(expr_root, renderer_root);
    travel_renderer(renderer_root, 0);
    test_size(renderer_root);
    printf("size=%d,%d\n", renderer_root->width, renderer_root->height);

    disp_set_color(RGB_24_TO_565(55, 55, 200));
    disp_line(x - 1, y - 1, x + rn->width + 1, y - 1);
    disp_line(x - 1, y + 1 + rn->height, x + rn->width + 1, y + 1 + rn->height);
    disp_line(x - 1, y - 1, x - 1, y + 1 + rn->height);
    disp_line(x + 1 + rn->width, y - 1, x + 1 + rn->width, y + 1 + rn->height);
    disp_set_color(RGB_24_TO_565(0, 0, 0));

    render(&x, &y, renderer_root);

    put_disp();

    if (p_width != NULL) *p_width = rn->width;
    if (p_height != NULL) *p_height = rn->height;

    rn_destory(renderer_root);
    en_destory(expr_root);
}

#if defined(APP_MSVC)
int app() {
#else
int main(int argc, char **argv) {
#endif
    static const char title[] = "=== Formula-Z Renderer ===";
    char buffer[200] = "expand((sqrt(a)+b)^2)+sqrt(x)/sqrt(y^5+1)/x+1";
    char result_buffer[] = "1 + a + b^2 + 2*a^(1/2)*b + 1 / (x^(1/2)*(y^5 + 1)^(1/2))";
    init_graph_app();

    all_clr();

    while (1) {
        int left = 0;
        int top = 14;

        disp_string(left, top, title);
        disp_string(left, top + 8, "> ");
        get_string(left + 6 * 3, top + 8, sizeof(title) - 1, buffer, sizeof(buffer));
        
        all_clr();
        /*
        while (1) {
            int key = wait_key();
            printf("%c = %d\n", key, key);
            if (key == 0)
                break;
        }
        */
        //run(buffer);
        //puts(result_buffer);
        // if (!check_expr(&analyzer)) {
        if (*result_buffer == '\n') {
            disp_set_color(RGB_24_TO_565(255,0 ,0));
            disp_string(left, top + 8 + 8, "Syntax Error!");
            disp_set_color(RGB_24_TO_565(0, 0, 0));
        }
        else {
            int width, height;
            int top2 = top + 8 + 8 + 4;

            draw_expr(left + 6 * 6, top2, &width, &height, buffer);

            disp_string(left, top2 + (height - 8) / 2, "In  = ");

            top2 += height + 8;

            draw_expr(left + 6 * 6, top2, &width, &height, result_buffer);

            disp_string(left, top2 + (height - 8) / 2, "Out = ");
        }
    }

    // while (1) {
    //     int key = wait_key();
    //     printf("%c = %d\n", key, key);
    //     if (key == 0)
    //         break;
    // }

    return 0;
}

