#ifndef JSON_H
#define JSON_H

#include <cassert>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>

#define json_init(v) do{ (v)->type = JSON_NULL; } while(0)
#define json_set_null(v) json_free(v)

typedef enum{
    STRING,
    NUMBER,
    TRUE,
    FALSE,
    OBJECT,
    ARRAY,
    JSON_NULL
}json_type;

typedef struct {
    json_type type;
    union 
    {
        double n;
        struct {char* s; int len;} s;
    };
}json_value;

enum{
    PARSE_OK = 0,
    PARSE_EXPCET_VALUE, // 估计值
    PARSE_INVALID_VALUE, // 无效值
    PARSE_ROOT_NOT_SINGULAR, // 奇怪的值
    PARSE_NUMBER_TOO_BIG, // 数字溢出
    PARSE_MISS_QUOTATION_MARK, // 引号缺失
    PARSE_INVALID_STRING_ESCAPE // 解析无效字符串转义
};

typedef struct {
    const char* json;
    char* stack;
    size_t size, top;
}context;



void set_number(json_value *v, double n);
void set_boolean(json_value *v, int n);
void set_string(json_value *v, const char* s, size_t len);

json_type get_value(const json_value *value);
double get_number(const json_value *value);
size_t get_string_length(const json_value *v); // 返回string的长度
const char* get_string(const json_value *v); // 返回string
int get_boolean(const json_value *v);

void json_free(json_value *v);
int parse(json_value *v, const char *json);



#endif //JSON_H
