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

#define EXPECT_INT(expect, actual) EXPECT_BASE(expect == actual, expect, actual,"%d")

void test_parse_null() {
    json_value v;
    v.type = FALSE;
    EXPECT_INT(PARSE_OK, parse(&v, "null"));
    EXPECT_INT(JSON_NULL, get_value(&v));
}

void test_parse_expect_value() {
    json_value v;

    v.type = FALSE;
    EXPECT_INT(PARSE_EXPCET_VALUE, parse(&v, ""));
    EXPECT_INT(JSON_NULL, get_value(&v));

    v.type = FALSE;
    EXPECT_INT(PARSE_EXPCET_VALUE, parse(&v, " "));
    EXPECT_INT(JSON_NULL, get_value(&v));
}

void test_parse_invalid_value() {
    json_value v;
    v.type = FALSE;
    EXPECT_INT(PARSE_INVALID_VALUE, parse(&v, "nul"));
    EXPECT_INT(JSON_NULL,get_value(&v));

    v.type = FALSE;
    EXPECT_INT(PARSE_INVALID_VALUE, parse(&v,"?"));
    EXPECT_INT(JSON_NULL,get_value(&v));
}

void test_parse_root_not_singular() {
    json_value v;
    v.type = FALSE;
    EXPECT_INT(PARSE_ROOT_NOT_SINGULAR, parse(&v, "null x"));
    EXPECT_INT(JSON_NULL, get_value(&v));
}

int main(){
    test_parse_null();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    printf("%d/%d (%3.2f%%) passed\n",test_pass, test_count, test_pass * 100.0 / test_count);
    return 0;
}
