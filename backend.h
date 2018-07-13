#ifndef  BACKEND_H
#define  BACKEND_H

#include <stdbool.h>

struct backend {
    /* Called to handle arguments specific to the backend. */
    bool (*parse_arg)(const char *name, const char *val);
    /* Called after argument parsing to verify arguments. */
    bool (*check_args)(void);
    /* Called at startup when not resuming to load the initial data. */
    bool (*init)(void);
    /* Called to save state when exiting. */
    bool (*save)(void);
    /* Called to resume from previously saved state. */
    bool (*resume)(void);
    /* Called when set_variable updates an NV variable. */
    bool (*set_variable)(void);
};

extern struct backend *db;
extern struct backend xapidb;

#endif
