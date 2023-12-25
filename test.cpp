#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main_ret = 0;
int test_count = 0;
int test_pass = 0;


#define EXPECT_BASE(equality, expect, actual, format) \
    do{\
        test_count++;\
        if(equality) {test_pass++;}\
        else {fprintf(stderr,"%s:%d: expect:" format " actual: " format "\n", __FILE__, __LINE__, expect, actual); main_ret = 1; }\
    }while(0);
#define TEST_ERROR(error, json)\
    do{\
        json_value v;\
        v.type = FALSE;\
        EXPECT_INT(error, parse(&v, json));\
        EXPECT_INT(JSON_NULL, get_value(&v));\
    }while(0)
#define TEST_NUMBER(expect, json)\
    do{\
        json_value v;\
        EXPECT_INT(PARSE_OK, parse(&v, json));\
        EXPECT_INT(NUMBER, get_value(&v));\
        EXPECT_DOUBLE(expect, get_number(&v));\
    }while(0)
#define TEST_AC_STRING(expect, actual, alength) EXPECT_BASE(sizeof(expect)-1 == (alength) && memcmp(expect, actual, alength) == 0, expect, actual, "%s")
#define EXPECT_INT(expect, actual) EXPECT_BASE(expect == actual, expect, actual,"%d")
#define EXPECT_DOUBLE(expect, actual) EXPECT_BASE(expect == actual, expect, actual,"%.17g")
#define TEST_AC_FALSE(expect) EXPECT_BASE(((bool)expect) == false, "false", "TRUE", "%s")
#define TEST_AC_TRUE(expect) EXPECT_BASE(((bool)expect) == true, "TRUE", "false", "%s")
#define TEST_AC_NUMBER(expect, actual) EXPECT_BASE(expect == actual, expect, actual, "%f")

void test_parse_null() {
    json_value v;
    v.type = FALSE;
    EXPECT_INT(PARSE_OK, parse(&v, "null"));
    EXPECT_INT(JSON_NULL, get_value(&v));
}

void test_parse_true() {
    json_value v;
    v.type = TRUE;
    EXPECT_INT(PARSE_OK, parse(&v, "true"));
    EXPECT_INT(TRUE, get_value(&v));
}

void test_parse_false() {
    json_value v;
    v.type = FALSE;
    EXPECT_INT(PARSE_OK, parse(&v, "false"));
    EXPECT_INT(FALSE, get_value(&v));
}
void test_parse_number() {
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

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

void test_parse_expect_value() {
    /* \n\t*/
    TEST_ERROR(PARSE_EXPCET_VALUE, "");
    TEST_ERROR(PARSE_EXPCET_VALUE, " ");
}


void test_parse_invalid_value() {
    /* null */
    TEST_ERROR(PARSE_INVALID_VALUE,"nul");
    TEST_ERROR(PARSE_INVALID_VALUE,"?");
    /* invalid number */
    TEST_ERROR(PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(PARSE_INVALID_VALUE, "nan");
}

void test_parse_root_not_singular() {
    /* null */
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "null x");
    /* invalid number */
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0x123");
}

void test_parse_number_too_big() {
    TEST_ERROR(PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(PARSE_NUMBER_TOO_BIG, "-1e309");
}

void test_access_string() {
    json_value v;
    json_init(&v);
    set_string(&v, "", 0);
    TEST_AC_STRING("", get_string(&v), get_string_length(&v));
    set_string(&v, "Hello", 5);
    TEST_AC_STRING("Hello", get_string(&v), get_string_length(&v));
    json_free(&v);
}

void test_access_null() {
    json_value v;
    json_init(&v);
    set_string(&v, "a", 1);
    json_set_null(&v);
    EXPECT_INT(JSON_NULL, get_value(&v));
    json_free(&v);
}


void test_access_boolean() {
    json_value v;
    json_init(&v);
    set_boolean(&v, 1);
    TEST_AC_TRUE(get_boolean(&v));
    set_boolean(&v, 0);
    TEST_AC_FALSE(get_boolean(&v));
    json_free(&v);
}


void test_access_number() {
    json_value v;
    json_init(&v);
    set_number(&v, 123.0);
    TEST_AC_NUMBER(123.0, get_number(&v));
    json_free(&v);
}

void test_parse(){
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();

    test_access_string();
    test_access_boolean();
    test_access_null();
    test_access_number();
}

int main(){
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n",test_pass, test_count, test_pass * 100.0 / test_count);
    return 0;
}
