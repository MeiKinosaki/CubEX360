#include "cubex360/covers.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define CUBEX_COVER_PATH_MAX 1024u
#define CUBEX_COVER_URL_MAX 2048u

static int valid_base_url(const char *url) {
    if (!url) return 0;
    return strncmp(url, "https://", 8) == 0 || strncmp(url, "http://", 7) == 0;
}

static int valid_game_id(const char *id) {
    size_t i, n;
    if (!id) return 0;
    n = strlen(id);
    if (n < 4u || n > 16u) return 0;
    for (i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)id[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) return 0;
    }
    return 1;
}

int cubex_cover_ensure(const char *base_url, const char *cover_directory,
                       const char *game_id, cubex_cover_download_fn download_fn,
                       void *user_data) {
    char url[CUBEX_COVER_URL_MAX], path[CUBEX_COVER_PATH_MAX];
    size_t base_len, dir_len;
    FILE *f;
    int n;

    if (!valid_base_url(base_url) || !cover_directory || !*cover_directory ||
        !valid_game_id(game_id) || !download_fn) return -1;

    base_len = strlen(base_url);
    dir_len = strlen(cover_directory);
    if (base_len > 1500u || dir_len > 900u) return -1;

    n = snprintf(url, sizeof(url), "%s%s%s.png",
                 base_url, base_len && base_url[base_len - 1u] == '/' ? "" : "/",
                 game_id);
    if (n < 0 || (size_t)n >= sizeof(url)) return -1;
    n = snprintf(path, sizeof(path), "%s%s%s.png",
                 cover_directory, dir_len && cover_directory[dir_len - 1u] == '/' ? "" : "/",
                 game_id);
    if (n < 0 || (size_t)n >= sizeof(path)) return -1;

    f = fopen(path, "rb");
    if (f) {
        fclose(f);
        return 0; /* cached: do not consume network bandwidth */
    }
    return download_fn(url, path, user_data) == 0 ? 1 : -2;
}
