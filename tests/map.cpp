#include <vscpplib.hpp>
#include <libcroc.h>
#include "catch.hpp"
#include "utils.hpp"

TEST_CASE("map read", "[map]")
{
    int            r;
    vsc::stdio_ptr f;
    crocmap_ptr    mptr;
    cjson_ptr      expected, actual;

    CrocMap map;
    croc_map_init(&map);
    mptr.reset(&map);

    const char *fname = "testdata/MP010_00.MAP";

    f.reset(fopen(fname, "rb"));
    REQUIRE(f != nullptr);

    r = croc_map_read(f.get(), &map);
    REQUIRE(r == 0);

    croc_extract_level_info(fname, &map._level, &map._sublevel);

    actual.reset(croc_map_write_json(&map));
    REQUIRE(actual != nullptr);
    expected = read_json("testdata/MP010_00.json");

    REQUIRE(cJSON_Compare(expected.get(), actual.get(), cJSON_True));
}

TEST_CASE("map write", "[map]")
{
    int            r;
    vsc::stdio_ptr f;
    crocmap_ptr    mptr;
    cjson_ptr      input;
    CrocMap        *pmap;
    uint8_t        buf[2452];

    CrocMap map;
    croc_map_init(&map);
    mptr.reset(&map);

    input = read_json("testdata/MP010_00.json");

    pmap = croc_map_read_json(input.get(), &map);
    REQUIRE(pmap == &map);

    f.reset(fmemopen(buf, sizeof(buf), "wb"));
    REQUIRE(f != nullptr);

    /*
     * NB: This will write it in v21, so the size will be
     * different to the input.
     */
    r = croc_map_write(f.get(), &map);
    REQUIRE(r == 0);

    fflush(f.get());
    size_t fsize = vsc_ftello(f.get());
    f.reset();

    REQUIRE(fsize == 2451);
    REQUIRE(croc_checksum(buf, 2451) == 51998);
}

