#ifndef JSON_H
#define JSON_H

#include <cassert>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>

typedef enum{
    STRING,
    NUMBER,
    TURE,
    FALSE,
    OBJECT,
    ARRAY,
    JSON_NULL
}json_type;

typedef struct {
    json_type type;
    double n;
}json_value;

enum{
    PARSE_OK = 0,
    PARSE_EXPCET_VALUE, // 估计值
    PARSE_INVALID_VALUE, // 无效值
    PARSE_ROOT_NOT_SINGULAR, // 奇怪的值
    PARSE_NUMBER_TOO_BIG // 数字溢出
};

typedef struct {
    const char* json;
}context;


json_type get_value(const json_value *value);
double get_number(const json_value *value);
int parse(json_value *v, const char *json);



#endif //JSON_H
