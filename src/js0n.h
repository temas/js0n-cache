#ifndef INCL_JS0N_H
#define INCL_JS0N_H

#include <vector>

namespace js0n {

typedef enum {
    JST_null,
    JST_boolean,
    JST_number,
    JST_string,
    JST_array,
    JST_object
} js_type;

typedef struct {
    js_type type;
    unsigned int depth;
    unsigned int start;
    unsigned int length;
} js_value;

static js_value invalid_js_value = {JST_null, 0, 0, 0};

typedef struct {
    size_t count;
    js_value* values;
    size_t buffer_length;
    const char* buffer;
} cached_object;

typedef std::vector<js_value> result_vector;

typedef struct {
    result_vector results;
    const char* buffer;
} js0n_buffer;

bool parse(const char *js, unsigned int len, result_vector& results);

js_value* find_key(const char* key, cached_object* cache);

}; // namespace js0n

#endif // INCL_JS0N_H
