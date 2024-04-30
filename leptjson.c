#include <stdlib.h> /* strtod,malloc */
#include <assert.h> /* assert */
#include <errno.h> /* errno */
#include <math.h> /* HUGE_VAL */
#include <string.h> /* memcpy */
#include "leptjson.h"


int lept_parse(lept_value* v, const char* json) {
    assert(v != NULL);
    int ret;
    lept_content c;
    c.json = json;
    c.stack = NULL;
    c.size = c.top = 0;
    lept_init(v); // 默认类型为NULL,解析失败时为此值

    // 解析空白符号
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            lept_free(v);
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    // 确认清空缓冲区
    assert(c.top == 0);
    free(c.stack);
    return ret;
}

void lept_free(lept_value* v) {
    size_t i;
    switch (lept_get_type(v)) {
        case LEPT_STRING:
            free(v->s);
            break;
        case LEPT_ARRAY: {
            for (i = 0; i < v->array_size; i++) {
                lept_free(&v->array[i]);
            }
            free(v->array);
            break;
        }
    }
    v->type = LEPT_NULL;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

int lept_get_boolean(const lept_value* v) {
    lept_type type = lept_get_type(v);
    assert(type == LEPT_TRUE || type == LEPT_FALSE);
    return type == LEPT_TRUE;
}

void lept_set_boolean(lept_value* v, int b) {
    lept_free(v);
    v->type = b ? LEPT_TRUE : LEPT_FALSE;
}

double lept_get_number(const lept_value* v) {
    assert(lept_get_type(v) == LEPT_NUMBER);
    return v->n;
}

void lept_set_number(lept_value* v, double d) {
    lept_free(v);
    v->n = d;
    v->type = LEPT_NUMBER;
}

const char* lept_get_string(const lept_value* v) {
    assert(lept_get_type(v) == LEPT_STRING);
    return v->s;
}

size_t lept_get_string_length(const lept_value* v) {
    assert(lept_get_type(v) == LEPT_STRING);
    return v->len;
}

void lept_set_string(lept_value* v, const char* c, size_t len) {
    assert(v != NULL && (c != NULL || len == 0));
    lept_free(v);
    v->s = (char*)malloc(len + 1);
    memcpy(v->s, c, len);
    v->s[len] = '\0';
    v->len = len;
    v->type = LEPT_STRING;
}

size_t lept_get_array_size(const lept_value* v) {
    assert(lept_get_type(v) == LEPT_ARRAY);
    return v->array_size;
}

lept_value* lept_get_array_element(const lept_value* v, size_t index) {
    assert(lept_get_type(v) == LEPT_ARRAY);
    assert(index < v->array_size);
    return v->array + index;
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


static void lept_parse_whitespace(lept_content* c) {
    const char* p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_value(lept_content* c, lept_value* v) {
    switch(*c->json) {
        case 'n': return lept_parse_literal(c, v, "null", LEPT_NULL);
        case 't': return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f': return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case '\"': return lept_parse_string(c, v);
        case '[': return lept_parse_array(c, v);
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

static int lept_parse_number(lept_content* c, lept_value* v) {
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

#define STRING_ERROR(error) do { c->top = old_top; return error; } while (0)

static int lept_parse_string(lept_content* c, lept_value* v) {
    EXPECT(c, '\"');
    const char* p = c->json;
    p++;
    size_t old_top = c->top, length;
    while (1) {
        char ch = *p++;
        switch(ch) {
            case '\"':
                length = c->top - old_top;
                lept_set_string(v, (const char*)lept_content_pop(c, length), length);
                c->json = p;
                return LEPT_PARSE_OK;
            case '\0':
                STRING_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK);
            case '\\':
                switch (*p++) {
                    case '\"':
                        PUTC(c, '\"');
                        break;
                    case '\\':
                        PUTC(c, '\\');
                        break;
                    case '/':
                        PUTC(c, '/');
                        break;
                    case 'b':
                        PUTC(c, '\b');
                        break;
                    case 'f':
                        PUTC(c, '\f');
                        break;
                    case 'n':
                        PUTC(c, '\n');
                        break;
                    case 'r':
                        PUTC(c, '\r');
                        break;
                    case 't':
                        PUTC(c, '\t');
                        break;
                    case 'u': {
                        unsigned u, u2;
                        if (!(p = lept_parse_hex4(p, &u)))
                            STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
                        if (u >= 0xD800 && u <= 0xD8FF) {
                            if (*p++ != '\\')
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            if (*p++ != 'u')
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            if (!(p = lept_parse_hex4(p, &u2)))
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
                            if (u2 < 0xDC00 || u2 > 0xDFFF)
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            u = 0x10000 + ((u - 0xD800) << 10) + (u2 - 0xDC00);
                        }
                        lept_encode_utf8(c, u);
                        break;
                    }
                    default:
                        STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);
                }
                break;
            default:
                if ((unsigned char)ch < 0x20) 
                    STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);
                PUTC(c, ch);
        }
    }
}

static void* lept_content_push(lept_content* c, size_t len) {
    void* ptr; // 返回栈中的缓冲区地址
    assert(len > 0);
    // 缓冲区大小不够，重新分配
    if (c->top + len >= c->size) {
        if (c->size == 0)
            c->size = LEPT_PARSE_STACK_INIT_LENGTH; // 这里不直接进行内存分配
        while (c->top + len >= c->size)
            // 将栈的长度增加至1.5倍
            c->size += c->size >> 1;
        // 分配内存，当c->stack==NULL时该函数作用相当于malloc
        c->stack = realloc(c->stack, c->size);
    }
    ptr = c->stack + c->top;
    c->top += len;
    return ptr;
}

static void* lept_content_pop(lept_content* c, size_t len) {
    assert(c->top >= len);
    return c->stack + (c->top -= len);
}

static const char* lept_parse_hex4(const char* p, unsigned* u) {
    int i;
    *u = 0;
    for (i = 0; i < 4; i++) {
        char ch = *p++;
        *u <<= 4;
        if (ch >= '0' && ch <= '9') *u |= ch - '0';
        else if (ch >= 'A' && ch <= 'F') *u |= ch - 'A' + 10; 
        else if (ch >= 'a' && ch <= 'f') *u |= ch - 'a' + 10;
        else return NULL;
    }
    return p;
} 

static void lept_encode_utf8(lept_content* c, const unsigned u) {
    if (u <= 0x7f) 
        PUTC(c, u & 0xFF);
    else if (u <= 0x07FF) {
        PUTC(c, 0xC0 | (u >> 6 & 0x1F));
        PUTC(c, 0x80 | (u & 0x3F));
    }
    else if (u <= 0xFFFF) {
        PUTC(c, 0xE0 | ((u >> 12) & 0x0F));
        PUTC(c, 0x80 | ((u >> 6) & 0x3F));
        PUTC(c, 0x80 | (u & 0x3F));
    }
    else {
        assert(u <= 0x10FFFF);
        PUTC(c, 0xF0 | ((u >> 18) & 0x07));
        PUTC(c, 0x80 | ((u >> 12) & 0x3F));
        PUTC(c, 0x80 | ((u >> 6) & 0x3F));
        PUTC(c, 0x80 | (u & 0x3F));
    }
}

static int lept_parse_array(lept_content* c, lept_value* v) {
    size_t size = 0; // size of array
    int ret; // 整体解析结果
    EXPECT(c, '[');
    c->json++;
    lept_parse_whitespace(c);
    // 空数组
    if (*c->json == ']') {
        c->json++;
        v->array = NULL;
        v->array_size = size;
        v->type = LEPT_ARRAY;
        return LEPT_PARSE_OK;
    }
    while (1) {
        lept_value e;
        lept_init(&e);
        // 解析该值
        if ((ret = lept_parse_value(c, &e)) != LEPT_PARSE_OK)
            break;
        // 解析成功，将该lept_value存入c的缓存区中
        memcpy(lept_content_push(c, sizeof(lept_value)), &e, sizeof(lept_value));
        size++; // 数组大小增加
        lept_parse_whitespace(c);
        if (*c->json == ',') { // 逗号，后面还有项
            c->json++;
            lept_parse_whitespace(c);
        } 
        else if (*c->json == ']') {
            c->json++;
            v->array_size = size;
            size *= sizeof(lept_value);
            memcpy((v->array = (lept_value*)malloc(size)), lept_content_pop(c, size), size);
            v->type = LEPT_ARRAY;
            return LEPT_PARSE_OK;
        }
        else {
            ret =  LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            break;
        }
    }
    size_t i;
    for (i = 0; i < size; i++) 
        lept_free((lept_value*)lept_content_pop(c, sizeof(lept_value)));
    return ret;
}
