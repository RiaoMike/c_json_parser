#ifndef LEPTJSON_H_
#define LEPTJSON_H_

#include <assert.h>
#include <stdio.h>

// 用于判断json类型是否是期望类型
#define EXPECT(c, ch) do { assert(*c->json == (ch)); } while (0)

// 用于判断json数字类型是否合法
#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT0TO9(ch) ((ch) >= '1' && (ch) <= '9')

// 初始化lept_value节点为null类型
#define lept_init(v) do { (v)->type = LEPT_NULL; } while(0)

// 表示json值类型
typedef enum { LEPT_NULL, LEPT_TRUE, LEPT_FALSE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT} lept_type;

// 存储json待解析值
typedef struct {
    const char* json;
    char* stack; // 缓冲区
    size_t size, top; // 栈大小以及栈顶
} lept_content;

typedef struct lept_value lept_value;

// json解析树节点
struct lept_value {
    union {
        struct {
            lept_value* array;
            size_t array_size;
        };
        struct {
            char* s;
            size_t len;
        }; // useful only when type --> LEPT_STRING
        double n; // useful only when type --> LEPT_NUMBER
    };
    lept_type type;
};

// 解析函数返回值
// 在非法值错误和空格后面还有值之间优先返回非法值错误
enum {
    LEPT_PARSE_OK, // 解析成功
    LEPT_PARSE_EXPECT_VALUE, // 全部是空格
    LEPT_PARSE_INVALID_VALUE, // 非法值
    LEPT_PARSE_ROOT_NOT_SINGULAR, // 空格后面还有值
    LEPT_PARSE_NUMBER_TOO_BIG, // 解析数字太大
    LEPT_PARSE_MISS_QUOTATION_MARK, // 字符串没有没有右引号
    LEPT_PARSE_INVALID_STRING_ESCAPE, // 不合法转移字符
    LEPT_PARSE_INVALID_STRING_CHAR, // 不合法字符
    LEPT_PARSE_INVALID_UNICODE_HEX, // 不合法unicode字符
    LEPT_PARSE_INVALID_UNICODE_SURROGATE,
    LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET
};

// 主要解析函数
int lept_parse(lept_value* v, const char* json);

// 释放lept内部变量，并将类型置为NULL
void lept_free(lept_value* v); 

// 获取节点中json值类型
lept_type lept_get_type(const lept_value* v);

// NULL类型
#define lept_set_null(v) lept_free(v)

// true/false类型
int lept_get_boolean(const lept_value* v);
void lept_set_boolean(lept_value* v, int b);

// 获取数字节点上的值
double lept_get_number(const lept_value* v);
void lept_set_number(lept_value* v, double d);

// string类型函数
const char* lept_get_string(const lept_value* v);
size_t lept_get_string_length(const lept_value* v);
// 将缓冲区字符串压入lept_value
void lept_set_string(lept_value* v, const char* c, size_t len);

// array类型函数
size_t lept_get_array_size(const lept_value* v);
lept_value* lept_get_array_element(const lept_value* v, size_t index);

// static function
static void lept_parse_whitespace(lept_content* c);

static int lept_parse_value(lept_content* c, lept_value* v);

static int lept_parse_literal(lept_content* c, lept_value* v, const char* literal, lept_type type);

static int lept_parse_number(lept_content* c, lept_value* v);

static int lept_parse_string(lept_content* c, lept_value* v);

// 第一次分配缓存空间时的大小
#ifndef LEPT_PARSE_STACK_INIT_LENGTH
#define LEPT_PARSE_STACK_INIT_LENGTH 256
#endif

// 将字符ch推进lept_content:c的缓冲区中
#define PUTC(c, ch) do { *(char*)lept_content_push(c, sizeof(char)) = (ch); } while (0)

static void* lept_content_push(lept_content* c, size_t len);
static void* lept_content_pop(lept_content* c, size_t len);

static const char* lept_parse_hex4(const char* p, unsigned* u); 
static void lept_encode_utf8(lept_content* c, const unsigned u);

static int lept_parse_array(lept_content* c, lept_value* v);


#endif
