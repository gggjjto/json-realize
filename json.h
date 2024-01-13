#ifndef JSON_H
#define JSON_H

#include <cassert>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <stddef.h>

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

typedef struct json_value json_value;
typedef struct json_member json_member;
struct json_value {
    json_type type;
    union 
    {
        double n;  /* number */
        struct { char* s; int len; } s; /* string */
        struct { json_value* e; size_t size; }a; /* array */
        struct { json_member* m; size_t size; }o; 
    };
};

struct json_member {
    char* k; size_t klen;
    json_value v;
};

enum{
    PARSE_OK = 0,
    PARSE_EXPCET_VALUE, // 估计值
    PARSE_INVALID_VALUE, // 无效值
    PARSE_ROOT_NOT_SINGULAR, // 奇怪的值
    PARSE_NUMBER_TOO_BIG, // 数字溢出
    PARSE_MISS_QUOTATION_MARK, // 引号缺失
    PARSE_INVALID_STRING_ESCAPE, // 解析无效字符串转义
    PARSE_INVALID_STRING_CHAR, // 解析无效的字符串字符
    PARSE_INVALID_UNICODE_HEX, // 解析无效的unicode十六进制
    PARSE_INVALID_UNICODE_SURROGATE, // 解析无效的unicode代理
    PARSE_MISS_COMMA_OR_SQUARE_BRACKET // 解析逗号或方括号
};

typedef struct {
    const char* json;
    char* stack;
    size_t size, top;
}context;



void set_number(json_value *v, double n);
double get_number(const json_value *value);

void set_boolean(json_value *v, int n);
int get_boolean(const json_value *v);

void set_string(json_value *v, const char* s, size_t len);
size_t get_string_length(const json_value *v); // 返回string的长度
const char* get_string(const json_value *v); // 返回string

json_type get_value(const json_value *value);
size_t get_array_size(const json_value* v);
json_value* get_array_element(const json_value* v, size_t index);

size_t get_object_size(const json_value* v);
const char* get_object_key(const json_value* v, size_t index);
size_t get_object_key_length(const json_value* v, size_t index);
json_value* get_object_value(const json_value* v, size_t index);


void json_free(json_value *v);
int parse(json_value *v, const char *json);



#endif //JSON_H
