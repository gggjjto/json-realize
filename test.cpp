#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main_ret = 0;
int test_count = 0;
int test_pass = 0;

#define EXPECT_BASE(equality, expect, actual, format)                                                             \
    do                                                                                                            \
    {                                                                                                             \
        test_count++;                                                                                             \
        if (equality)                                                                                             \
        {                                                                                                         \
            test_pass++;                                                                                          \
        }                                                                                                         \
        else                                                                                                      \
        {                                                                                                         \
            fprintf(stderr, "%s:%d: expect:" format " actual: " format "\n", __FILE__, __LINE__, expect, actual); \
            main_ret = 1;                                                                                         \
        }                                                                                                         \
    } while (0);

#define TEST_AC_STRING(expect, actual, alength) EXPECT_BASE(sizeof(expect) - 1 == (alength) && memcmp(expect, actual, alength) == 0, expect, actual, "%s")
#define TEST_AC_INT(expect, actual) EXPECT_BASE(expect == actual, expect, actual, "%d")
#define TEST_AC_DOUBLE(expect, actual) EXPECT_BASE(expect == actual, expect, actual, "%.17g")
#define TEST_AC_FALSE(expect) EXPECT_BASE(((bool)expect) == false, "false", "true", "%s")
#define TEST_AC_TRUE(expect) EXPECT_BASE(((bool)expect) == true, "true", "false", "%s")
#define TEST_AC_NUMBER(expect, actual) EXPECT_BASE(expect == actual, expect, actual, "%f")

void test_parse_null()
{
    json_value v;
    v.type = FALSE;
    TEST_AC_INT(PARSE_OK, parse(&v, "null"));
    TEST_AC_INT(JSON_NULL, get_value(&v));
}

void test_parse_true()
{
    json_value v;
    v.type = TRUE;
    TEST_AC_INT(PARSE_OK, parse(&v, "true"));
    TEST_AC_INT(TRUE, get_value(&v));
}

void test_parse_false()
{
    json_value v;
    v.type = FALSE;
    TEST_AC_INT(PARSE_OK, parse(&v, "false"));
    TEST_AC_INT(FALSE, get_value(&v));
}

#define TEST_NUMBER(expect, json)               \
    do                                          \
    {                                           \
        json_value v;                           \
        TEST_AC_INT(PARSE_OK, parse(&v, json)); \
        TEST_AC_INT(NUMBER, get_value(&v));     \
        TEST_AC_DOUBLE(expect, get_number(&v)); \
    } while (0)

void test_parse_number()
{
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
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
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002");           /* the smallest number > 1 */
    TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308"); /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308"); /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308"); /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_STRING(expect, json)               \
    do                                          \
    {                                           \
        json_value v;                           \
        json_init(&v);                          \
        TEST_AC_INT(PARSE_OK, parse(&v, json)); \
        TEST_AC_INT(STRING, get_value(&v));         \
        TEST_AC_STRING(expect, get_string(&v), get_string_length(&v));\
        json_free(&v);\
    } while (0)

void parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
    TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

#define TEST_ERROR(error, json)                \
    do                                         \
    {                                          \
        json_value v;                          \
        v.type = FALSE;                        \
        TEST_AC_INT(error, parse(&v, json));   \
        TEST_AC_INT(JSON_NULL, get_value(&v)); \
    } while (0)

void test_parse_expect_value()
{
    /* \n\t*/
    TEST_ERROR(PARSE_EXPCET_VALUE, "");
    TEST_ERROR(PARSE_EXPCET_VALUE, " ");
}

void test_parse_invalid_value()
{
    /* null */
    TEST_ERROR(PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(PARSE_INVALID_VALUE, "?");
    /* invalid number */
    TEST_ERROR(PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(PARSE_INVALID_VALUE, "nan");

    /* invalid value in array */
    TEST_ERROR(PARSE_INVALID_VALUE, "[1,]");
    TEST_ERROR(PARSE_INVALID_VALUE, "[\"a\", nul]");
}

void test_parse_root_not_singular()
{
    /* null */
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "null x");
    /* invalid number */
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0x123");
}

void test_parse_number_too_big()
{
    TEST_ERROR(PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(PARSE_NUMBER_TOO_BIG, "-1e309");
}

void test_parse_missing_quotation_mark() {
    TEST_ERROR(PARSE_MISS_QUOTATION_MARK,"\"");
    TEST_ERROR(PARSE_MISS_QUOTATION_MARK,"\"abc");
}

void test_parse_invalid_string_escape() {
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

void test_parse_invalid_string_char() {
    TEST_ERROR(PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

void test_parse_invalid_unicode_hex() {
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

void test_parse_invalid_unicode_surrogate() {
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_parse_miss_comma_or_square_bracket() {
    TEST_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
    TEST_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
    TEST_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
    TEST_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}
#if defined(_MSC_VER)
#define EXPECT_AC_SIZE_T(expect, actual) EXPECT_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%Iu")
#else
#define EXPECT_AC_SIZE_T(expect, actual) EXPECT_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
#endif

void test_parse_array()
{
    size_t i, j;
    json_value v;

    json_init(&v);
    TEST_AC_INT(PARSE_OK, parse(&v, "[ ]"));
    TEST_AC_INT(ARRAY, get_value(&v));
    EXPECT_AC_SIZE_T(0, get_array_size(&v));
    json_free(&v);

    json_init(&v);
    TEST_AC_INT(PARSE_OK, parse(&v, "[ null , false , true , 123 , \"abc\" ]"));
    TEST_AC_INT(ARRAY, get_value(&v));
    EXPECT_AC_SIZE_T(5, get_array_size(&v));
    TEST_AC_INT(JSON_NULL, get_value(get_array_element(&v, 0)));
    TEST_AC_INT(FALSE, get_value(get_array_element(&v, 1)));
    TEST_AC_INT(TRUE, get_value(get_array_element(&v, 2)));
    TEST_AC_INT(NUMBER, get_value(get_array_element(&v, 3)));
    TEST_AC_INT(STRING, get_value(get_array_element(&v, 4)));
    TEST_AC_DOUBLE(123.0, get_number(get_array_element(&v, 3)));
    TEST_AC_STRING("abc", get_string(get_array_element(&v, 4)), get_string_length(get_array_element(&v, 4)));
    json_free(&v);

    json_init(&v);
    TEST_AC_INT(PARSE_OK, parse(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    TEST_AC_INT(ARRAY, get_value(&v));
    EXPECT_AC_SIZE_T(4, get_array_size(&v));
    for (i = 0; i < 4; i++) {
        json_value* a = get_array_element(&v, i);
        TEST_AC_INT(ARRAY, get_value(a));
        EXPECT_AC_SIZE_T(i, get_array_size(a));
        for (j = 0; j < i; j++) {
            json_value* e = get_array_element(a, j);
            TEST_AC_INT(NUMBER, get_value(e));
            TEST_AC_DOUBLE((double)j, get_number(e));
        }
    }
    json_free(&v);
}

void test_access_null()
{
    json_value v;
    json_init(&v);
    set_string(&v, "a", 1);
    json_set_null(&v);
    TEST_AC_INT(JSON_NULL, get_value(&v));
    json_free(&v);
}

void test_access_boolean()
{
    json_value v;
    json_init(&v);
    set_string(&v, "a", 1);
    set_boolean(&v, 1);
    TEST_AC_TRUE(get_boolean(&v));
    set_boolean(&v, 0);
    TEST_AC_FALSE(get_boolean(&v));
    json_free(&v);
}

void test_access_number()
{
    json_value v;
    json_init(&v);
    set_string(&v, "a", 1);
    set_number(&v, 123.0);
    TEST_AC_DOUBLE(123.0, get_number(&v));
    json_free(&v);
}

void test_access_string()
{
    json_value v;
    json_init(&v);
    set_string(&v, "", 0);
    TEST_AC_STRING("", get_string(&v), get_string_length(&v));
    set_string(&v, "Hello", 5);
    TEST_AC_STRING("Hello", get_string(&v), get_string_length(&v));
    json_free(&v);
}

void test_parse()
{
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_array();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();
    test_parse_miss_comma_or_square_bracket();
    
    test_access_string();
    test_access_boolean();
    test_access_null();
    test_access_number();
}

int main()
{
#ifdef _WINDOWS
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return 0;
}
