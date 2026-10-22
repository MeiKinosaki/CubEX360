#ifndef CUBEX360_COVERS_H
#define CUBEX360_COVERS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*cubex_cover_download_fn)(const char *url,
                                      const char *destination,
                                      void *user_data);

/* Fetches a cover only when the destination does not already exist.
 * game_id is restricted to ASCII letters/digits to prevent path/URL injection.
 * base_url must be an HTTP(S) URL supplied by the user/operator.
 * The platform callback must implement network transport (Wi-Fi/Ethernet via
 * the Xbox 360 network stack) and write a complete image to destination.
 * Returns 0 when already cached or downloaded, negative on failure. */
int cubex_cover_ensure(const char *base_url,
                       const char *cover_directory,
                       const char *game_id,
                       cubex_cover_download_fn download_fn,
                       void *user_data);

#ifdef __cplusplus
}
#endif
#endif
