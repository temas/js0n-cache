#include "js0n.h"

namespace js0n {

void reset_value(js_value& value)
{
    value.type = JST_null;
    value.depth = 0;
    value.start = 0;
    value.length = 0;
}

static void fillmem(void* dest[255], void* address, int start, int end)
{
    for (int i = start; i <= end; ++i) {
        dest[i] = address;
    }
}

static void* gostruct[255];
static void* gobare[255];
static void* gostring[255];
static void* goutf8_continue[255];
static void* goesc[255];

// Originally by jeremie miller - 2010
// public domain;
// contributions/improvements welcome via github


bool parse(const char *js, unsigned int len, result_vector& results)
{
	const char *cur, *end;
	int depth=0;
	int utf8_remain=0;
    js_value cur_value;

    static bool init_done = false;
    
    if (!init_done) {
        // Setup gostruct
        fillmem(gostruct, &&l_bad, 0, 255);
        gostruct['\t'] = &&l_loop;
        gostruct[' '] = &&l_loop;
        gostruct['\r'] = &&l_loop;
        gostruct['\n'] = &&l_loop;
        gostruct['"'] = &&l_qup;
        gostruct[':'] = &&l_loop;
        gostruct[','] = &&l_loop;
        gostruct['['] = &&l_arr_up; 
        gostruct[']'] = &&l_arr_down; // tracking [] and {} individually would allow fuller validation but is really messy
        gostruct['{'] = &&l_obj_up; 
        gostruct['}'] = &&l_obj_down;
        gostruct['-'] = &&l_bare;
        //gostruct[48 ... 57] = &&l_bare, // 0-9
        fillmem(gostruct, &&l_bare, 48, 57);
        gostruct['t'] = &&l_bare;
        gostruct['f'] = &&l_bare;
        gostruct['n'] = &&l_bare; // true, false, null

        // gobare
        fillmem(gobare, &&l_bad, 0, 255);
        //[32 ... 126] = &&l_loop, // could be more pedantic/validation-checking
        fillmem(gobare, &&l_loop, 32, 126);
        gobare['\t'] = &&l_unbare;
        gobare[' '] = &&l_unbare;
        gobare['\r'] = &&l_unbare;
        gobare['\n'] = &&l_unbare;
        gobare[','] = &&l_unbare;
        gobare[']'] = &&l_unbare;
        gobare['}'] = &&l_unbare;

        // gostring
        fillmem(gostring, &&l_bad, 0, 255);
        // gostring[32 ... 126] = &&l_loop;
        fillmem(gostring, &&l_loop, 32, 126);
        gostring['\\'] = &&l_esc;
        gostring['"'] = &&l_qdown;
        //gostring[192 ... 223] = &&l_utf8_2;
        fillmem(gostring, &&l_utf8_2, 192, 223);
        //[224 ... 239] = &&l_utf8_3,
        fillmem(gostring, &&l_utf8_3, 224, 239);
        //[240 ... 247] = &&l_utf8_4,
        fillmem(gostring, &&l_utf8_4, 240, 247);


        fillmem(goutf8_continue, &&l_bad, 0, 255);
        //[128 ... 191] = &&l_utf_continue,
        fillmem(goutf8_continue, &&l_utf_continue, 128, 191);

        // go escape it!
        fillmem(goesc, &&l_bad, 0, 255);
        goesc['"'] = &&l_unesc;
        goesc['\\'] = &&l_unesc;
        goesc['/'] = &&l_unesc;
        goesc['b'] = &&l_unesc;
        goesc['f'] = &&l_unesc;
        goesc['n'] = &&l_unesc;
        goesc['r'] = &&l_unesc;
        goesc['t'] = &&l_unesc;
        goesc['u'] = &&l_unesc;

        init_done = true;
    }

	static void **go = gostruct;
	
    //full_obj = malloc(sizeof(struct js_value));
    //cur_value = full_obj;
	for(cur=js,end=js+len; cur<end; cur++)
	{
        goto *go[*cur];
        l_loop:;
	}
	
    /* for (int i = 0; i < results.size(); ++i) { */
    /*     std::cout << "[type:" << results[i].type << ", depth:" << results[i].depth */
    /*         << ", start:" << results[i].start << ", length:" << results[i].length */
    /*         << "]" << std::endl; */
    /* } */

	return (depth == 0); // 0 if successful full parse, >0 for incomplete data
	
	l_bad:
		return 1;
	
	l_arr_up:
        //printf("arr up at %d\n", ((cur) - js));
        cur_value.type = JST_array;
        cur_value.start = cur - js;
        cur_value.depth = depth;
        cur_value.length = 0;
        results.push_back(cur_value);
        //PUSH(0);
        ++depth;
        goto l_loop;
        
    l_arr_down:
        //printf("arr down at %d\n", (cur) - (js + *(out-1)) + 1);
        --depth;
        //CAP(0);
        goto l_loop;
        
	l_obj_up:
        cur_value.type = JST_object;
        cur_value.start = cur - js;
        cur_value.depth = depth;
        cur_value.length = 0;
        results.push_back(cur_value);
	    //printf("obj up at %d\n", ((cur) - js));
		//PUSH(0);
		++depth;
		goto l_loop;

	l_obj_down:
	    //printf("obj down at %d\n", (cur) - (js + *(out-1)) + 1);
		--depth;
		//CAP(0);
		goto l_loop;

	l_qup:
        //printf("qup\n");
        cur_value.type = JST_string;
        cur_value.start = (cur - js) + 1;
        cur_value.depth = depth;
		//PUSH(1);
		go=gostring;
		goto l_loop;

	l_qdown:
        //printf("qdown\n");
        cur_value.length = (cur - js) - cur_value.start;
        results.push_back(cur_value);
		//CAP(-1);
		go=gostruct;
		goto l_loop;
		
	l_esc:
        //printf("esc\n");
		go = goesc;
		goto l_loop;
		
	l_unesc:
        //printf("unesc\n");
		go = gostring;
		goto l_loop;

	l_bare:
        //printf("bare\n");
        switch(*cur) {
            case 'n':
                cur_value.type = JST_null;
                break;
            case 't':
            case 'f':
                cur_value.type = JST_boolean;
                break;
            default:
                cur_value.type = JST_number;
        };
        cur_value.start = (cur - js);
        cur_value.depth = depth;
		//PUSH(0);
		go = gobare;
		goto l_loop;

	l_unbare:
        //printf("unbare\n");
        cur_value.length = ((cur - js) - cur_value.start);
        results.push_back(cur_value);
		//CAP(-1);
		go = gostruct;
		goto *go[*cur];

	l_utf8_2:
		go = goutf8_continue;
		utf8_remain = 1;
		goto l_loop;

	l_utf8_3:
		go = goutf8_continue;
		utf8_remain = 2;
		goto l_loop;

	l_utf8_4:
		go = goutf8_continue;
		utf8_remain = 3;
		goto l_loop;

	l_utf_continue:
		if (!--utf8_remain)
			go=gostring;
		goto l_loop;

}

/// cache->values[0] must be at the first child of the correct depth to iterate
js_value* find_key(const char* key, cached_object* cache)
{
    //printf("Finding a key: %s\n", key);
    int keylen = strlen(key);

    // TODO:  Needs to watch depth!
    // We assume that values[0] is already set to the correct depth here
    int curDepth = cache->values[0].depth;
    for (int i = 0; i < cache->count; i += 2) {
        const js_value& cur(cache->values[i]);
        //printf("i(%d) d(%d) Checking: %.*s\n", i, cur.depth, cur.length, cache->buffer + cur.start);
        if (cur.type == JST_string && cur.length == keylen && memcmp(key, cache->buffer + cur.start, keylen) == 0) {
            return &cache->values[i+1];
        }
        const js_value* next = &cache->values[i + 1];
        // On an object or array iterate by 1 until we our out of that depth
        if (next->type == JST_array || next->type == JST_object) {
            //printf("Skipping an array or object starting at %d\n", i);
            do {
                next = &cache->values[++i + 1];
                //printf("Skipping curDepth(%d) i(%d) t(%d) d(%d): %.*s\n", curDepth, i, next->type, next->depth, next->length, cache->buffer + next->start);
            } while (next->depth > curDepth && i < cache->count);
            //printf("Back at depth at i(%d) d(%d): %.*s\n", i, next->depth, next->length, cache->buffer + next->start);
            // We have to go back one to be at what should be current because we advance one at the next iteration
            --i;
        }
    }
    
    return &invalid_js_value;
}

}; // namespace js0n
