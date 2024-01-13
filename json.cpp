#include "json.h"

#define EXPECT(c,ch)   do{ assert(*c->json == (ch)); c->json++; } while (0);
#define ISDIGIT(ch)   (ch >= '0' && ch<='9')
#define ISDIGIT1TO9(ch)   (ch >= '1' && ch <= '9')
#define PUTC(c, ch) do{ *(char*)context_push(c, sizeof(char)) = (ch); }while(0);

int parse_value(context *c, json_value* v);
int parse_null(context *c, json_value* v);
void parse_whitespace(context* c);
int parse_true(context *c, json_value *v);
int parse_false(context *c, json_value *v);
int parse_number(context *c, json_value *v);
int parse_string(context* c, json_value* v);
int parse_literal(context *c, json_value *v, const char* literal, json_type type);
int parse_array(context *c, json_value *v);
void* context_push(context* c, size_t size);
void* context_pop(context* c, size_t size);

#ifndef PARSE_STACK_INIT_SIZE
#define PARSE_STACK_INIT_SIZE 256
#endif

void* context_push(context* c, size_t size)
{
    void* ret;
    assert(size > 0);
    if(c->top + size >= c->size) {
        if(c->size == 0) {
            c->size = PARSE_STACK_INIT_SIZE;
        }
        while(c->top + size >= c->size) {
            c->size += c->size >> 1;  /* c-size * 1.5 */
        }
        c->stack = (char*)realloc(c->stack, c->size);
    }
    ret = c->stack + c->top;
    c->top += size;
    return ret;
}

void* context_pop(context* c,size_t size)
{
    assert(c->top >= size);
    return c->stack + (c->top -= size);
}


json_type get_value(const json_value *value){
    assert(value != nullptr);
    return value->type;
}

double get_number(const json_value *value){
    assert(value != nullptr && value->type == NUMBER);
    return value->n;
}

void set_number(json_value *v, double n){
    assert(v != NULL);
    json_free(v);
    v->type = NUMBER;
    v->n = n;
}

void set_string(json_value* v, const char* s, size_t len)
{
    assert(v != nullptr && (s != NULL || len == 0));
    json_free(v);
    v->s.s = (char *)malloc(len + 1);
    memcpy((void *)v->s.s, s, len);
    v->s.s[len] = '\0';
    v->s.len = len;
    v->type = STRING;
}

const char* get_string(const json_value* v)
{
    assert(v != NULL);
    return v->s.s;
}

size_t get_string_length(const json_value* v)
{
    assert(v != NULL);
    return v->s.len;
}

void set_boolean(json_value* v, int n)
{
    assert(v != NULL);
    json_free(v);
    v->type = n ? TRUE : FALSE;
    v->n = n;
}

int get_boolean(const json_value* v)
{
    assert(v != NULL && (v->type == FALSE || v->type == TRUE));
    return v->n;
}

size_t get_array_size(const json_value* v) 
{
    assert(v != NULL && v->type == ARRAY);
    return v->a.size;
}

json_value* get_array_element(const json_value* v, size_t index)
{
    assert(v != NULL && v->type == ARRAY);
    assert(index < v->a.size);
    return &v->a.e[index];
}

void json_free(json_value* v)
{
    assert(v != NULL);
    switch(v->type) 
    {
        case STRING :
            free(v->s.s);
            break;
        case ARRAY :
            for (size_t i = 0; i < v->a.size; i++){
                json_free(&v->a.e[i]);
            }
            free(v->a.e);
            break;
        default: break;
    }
    v->type = JSON_NULL;
}

int parse(json_value *v, const char *json)
{
    context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    c.stack = NULL;
    c.size = c.top = 0;
    json_init(v);
    parse_whitespace(&c);
    if((ret = parse_value(&c, v)) == PARSE_OK){
        parse_whitespace(&c);
        if(*c.json != '\0'){
            v->type = JSON_NULL;
            ret = PARSE_ROOT_NOT_SINGULAR;
        }
    }
    assert(c.top == 0);
    free(c.stack);
    return ret;
}

/**
 * json的状态机
*/
int parse_value(context *c, json_value* v)
{
    switch(*c->json){
        case 't': return parse_literal(c, v, "true", TRUE);
        case 'f': return parse_literal(c, v, "false", FALSE);
        case 'n': return parse_literal(c, v, "null", JSON_NULL);
        case '"': return parse_string(c, v);
        case '[': return parse_array(c, v);
        case '\0': return PARSE_EXPCET_VALUE;
        default : return parse_number(c, v);
    }
}
/**
 * 判断json字符串的类型是null
*/
int parse_null(context *c, json_value* v)
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
void parse_whitespace(context* c)
{
    const char* p = c->json;
    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'){
        p++;
    }
    c->json = p;
}

int parse_true(context *c, json_value *v){
    EXPECT(c, 't');
    if(c->json[0] == 'r' && c->json[1] == 'u' && c->json[2] == 'e'){
        v->type = TRUE;
        c->json += 3;
        return PARSE_OK;
    }
    return PARSE_INVALID_VALUE;
}

int parse_false(context *c, json_value *v){
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
int parse_number(context *c, json_value *v){
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
int parse_literal(context *c, json_value *v, const char* literal, json_type type)
{
    int i;
    EXPECT(c,literal[0]);
    for(i = 0; literal[i + 1]; i++){
        if(literal[i + 1] != c->json[i]){
            return PARSE_INVALID_VALUE;
        }
    }
    c->json += i;
    v->type = type;
    return PARSE_OK;
}

const char* parse_hex4(const char* p, unsigned *u)
{
    *u = 0;
    for (int i = 1; i <= 4; i++)
    {
        char ch = *p++;
        *u <<= 4;
        if      (ch >= '0' && ch <= '9') *u |= (ch - '0');
        else if (ch >= 'A' && ch <= 'F') *u |= (ch - 'A') + 10;
        else if (ch >= 'a' && ch <= 'f') *u |= (ch - 'a') + 10;
        else return NULL;
    }
    return p;
}

void encode_utf8(context* c, unsigned u)
{
    if (u < 0x7F) {
        PUTC(c, u & 0xFF);
    }
    else if (u <= 0x7FF) {
        PUTC(c, 0xC0 | ((u >> 6) & 0xFF));
        PUTC(c, 0x80 | ( u       & 0x3F));
    }
    else if (u <= 0xFFFF) {
        PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
        PUTC(c, 0x80 | ((u >> 6)  & 0x3F));
        PUTC(c, 0x80 | ( u        & 0x3F));
    }
    else {
        assert(u <= 0x10FFFF);
        PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
        PUTC(c, 0x80 | ((u >> 12) & 0x3F));
        PUTC(c, 0x80 | ((u >> 6)  & 0x3F));
        PUTC(c, 0x80 | ( u        & 0x3F));
    }
}

#define STRING_ERROR(ret) do{ c->top = head; return ret; }while(0)

int parse_string(context* c, json_value* v)  
{
    size_t head = c->top, len;
    unsigned u, u2;
    const char* p;
    EXPECT(c, '\"');
    p = c->json;
    while(1)
    {
        char ch = *p++;
        switch (ch) {
            case '\"':
                len = c->top - head;
                set_string(v, (const char*)context_pop(c, len), len);
                c->json = p;
                return PARSE_OK;
            case '\0':
                STRING_ERROR(PARSE_MISS_QUOTATION_MARK);
            case '\\':
                switch(*p++) {
                    case '\"': PUTC(c, '\"'); break;
                    case '\\': PUTC(c,'\\');  break;
                    case '/':  PUTC(c,'/');   break;
                    case 'b':  PUTC(c,'b');   break;
                    case 'f':  PUTC(c,'f');   break;
                    case 'n':  PUTC(c,'n');   break;
                    case 'r':  PUTC(c,'r');   break;
                    case 't':  PUTC(c,'t');   break;
                    case 'u':
                        if (!(p = parse_hex4(p, &u))) {
                            STRING_ERROR(PARSE_INVALID_UNICODE_HEX);
                        }
                        if (u >= 0xD800 && u <= 0xDBFF) {
                            if (*p++ != '\\') {
                                STRING_ERROR(PARSE_INVALID_UNICODE_SURROGATE);
                            }
                            if (*p++ != 'u') {
                                STRING_ERROR(PARSE_INVALID_UNICODE_SURROGATE);
                            }
                            if (!(p = parse_hex4(p, &u2))) {
                                STRING_ERROR(PARSE_INVALID_UNICODE_HEX);
                            }
                            if (u2 < 0xDC00 || u2 > 0xDFFF) {
                                STRING_ERROR(PARSE_INVALID_UNICODE_SURROGATE);
                            }
                            u = (((u - 0xD800) << 10) + (u2 -0xDC00) + 0x10000);
                        }
                        encode_utf8(c, u);
                        break;
                    default:
                        STRING_ERROR(PARSE_INVALID_STRING_ESCAPE);
                }
                break;

            default:
                if ((unsigned char)ch < 0x20) {
                    STRING_ERROR(PARSE_INVALID_STRING_CHAR);
                }
                PUTC(c,ch);
        }
    }
}

int parse_array(context* c, json_value* v)
{
    size_t size = 0;
    int ret;
    EXPECT(c, '[');
    parse_whitespace(c);
    if(*c->json == ']') {
        c->json++;
        v->type = ARRAY;
        v->a.size = 0;
        v->a.e = NULL;
        return PARSE_OK;
    }
    while(1)
    {
        json_value e;
        json_init(&e);
        if((ret = parse_value(c, &e)) != PARSE_OK){
            break;
        }
        memcpy(context_push(c, sizeof(json_value)), &e, sizeof(json_value));
        size++;
        parse_whitespace(c);
        if (*c->json == ','){
            c->json++;
            parse_whitespace(c);
        }
        else if (*c->json == ']') {
            c->json++;
            v->type = ARRAY;
            v->a.size = size;
            size *= sizeof(json_value);
            memcpy(v->a.e = (json_value*)malloc(size), context_pop(c, size), size);
            return PARSE_OK;
        }
        else 
        {
            ret = PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            break;
        }
    }

    for(size_t i = 0; i < size; i++){
        json_free((json_value*)context_pop(c, sizeof(json_value)));
    }
    return ret;
}
