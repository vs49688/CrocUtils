#include <brender.h>
#include <vsclib.h>
#include <libcroc/wad.h>

typedef struct wadfs_handle {
    uint8_t            *data;
    const CrocWadEntry *entry;
    br_size_t           pos;
} wadfs_handle;

static CrocWadFs *current_wad = NULL;

CrocWadFs *br_wadfs_set_current(CrocWadFs *w)
{
    CrocWadFs *old = current_wad;
    current_wad    = w;
    return old;
}

static br_uint_32 wadfs_attributes(void)
{
    return BR_FS_ATTR_READABLE | BR_FS_ATTR_HAS_BINARY | BR_FS_ATTR_HAS_ADVANCE;
}

static void *wadfs_load_internal(void *res, const char *name, const CrocWadEntry **const _entry)
{
    const CrocWadEntry *entry;
    void               *tmpbuf, *buf;

    if(current_wad == NULL)
        return NULL;

    if(croc_wadfs_load(current_wad, name, &tmpbuf, &entry) < 0)
        return NULL;

    if((buf = BrResAllocate(res, entry->uncompressed_size, BR_MEMORY_APPLICATION)) == NULL)
        return NULL;

    BrMemCpy(buf, tmpbuf, entry->uncompressed_size);

    vsc_free(tmpbuf);

    *_entry = entry;
    return buf;
}

static void *wadfs_open_read(const char *name, br_size_t n_magics, br_mode_test_cbfn *mode_test, int *mode_result)
{
    wadfs_handle       *h;
    const CrocWadEntry *entry;
    void               *buf;

    if((h = BrResAllocate(NULL, sizeof(wadfs_handle), BR_MEMORY_APPLICATION)) == NULL)
        return NULL;

    if((buf = wadfs_load_internal(h, name, &entry)) == NULL) {
        BrResFree(h);
        return NULL;
    }

    *h = (wadfs_handle){
        .data  = buf,
        .entry = entry,
        .pos   = 0,
    };

    if(mode_test && mode_result)
        *mode_result = mode_test(buf, n_magics);

    return h;
}

static void *wadfs_open_write(const char *name, int text)
{
    (void)name;
    (void)text;
    return NULL;
}

static void wadfs_close(void *f)
{
    BrResFree((wadfs_handle *)f);
}

static int wadfs_eof(void *f)
{
    wadfs_handle *h = f;
    return h->pos >= h->entry->uncompressed_size ? ~0 : 0;
}

static int wadfs_getchr(void *f)
{
    wadfs_handle *h = f;

    if(h->pos >= h->entry->uncompressed_size)
        return BR_EOF;

    return (int8_t)h->data[h->pos++];
}

static void wadfs_putchr(int c, void *f)
{
    (void)c;
    (void)f;
}

static br_size_t wadfs_read(void *buf, br_size_t size, br_size_t nelems, void *f)
{
    wadfs_handle *h = f;
    br_size_t     bytes_left, elems_left;

    if(size == 0 || nelems == 0)
        return 0;

    bytes_left = h->entry->uncompressed_size - h->pos;
    elems_left = bytes_left / size;
    if(elems_left < nelems)
        nelems = elems_left;

    if(nelems == 0)
        return 0;

    size = size * nelems;

    BrMemCpy(buf, h->data + h->pos, size);
    h->pos += size;

    return nelems;
}

static br_size_t wadfs_write(const void *buf, br_size_t size, br_size_t nelems, void *f)
{
    (void)buf;
    (void)size;
    (void)nelems;
    (void)f;
    return 0;
}

static br_size_t wadfs_getline(char *buf, br_size_t buf_len, void *f)
{
    (void)buf;
    (void)buf_len;
    (void)f;
    return 0;
}

static void wadfs_putline(const char *buf, void *f)
{
    (void)buf;
    (void)f;
}

static void wadfs_advance(br_size_t count, void *f)
{
    wadfs_handle *h = f;

    if(count > (h->pos - h->entry->uncompressed_size))
        h->pos = h->entry->uncompressed_size;
    else
        h->pos += count;
}

static void *wadfs_load(void *res, const char *name, br_size_t *size)
{
    const CrocWadEntry *entry;
    void               *buf;

    if((buf = wadfs_load_internal(res, name, &entry)) == NULL)
        return NULL;

    *size = entry->uncompressed_size;
    return buf;
}

br_filesystem br_wadfs = {
    .identifier = "wad filesystem",
    .attributes = wadfs_attributes,
    .open_read  = wadfs_open_read,
    .open_write = wadfs_open_write,
    .close      = wadfs_close,
    .eof        = wadfs_eof,
    .getchr     = wadfs_getchr,
    .putchr     = wadfs_putchr,
    .read       = wadfs_read,
    .write      = wadfs_write,
    .getline    = wadfs_getline,
    .putline    = wadfs_putline,
    .advance    = wadfs_advance,
    .load       = wadfs_load,
};
