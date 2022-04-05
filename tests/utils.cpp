#include "catch.hpp"
#include "utils.hpp"

std::pair<vsc_ptr<char>, size_t> read_file(const char *fname)
{
    int            r;
    void           *data;
    size_t         size;
    vsc::stdio_ptr f(fopen(fname, "rb"));
    REQUIRE(f != nullptr);

    r = vsc_freadall(&data, &size, f.get());
    REQUIRE(r == 0);

    return std::make_pair(vsc::vsc_ptr<char>((char *)data), size);
}

cjson_ptr read_json(const char *fname)
{
    const auto&&[ptr, size] = read_file(fname);
    cJSON *j = cJSON_ParseWithLength(ptr.get(), size);
    REQUIRE(j != nullptr);
    return cjson_ptr(j);
}
