/* Common utility functions. */
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "errhandle.h"
#include "oleg.h"
#include "utils.h"
#include "logging.h"

ol_bucket *_ol_get_last_bucket_in_slot(ol_bucket *bucket) {
    ol_bucket *tmp_bucket = bucket;
    int depth = 0;
    while (tmp_bucket->next != NULL) {
        tmp_bucket = tmp_bucket->next;
        depth++;
        if (depth > 1000)
            ol_log_msg(LOG_WARN, "Depth of bucket stack is crazy, help! It's at %i", depth);
    }
    return tmp_bucket;
}

void _ol_free_bucket(ol_bucket **ptr) {
    free((*ptr)->expiration);
    free((*ptr)->content_type);
    free((*ptr)->data_ptr);
    free((*ptr));
    *ptr = NULL;
}

int _ol_calc_idx(const size_t ht_size, const uint32_t hash) {
    int index;
    /* Powers of two, baby! */
    index = hash & (ol_ht_bucket_max(ht_size) - 1);
    return index;
}

int _ol_get_stat(const char *filepath, struct stat *sb) {
    int fd;
    fd = open(filepath, O_RDONLY);
    if (fd == -1)
        return 0;

    if (fstat(fd, sb) == -1)
        return 0;
    close(fd);
    return 1;
}

int _ol_get_file_size(const char *filepath) {
    struct stat sb = {0};
    int ret = _ol_get_stat(filepath, &sb);
    if (ret) /* Maybe the file doesn't exist. */
        return sb.st_size;
    return -1;
}

void *_ol_mmap(size_t to_mmap, int fd) {
    /* TODO: Investigate usage of madvise here. */
    void *to_return = NULL;

    to_return = mmap(NULL, to_mmap, PROT_READ | PROT_WRITE, MAP_SHARED,
                          fd, 0);
    check(to_return != MAP_FAILED, "Could not mmap hashes file.");
    return to_return;

error:
    return NULL;
}
