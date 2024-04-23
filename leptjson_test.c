#include <stdio.h>
#include "leptjson.h"

static int main_ret = 0; // 整体是否通过
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(statement, expect, actual, format) \
    do { \
        test_count++;\
        if (statement) \
            test_pass++; \
        else { \
            fprintf(stderr, "%s:%d" " expect: " format " actual: " format, __FILE__, __LINE__, expect, actual); \
            main_ret = 1; \
        } \
    } while (0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

// 测试解析null类型
static void test_parse_null() {
    lept_value v;

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "null"));
    EXPECT_EQ_INT(LEPT_NULL, get_lept_type(&v));

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "  null\n"));
    EXPECT_EQ_INT(LEPT_NULL, get_lept_type(&v));
}

// 测试解析ture类型
static void test_parse_true() {
    lept_value v;

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "true"));
    EXPECT_EQ_INT(LEPT_TRUE, get_lept_type(&v));

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "  true\n"));
    EXPECT_EQ_INT(LEPT_TRUE, get_lept_type(&v));
}

// 测试解析false类型
static void test_parse_false() {
    lept_value v;

    v.type = LEPT_TRUE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "false"));
    EXPECT_EQ_INT(LEPT_FALSE, get_lept_type(&v));

    v.type = LEPT_TRUE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "  false\n"));
    EXPECT_EQ_INT(LEPT_FALSE, get_lept_type(&v));
}

// 测试解析全空类型错误
static void test_parse_expect_value() {
    lept_value v;

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, ""));
    EXPECT_EQ_INT(LEPT_NULL, get_lept_type(&v));

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, " "));
    EXPECT_EQ_INT(LEPT_NULL, get_lept_type(&v));
}

// 测试非法值错误
static void test_parse_invalid_value() {
    lept_value v;

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "nul"));
    EXPECT_EQ_INT(LEPT_NULL, get_lept_type(&v));

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "?"));
    EXPECT_EQ_INT(LEPT_NULL, get_lept_type(&v));

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "n? null"));
    EXPECT_EQ_INT(LEPT_NULL, get_lept_type(&v));
}

// 测试空格后面有值错误
static void test_parse_root_not_singular() {
    lept_value v;

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "null x"));
    EXPECT_EQ_INT(LEPT_NULL, get_lept_type(&v));
}

// 综合测试
static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
}

int main() {
    test_parse();
    printf("test_count: %d, test_pass: %d, pass_rate: %3.2f%%\n", test_count, test_pass, test_pass * 100.0 / test_count);
    if (main_ret == 0) 
        printf("Success!\n");
    else
        printf("Fail!\n");
    return 0;
}
