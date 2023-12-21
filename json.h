#ifndef JSON_H
#define JSON_H

#include <cassert>


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
}json_value;

enum{
    PARSE_OK = 0,
    PARSE_EXPCET_VALUE, // 估计值
    PARSE_INVALID_VALUE, // 无效值
    PARSE_ROOT_NOT_SINGULAR 
};

typedef struct {
    const char* json;
}context;

class Json
{
private:

public:
    json_type get_value(const json_value *value);
    int parse(json_value *v, const char *json);
};


#endif //JSON_H
