/* algocli-defs.h — declaration of the master command table.
 *
 * The actual command tree (commands[] and its subarrays) lives in
 * algocli-defs.c. Defining the table in a header was the original
 * code's biggest footgun: any TU that included this header got a
 * private definition. We dropped that pattern in Phase 5; only the
 * extern declaration remains here. */

#ifndef ALGOCLI_DEFS_H
#define ALGOCLI_DEFS_H

#include "algocli.h"

extern command_t commands[];

#endif /* ALGOCLI_DEFS_H */
