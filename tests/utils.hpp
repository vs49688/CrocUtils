#ifndef _UTILS_HPP
#define _UTILS_HPP

#include <memory>
#include <cJSON.h>
#include <vscpplib.hpp>
#include <libcroc.h>

struct crocmap_deleter {
    using pointer = CrocMap *;

    void operator()(pointer p) const noexcept
    { croc_map_free(p); }
};

using crocmap_ptr = std::unique_ptr<CrocMap, crocmap_deleter>;

struct cjson_deleter {
    using pointer = cJSON *;

    void operator()(pointer p) const noexcept
    { cJSON_Delete(p); }
};

using cjson_ptr = std::unique_ptr<cJSON, cjson_deleter>;

template<typename T>
using vsc_ptr = vsc::vsc_ptr<T>;

std::pair<vsc_ptr<char>, size_t> read_file(const char *fname);

cjson_ptr read_json(const char *fname);

std::string json_to_string(const cJSON *a);

void require_json_equal(const cJSON *expected, const cJSON *actual);

#endif /* _UTILS_HPP */