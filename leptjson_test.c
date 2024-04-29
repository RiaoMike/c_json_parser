#include <stdio.h>
#include <string.h> /* memcmp */
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
            fprintf(stderr, "%s:%d" " expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual); \
            main_ret = 1; \
        } \
    } while (0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, ".17g")

#define EXPECT_EQ_STRING(expect, actual, slength) \
    EXPECT_EQ_BASE((sizeof(expect) - 1 == slength && memcmp(expect, actual, slength) == 0), expect, actual, "%s")
#define EXPECT_EQ_TRUE(actual) EXPECT_EQ_BASE((actual != 0), "true", "false", "%s")
#define EXPECT_EQ_FALSE(actual) EXPECT_EQ_BASE((actual == 0), "false", "true", "%s")

// 合并错误检测
#define TEST_ERROR(error, json) \
    do { \
        lept_value v; \
        lept_init(&v); \
        v.type = LEPT_FALSE; \
        EXPECT_EQ_INT(error, lept_parse(&v, json)); \
        EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v)); \
    } while (0)

// 测试解析null类型
static void test_parse_null() {
    lept_value v;

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "null"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "  null\n"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}

// 测试解析ture类型
static void test_parse_true() {
    lept_value v;

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "true"));
    EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "  true\n"));
    EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));
}

// 测试解析false类型
static void test_parse_false() {
    lept_value v;

    v.type = LEPT_TRUE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "false"));
    EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));

    v.type = LEPT_TRUE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "  false\n"));
    EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));
}

// 测试数字值
#define TEST_NUMBER(expect, json) \
    do { \
        lept_value v; \
        lept_init(&v); \
        v.type = LEPT_FALSE; \
        EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json)); \
        EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v)); \
        EXPECT_EQ_DOUBLE(expect, lept_get_number(&v)); \
    } while (0)

static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000");

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324" ); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308" );  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308" );  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308" );  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_STRING(expect, json, slength) \
    do { \
        lept_value v; \
        lept_init(&v); \
        EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json)); \
        EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&v)); \
        EXPECT_EQ_STRING(expect, lept_get_string(&v), lept_get_string_length(&v)); \
       lept_free(&v); \
    } while (0)

static void test_parse_string() {
    TEST_STRING("", "\"\"", 0);
    TEST_STRING("Hello", "\"Hello\"", 5);
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"", 11);
    TEST_STRING("\" \\ / \t \f \b \n \r", "\"\\\" \\\\ \\/ \\t \\f \\b \\n \\r\"", 15);

    TEST_STRING("Hello\0World", "\"Hello\\u0000World\"", 11);
    TEST_STRING("\x24", "\"\\u0024\"", 1);
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"", 2);
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\"", 3);
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"", 4);
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"", 4);
}

// 测试解析全空类型错误
static void test_parse_expect_value() {
    TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, " \n\t\r");
}

// 测试非法值错误
static void test_parse_invalid_value() {
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "?");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nu? x");

    // 测试数字串不合法
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, ".123");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.23e");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nan");
}

// 测试空格后面有值错误
static void test_parse_root_not_singular() {
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "true x");

    // 测试数字串不合法
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0123");
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x123");
}

// 测试数字溢出
static void test_parse_number_too_big() {
    TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "1e10001");
    TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "-1e10001");
}

// 测试字符串 引号缺失，非法转义，非法字符
static void test_parse_miss_quotation_mark() {
    TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"abcd");
}

static void test_parse_invalid_string_escape() {
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\012\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\x123\"");
}

static void test_parse_invalid_string_char() {
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x10\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_access_null() {
    lept_value v;
    lept_init(&v);
    lept_set_string(&v, "a", 1);
    lept_set_null(&v);
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
    lept_free(&v);
}

static void test_access_boolean() {
    lept_value v;
    lept_init(&v);
    lept_set_string(&v, "a", 1);
    lept_set_boolean(&v, 1);
    EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));
    lept_set_boolean(&v, 0);
    EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));
    lept_free(&v);
}

static void test_access_number() {
    lept_value v;
    lept_init(&v);
    lept_set_string(&v, "a", 1);
    lept_set_number(&v, 123.4);
    EXPECT_EQ_DOUBLE(123.4, lept_get_number(&v));
    lept_free(&v);
}

static void test_access_string() {
    lept_value v;
    lept_init(&v);
    lept_set_string(&v, "", 0);
    EXPECT_EQ_STRING("", lept_get_string(&v), lept_get_string_length(&v));
    lept_set_string(&v, "Hello", 5);
    EXPECT_EQ_STRING("Hello", lept_get_string(&v), lept_get_string_length(&v));
    lept_free(&v);
}

static void test_parse_invalid_unicode_hex() {
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u/123\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\uG123\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/23\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0G23\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u01/3\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u01G3\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u012/\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u012G\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u 0123\"");
}

static void test_parse_invalid_unicode_surrogate() {
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD8FF\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uD8FF\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

// 综合测试
static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_string();

    test_parse_number_too_big();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_miss_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
    test_access_null();
    test_access_boolean();
    test_access_number();
    test_access_string();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();
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
