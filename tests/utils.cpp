#include <iostream>
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

std::string json_to_string(const cJSON *a)
{
    char *s = cJSON_Print(a);
    if(s == nullptr)
        throw std::bad_alloc();

    try {
        return std::string(s);
    } catch(...) {
        cJSON_free(s);
        throw;
    }
}

void require_json_equal(const cJSON *expected, const cJSON *actual)
{
    cJSON_bool equal = cJSON_Compare(expected, actual, cJSON_True);
    if(equal)
        return;

    std::string exps = json_to_string(expected);
    std::string acts = json_to_string(actual);

    std::cout << "Expected:" << std::endl << exps << std::endl;
    std::cout << "Actual:  " << std::endl << acts << std::endl;
    throw std::exception();
}
