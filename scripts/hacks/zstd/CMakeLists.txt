cmake_minimum_required(VERSION 3.11)

project(zstd)

option(ZSTD_LIB    "Use Static Libaray" ON)

set (ZSTD_SRC
lib/compress/zstd_compress_superblock.c
lib/compress/zstdmt_compress.c
lib/compress/zstd_double_fast.c
lib/compress/zstd_fast.c
lib/compress/zstd_compress_sequences.c
lib/compress/zstd_ldm.c
lib/compress/hist.c
lib/compress/zstd_compress.c
lib/compress/zstd_lazy.c
lib/compress/zstd_compress_literals.c
lib/compress/huf_compress.c
lib/compress/zstd_opt.c
lib/compress/fse_compress.c
lib/dictBuilder/cover.c
lib/dictBuilder/divsufsort.c
lib/dictBuilder/fastcover.c
lib/dictBuilder/zdict.c
lib/decompress/zstd_ddict.c
lib/decompress/huf_decompress.c
lib/decompress/zstd_decompress.c
lib/decompress/zstd_decompress_block.c
lib/legacy/zstd_v05.c
lib/legacy/zstd_v01.c
lib/legacy/zstd_v06.c
lib/legacy/zstd_v02.c
lib/legacy/zstd_v07.c
lib/legacy/zstd_v03.c
lib/legacy/zstd_v04.c
lib/common/entropy_common.c
lib/common/fse_decompress.c
lib/common/debug.c
lib/common/xxhash.c
lib/common/pool.c
lib/common/threading.c
lib/common/zstd_common.c
lib/common/error_private.c
)

include_directories(lib)

add_definitions(-DNDEBUG)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if (${ZSTD_LIB})
    add_library(zstd STATIC ${ZSTD_SRC})
else()
    add_library(zstd SHARED ${ZSTD_SRC})
endif()

IF(MSVC)
  target_compile_options(zstd PRIVATE /W3)
endif()

