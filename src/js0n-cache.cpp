#include <v8.h>
#include <node.h>

#include "js0n.h"

using namespace v8;
using namespace node;

namespace js0n { namespace cache {

static Persistent<ObjectTemplate> object_template_;

class object : public ObjectWrap
{
public:
    object(cached_object obj) : cache_(obj)
    {
    }

    ~object()
    {
        //printf("Bye bye object!\n");
    }

    static Handle<Value> WrapResults(cached_object cache)
    {
        HandleScope handle_scope;


        if (object_template_.IsEmpty()) {
            Handle<ObjectTemplate> raw_template = PrepareTemplate();
            object_template_ = Persistent<ObjectTemplate>::New(raw_template);
        }

        Handle<Object> new_object = object_template_->NewInstance();
        //new_object->SetInternalField(1, External::New(results));

        //printf("Here: %x\n", buffer);

        object* obj = new object(cache);
        obj->Wrap(new_object);
        // TODO: Lookup the args id and wrap it

        //printf("Wrapped and returning\n");

        return handle_scope.Close(new_object);
    }

private:
    cached_object cache_;

private:
    static Handle<Value> ObjectGet(Local<String> name, const AccessorInfo& info)
    {
        object* obj = ObjectWrap::Unwrap<object>(info.Holder());

        // Fetch the map wrapped by this object.
        /*
        Handle<External> field = Handle<External>::Cast(info.Holder()->GetInternalField(1));
        void* ptr = field->Value();
        js0n::result_vector* members = reinterpret_cast<js0n::result_vector*>(ptr);
        */
        //printf("Going to call find_key\n");
        js0n::js_value* value = js0n::find_key(*(String::Utf8Value(name)), &obj->cache_);
        //printf("Found type(%u) length(%u) start(%u) depth(%u)\n", value->type, value->length, value->start, value->depth);

        switch (value->type) {
        case JST_string:
            return String::New(obj->cache_.buffer + value->start, value->length);
        case JST_number:
            char tmpBuf[value->length];
            strncpy(tmpBuf, obj->cache_.buffer + value->start, value->length);
            // TODO: determine the if this is a float or an integer and return correctly
            return Integer::New(atoi(tmpBuf));
        case JST_null:
            return Null();
        case JST_boolean:
            // This was validated by the parser already so just check the first letter
            return Boolean::New((obj->cache_.buffer[value->start] == 't') ? true : false);
        case JST_array:
            // TODO: make it return a warpped array
            return Undefined();
        case JST_object:
            {
                // TODO: return a new wrapped object
                Local<Object> new_object = object_template_->NewInstance();
                //new_object->SetInternalField(1, External::New(results));

                //printf("Here: %x\n", buffer);
                cached_object subCache;
                // -1 +sizeof(js_value) to shift into the object actually
                subCache.count = obj->cache_.count - ((value - obj->cache_.values) / sizeof(js_value)) - 1;
                subCache.values = &value[1];
                subCache.buffer_length = obj->cache_.buffer_length;
                subCache.buffer = obj->cache_.buffer;

                object* obj = new object(subCache);
                obj->Wrap(new_object);
                return new_object;
            }
        default:
            return Undefined();
        }
        //js0n::js_value member = js0n::find_key(*members, *String::Utf8Value(name)); 
        //printf("Out of find_key\n");
        //printf("member type(%u) length(%u) start(%u) depth(%u)\n", member.type, member.length, member.start, member.depth);

      /* // Convert the JavaScript string to a std::string. */
      /* string key = ObjectToString(name); */

      /* // Look up the value if it exists using the standard STL ideom. */
      /* map<string, string>::iterator iter = obj->find(key); */

      /* // If the key is not present return an empty handle as signal */
      /* if (iter == obj->end()) return Handle<Value>(); */

      /* // Otherwise fetch the value and wrap it in a JavaScript string */
      /* const string& value = (*iter).second; */
    }

    static Handle<Value> ObjectSet(Local<String> name, Local<Value> value_obj, const AccessorInfo& info)
    {
        // TODO:  Is this the best thing to return?
        return Undefined();
    }

    static Handle<ObjectTemplate> PrepareTemplate()
    {
        HandleScope handle_scope;

        Handle<ObjectTemplate> result = ObjectTemplate::New();
        result->SetInternalFieldCount(2);
        result->SetNamedPropertyHandler(ObjectGet, ObjectSet);

        return handle_scope.Close(result);
    }
};




static Handle<Value> ParseObject(const Arguments& args)
{
    HandleScope handle_scope;
    //printf("Args: %d\n", args.Length());
    if (args.Length() < 1 || !(args[0]->IsString())) {
        return ThrowException(Exception::TypeError(String::New("The first argument to parse must be a string.")));
    }
    Local<String> raw = Local<String>::Cast(args[0]);
    String::Utf8Value json(raw);
    js0n::result_vector* results = new js0n::result_vector;
    char* buffer = (char*)malloc(json.length());
    memcpy(buffer, *json, json.length());
    //printf("Buffer: (%d) %.*s\n", json.length(), json.length(), buffer);
    if (!js0n::parse(buffer, json.length(), *results)) {
        // TODO:  Throw an exception
        delete results;
        return handle_scope.Close(Undefined());
    }

    cached_object cache;
    // Here we shift into the object
    cache.count = results->size() - 1;
    cache.values = &((*results)[1]);
    cache.buffer_length = json.length();
    cache.buffer = buffer;
    return handle_scope.Close(object::WrapResults(cache));
}

}; }; // namespace js0n::cache

extern "C" void init(Handle<Object> target)
{
    HandleScope scope;
    NODE_SET_METHOD(target, "parse", js0n::cache::ParseObject);
}
