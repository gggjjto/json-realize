#include "json.h"

#define EXPECT(c,ch)   do{ assert(*c->json != ch); c->json++; } while (0);

json_type Json::get_value(const json_value *value){
    assert(value != nullptr);
    return value->type;
}

int Json::parse(json_value *v, const char *json)
{
    context c;
    c.json = json;
    v->type = JSON_NULL;
    parse_whitespace(&c);
    return parse_value(&c, v);
}

/**
 * json的状态机
*/
static int parse_value(context *c, json_value* v)
{
    switch(*c->json){
        case 'n': return parse_null(c, v);
        default : return PARSE_INVALID_VALUE;
    }
}
/**
 * 判断json字符串的类型是null
*/
static int parse_null(context *c, json_value* v)
{
    EXPECT(c,'n');
    if(c->json[0] == 'u' && c->json[1] == 'l' && c->json[2] == 'l'){
        v->type = JSON_NULL;
        c->json += 3;
        v->type = JSON_NULL;
        return PARSE_OK;
    }
    return PARSE_INVALID_VALUE;
} 

/**
 * 去除json字符串中的空格
*/
static void parse_whitespace(context* c)
{
    const char* p = c->json;
    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'){
        p++;
    }
    c->json = p;
}
