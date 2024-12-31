#include "interface.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// https://soundcloud.com/gromov/gromov-allright-here-we-go

/*
What is going on here:
* The compiler's runtime provides its default context and imports lib1
* Lib1 mocks the context for lib2 and calls lib2
* Lib2 tries to get lib3 from the context, but it was mocked by lib1 and lib2 gets lib4 instead of lib3
* Lib2 then tries to get lib5, but it never existed
* Lib2 then tries to get lib6, which did not exist initially, but was mocked in by lib1
* Lib2 then tries to get lib7, which did exist initially, but was mocked out by lib1
*/

typedef struct {
    char* libname;
} Lib3Layout;

typedef struct {
    char* libname;
} Lib5Layout;

typedef struct {
    char* libname;
} Lib6Layout;

typedef struct {
    char* libname;
} Lib7Layout;

typedef struct {
    void (*run) (Context);
} Lib2Layout;

typedef struct {
    Context previous;
    Lib3Layout lib3_mock;
    Lib6Layout lib6_mock;
} Lib1ContextData;

GettingResult lib1_context_getter(void* data_void, ResourceName name) {
    Lib1ContextData* data = data_void;
    if (strcmp(name, "lib3") == 0) {
        return (GettingResult){ .is_success = true, .data = &data->lib3_mock };
    }
    if (strcmp(name, "lib6") == 0) {
        return (GettingResult){ .is_success = true, .data = &data->lib6_mock };
    }
    if (strcmp(name, "lib7") == 0) {
        return (GettingResult){ .is_success = false };
    }
    return get(&data->previous, name);
}

void lib1(Context ctx) {
    Lib1ContextData *lib1_context_data = malloc(sizeof(*lib1_context_data));
    lib1_context_data->previous = ctx;
    lib1_context_data->lib3_mock.libname = "lib4";
    lib1_context_data->lib6_mock.libname = "lib6";
    Context lib1_context = {
        .getter = lib1_context_getter,
        .getter_data = lib1_context_data,
    };
    GettingResult lib2_result = get(&ctx, "lib2");
    if (lib2_result.is_success) {
        Lib2Layout* layout = lib2_result.data;
        layout->run(lib1_context);
    } else {
        fprintf(stderr, "Failure to get lib2\n");
    }
}

void lib2(Context ctx) {
    GettingResult lib3_result = get(&ctx, "lib3");
    if (lib3_result.is_success) {
        Lib3Layout* layout = lib3_result.data;
        printf("Lib3Layout: %s\n", layout->libname);
    } else {
        fprintf(stderr, "Failure to get lib3\n");
    }
    GettingResult lib5_result = get(&ctx, "lib5");
    if (lib5_result.is_success) {
        Lib5Layout* layout = lib5_result.data;
        printf("Lib5Layout: %s\n", layout->libname);
    } else {
        fprintf(stderr, "Failure to get lib5\n");
    }
    GettingResult lib6_result = get(&ctx, "lib6");
    if (lib6_result.is_success) {
        Lib6Layout* layout = lib6_result.data;
        printf("Lib6Layout: %s\n", layout->libname);
    } else {
        fprintf(stderr, "Failure to get lib6\n");
    }
    GettingResult lib7_result = get(&ctx, "lib7");
    if (lib7_result.is_success) {
        Lib7Layout* layout = lib7_result.data;
        printf("Lib7Layout: %s\n", layout->libname);
    } else {
        fprintf(stderr, "Failure to get lib7\n");
    }
}

typedef struct {
    Lib2Layout lib2;
    Lib3Layout lib3;
    Lib7Layout lib7;
} InitialContextData;

GettingResult initial_context_getter(void* data_void, ResourceName name) {
    InitialContextData* data = data_void;
    if (strcmp(name, "lib2") == 0) {
        return (GettingResult){ .is_success = true, .data = &data->lib2 };
    }
    if (strcmp(name, "lib3") == 0) {
        return (GettingResult){ .is_success = true, .data = &data->lib3 };
    }
    if (strcmp(name, "lib7") == 0) {
        return (GettingResult){ .is_success = true, .data = &data->lib7 };
    }
    return (GettingResult){ .is_success = false };
}

int main(void) {
    InitialContextData *initial_context_data = malloc(sizeof(*initial_context_data));
    initial_context_data->lib2.run = lib2;
    initial_context_data->lib3.libname = "lib3";
    initial_context_data->lib7.libname = "lib7";
    Context initial_context = {
        .getter = initial_context_getter,
        .getter_data = initial_context_data,
    };
    lib1(initial_context);
    return 0;
}
