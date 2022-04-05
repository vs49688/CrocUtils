#include <libcroc.h>
#include <vscpplib.hpp>
#include "catch.hpp"

TEST_CASE("asdf", "[colour]")
{
    uint8_t tmp[5];
    int r;
    vsc::stdio_ptr f(fmemopen(tmp, sizeof(tmp), "wb"));
    REQUIRE(f != nullptr);

    CrocColour col = {0xAA, 0xBB, 0xCC, 0xEE};
    r = croc_colour_fwrite(f.get(), &col);
    REQUIRE(r == 0);
    f.reset();

    CHECK(tmp[0] == 0xAA);
    CHECK(tmp[1] == 0xBB);
    CHECK(tmp[2] == 0xCC);
    CHECK(tmp[3] == 0xEE);
}