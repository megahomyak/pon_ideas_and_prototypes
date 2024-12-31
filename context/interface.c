#include <stdbool.h>

typedef struct {
    void* data; /* Semantically not necessarily a pointer, should just be "any data" (transferred in any way) */
    bool is_success;
} GettingResult;

typedef void* GetterContext; /* Semantically not necessarily a pointer */
typedef char* ResourceName; /* Should not ideally be a string, but an unsigned bigint, but for the sake of demonstration this right here will be a string */

typedef struct {
    GettingResult (*getter) (GetterContext, ResourceName);
    GetterContext getter_data;
} Context;

GettingResult get(Context* context, ResourceName name) {
    return context->getter(context->getter_data, name);
}
