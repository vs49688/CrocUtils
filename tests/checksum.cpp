#include <libcroc.h>
#include "catch.hpp"

TEST_CASE("checksum", "[checksum]")
{
    uint8_t buf[] = {1, 2, 3, 4};

    REQUIRE(croc_checksum(buf, sizeof(buf)) == 10);
}
