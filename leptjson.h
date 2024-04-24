#ifndef LEPTJSON_H_
#define LEPTJSON_H_

#include <assert.h>

// 用于判断json类型是否是期望类型
#define EXPECT(c, ch) do { assert(*c->json == (ch)); } while (0)

// 用于判断json数字类型是否合法
#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT0TO9(ch) ((ch) >= '1' && (ch) <= '9')

// 表示json值类型
typedef enum { LEPT_NULL, LEPT_TRUE, LEPT_FALSE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT} lept_type;

// 存储json待解析值
typedef struct {
    const char* json;
} lept_content;

// json解析树节点
typedef struct {
    double n; // useful only when type --> LEPT_NUMBER
    lept_type type;
} lept_value;

// 解析函数返回值
// 在非法值错误和空格后面还有值之间优先返回非法值错误
enum {
    LEPT_PARSE_OK = 0, // 解析成功
    LEPT_PARSE_EXPECT_VALUE, // 全部是空格
    LEPT_PARSE_INVALID_VALUE, // 非法值
    LEPT_PARSE_ROOT_NOT_SINGULAR, // 空格后面还有值
    LEPT_PARSE_NUMBER_TOO_BIG // 解析数字太大
};

// 解析json串开头的空白字符，包括空白，制表，换行，回车
static void lept_parse_whitespace(lept_content* c);

// 解析json字面量json值， null, true, false
// literal 为 "null", "true", "false"
static int lept_parse_literal(lept_content* c, lept_value* v, const char* literal, lept_type type);

// 解析json数字类型
static double lept_parse_number(lept_content* c, lept_value* v);

// 解析json串，被主函数lept_parse调用
static int lept_parse_value(lept_content* c, lept_value* v);

// 主要解析函数
int lept_parse(lept_value* v, const char* json);

// 获取节点中json值类型
lept_type get_lept_type(const lept_value* v);

// 获取数字节点上的值
double get_lept_number(const lept_value* v);

#endif
