#include <libcroc.h>
#include <vector>
#include <cstring>
#include <vscpplib.hpp>
#include "catch.hpp"

bool operator==(const CrocDirEntry& a, const CrocDirEntry& b) noexcept
{
    return memcmp(a.name, b.name, sizeof(a.name)) == 0 &&
           a.size == b.size &&
           a.restart == b.restart &&
           a.offset == b.offset &&
           a.usage == b.usage;
}

TEST_CASE("directory read/write", "[directory]")
{
    uint8_t buf[1024];

    std::vector<CrocDirEntry> directory{
        {
            CrocDirEntry{"BIGFILE.BIN", 0xFEFEFE, 0xAA, 0, 42154},
            CrocDirEntry{"EMPTY.TXT", 0, 0, 0xFEFEFE, 0},
        }};

    vsc::stdio_ptr f(fmemopen(buf, sizeof(buf), "wb"));
    REQUIRE(f != nullptr);

    int r = croc_dir_write(f.get(), directory.data(), directory.size(), 0);
    REQUIRE(r == 0);
    size_t fsize = (size_t)vsc_ftello(f.get());
    f.reset();

    f.reset(fmemopen(buf, fsize, "rb"));
    REQUIRE(f != nullptr);

    CrocDirEntry               *entries;
    size_t                     nentries;
    int                        old;
    entries = croc_dir_read(f.get(), &nentries, &old);
    REQUIRE(entries != nullptr);
    REQUIRE(f != nullptr);
    vsc::vsc_ptr<CrocDirEntry> _entries(entries);

    CHECK(nentries == 2);
    CHECK(old == 0);
    CHECK(entries[0] == directory[0]);
    CHECK(entries[1] == directory[1]);
}
