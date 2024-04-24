#include <stdlib.h> /* strtod */
#include <assert.h> /* assert */
#include <errno.h> /* errno */
#include <math.h> /* HUGE_VAL */
#include "leptjson.h"


lept_type get_lept_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double get_lept_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}

// !!注意下面代码是错误的，野指针，即使经常写代码也容易犯这种错误
// 不要使用未初始化的指针
// int lept_parse(lept_value* v, const char* json) {
//     assert(v != NULL);
//     lept_content* c;
//     c->json = json;
//     v->type = LEPT_NULL; // 默认类型为NULL,解析失败时为此值
//
//     // 解析空白符号
//     lept_parse_whitespace(c);
//     return lept_parse_value(c, v);
// }

int lept_parse(lept_value* v, const char* json) {
    assert(v != NULL);
    lept_content c;
    c.json = json;
    v->type = LEPT_NULL; // 默认类型为NULL,解析失败时为此值

    // 解析空白符号
    lept_parse_whitespace(&c);
    int ret;
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

static int lept_parse_value(lept_content* c, lept_value* v) {
    switch(*c->json) {
        case 'n': return lept_parse_literal(c, v, "null", LEPT_NULL);
        case 't': return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f': return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
        default: return lept_parse_number(c, v);
    }
}

static int lept_parse_literal(lept_content* c, lept_value* v, const char* literal, lept_type type) {
    EXPECT(c, literal[0]);
    int index = 1;
    while (literal[index] != '\0') {
        if (c->json[index] != literal[index])
            return LEPT_PARSE_INVALID_VALUE;
        index++;
    }
    c->json += index;
    v->type = type;
    return LEPT_PARSE_OK;
}

static double lept_parse_number(lept_content* c, lept_value* v) {
    const char* p = c->json;
    // check format
    // 负号
    if (*p == '-') 
        p++;
    // 数字
    if (*p == '0') p++;
    else {
        if (!ISDIGIT0TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    // 小数
    if (*p == '.') {
        p++;
        // 小数后面一个以上数字
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    // 指数
    if (*p == 'e' || *p == 'E') {
        p++;
        // 指数可能有正负
        if (*p == '-' || *p == '+') p++;
        // 后面是一个以上数字
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }

    // 处理越界
    errno = 0;
    v->n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    c->json = p;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static void lept_parse_whitespace(lept_content* c) {
    const char* p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}


