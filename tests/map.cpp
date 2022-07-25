#include <vscpplib.hpp>
#include <libcroc.h>
#include "catch.hpp"
#include "utils.hpp"
#include <iostream>

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

    require_json_equal(expected.get(), actual.get());
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

TEST_CASE("extract level info", "[map]")
{
    uint16_t level, sublevel;
    int r;

    r = croc_extract_level_info("MP010_00.MAP", &level, &sublevel);
    CHECK(r == 0);
    CHECK(level == 10);
    CHECK(sublevel == 00);

    r = croc_extract_level_info("MP999_99.MAP", &level, &sublevel);
    CHECK(r == 0);
    CHECK(level == 999);
    CHECK(sublevel == 99);

    r = croc_extract_level_info("asdfasd", &level, &sublevel);
    CHECK(r < 0);

    r = croc_extract_level_info("/path/to/MP123_45.MAP", &level, &sublevel);
    CHECK(r == 0);
    CHECK(level == 123);
    CHECK(sublevel == 45);

    r = croc_extract_level_info("/path/to/MPAAA_00.MAP", &level, &sublevel);
    CHECK(r < 0);
}