#include "json.h"

#define EXPECT(c,ch)   do{ assert(*c->json == (ch)); c->json++; } while (0);
#define ISDIGIT(ch)   (ch >= '0' && ch<='9')
#define ISDIGIT1TO9(ch)   (ch >= '1' && ch <= '9')

static int parse_value(context *c, json_value* v);
static int parse_null(context *c, json_value* v);
static void parse_whitespace(context* c);
static int parse_true(context *c, json_value *v);
static int parse_false(context *c, json_value *v);
static int parse_number(context *c, json_value *v);
static int parse_literal(context *c, json_value *v, const char* literal, json_type type);

json_type get_value(const json_value *value){
    assert(value != nullptr);
    return value->type;
}

double get_number(const json_value *value){
    assert(value != nullptr && value->type == NUMBER);
    return value->n;
}

int parse(json_value *v, const char *json)
{
    context c;
    int ret;
    c.json = json;
    v->type = JSON_NULL;
    parse_whitespace(&c);
    if((ret = parse_value(&c, v)) == PARSE_OK){
        parse_whitespace(&c);
        if(*c.json != '\0'){
            v->type = JSON_NULL;
            ret = PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

/**
 * json的状态机
*/
static int parse_value(context *c, json_value* v)
{
    switch(*c->json){
        // case 't': return parse_true(c, v);
        case 't': return parse_literal(c, v, "true", TURE);
        // case 'f': return parse_false(c, v);
        case 'f': return parse_literal(c, v, "false", FALSE);
        // case 'n': return parse_null(c, v);
        case 'n': return parse_literal(c, v, "null", JSON_NULL);
        case '\0': return PARSE_EXPCET_VALUE;
        default : return parse_number(c, v);
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
        // if(*c->json != '\0') { return PARSE_ROOT_NOT_SINGULAR; }
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

static int parse_true(context *c, json_value *v){
    EXPECT(c, 't');
    if(c->json[0] == 'r' && c->json[1] == 'u' && c->json[2] == 'e'){
        v->type = TURE;
        c->json += 3;
        return PARSE_OK;
    }
    return PARSE_INVALID_VALUE;
}

static int parse_false(context *c, json_value *v){
    EXPECT(c, 'f');
    if(c->json[0] == 'a' && c->json[1] == 'l' && c->json[2] == 's' && c->json[3] == 'e'){
        v->type = FALSE;
        c->json += 4;
        return PARSE_OK;
    }
    return PARSE_INVALID_VALUE;
}

/**
 * 字符转数字
*/
static int parse_number(context *c, json_value *v){
    char *end;
    const char* p = c->json;
    if(*p == '-') p++;
    if(*p == '0') p++;
    else{
        if(!ISDIGIT1TO9(*p)){
            return PARSE_INVALID_VALUE;
        }
        p++;
        while(ISDIGIT(*p)) p++;
    }
    if(*p == '.') {
        p++;    
        if(ISDIGIT(*p)){
            p++;
            while(ISDIGIT(*p)) p++;
        }else{
            return PARSE_INVALID_VALUE;
        }
    }
    if(*p == 'e' || *p == 'E'){
        p++;
        if(*p == '-' || *p == '+'){
            p++;
        }
        if(!ISDIGIT(*p)) return PARSE_INVALID_VALUE;
        p++;
        while(ISDIGIT(*p)) p++;
    }
    errno = 0;
    v->n = strtod(c->json,NULL);
    if(errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)){
        return PARSE_NUMBER_TOO_BIG;
    }
    v->type = NUMBER;
    c->json = p;
    return PARSE_OK;
    
}
/**
 * 处理字符串匹配
*/
static int parse_literal(context *c, json_value *v, const char* literal, json_type type)
{
    int i;
    EXPECT(c,literal[0]);
    for(i = 0; literal[i + 1]; i++){
        //std::cout<<literal[i + 1]<<"=="<<c->json[i]<<'\n';
        if(literal[i + 1] != c->json[i]){
            //std::cout<<literal[i + 1]<<"!="<<c->json[i]<<'\n';
            return PARSE_INVALID_VALUE;
        }
    }
    c->json += i;
    v->type = type;
    return PARSE_OK;
}
