# Bare Metal Engine Third-party Libraries
Scripts for building third-party dependencies for engine projects

# Status

| Library           | Status |
| :---------------- | :----: |
| Zlib              | [![zlib](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_zlib.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_zlib.yml) |
| LZ4               | [![lz4](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_lz4.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_lz4.yml) |
| FreeType          | [![freetype](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_freetype.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_freetype.yml) |
| FreeImage         | [![freeimage](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_freeimage.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_freeimage.yml) |
| Squish            | [![squish](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_squish.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_squish.yml) |
| DXC               | [![dxc](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_dxc.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_dxc.yml)
| MbedTls           | [![mbedtls](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_mbedtls.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_mbedtls.yml)
| OpenFBX           | [![ofbx](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_ofbx.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_ofbx.yml) |
| LUA               | [![lua](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_lua.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_lua.yml) |
| Google Test       | [![gtest](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_gtest.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_gtest.yml) |
| ImGUI             | [![imgui](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_imgui.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_imgui.yml) |
| Embree            | [![embree](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_embree.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_embree.yml) |
| Bullet            | [![bullet](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_bullet.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_bullet.yml) |
| PhysX             | [![physx](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_physx.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_physx.yml) |
| OpenAL            | [![openal](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_openal.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_openal.yml) |
| LLVM              | [![llvm](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_llvm.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_llvm.yml) |
| Recast            | [![recast](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_recast.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_recast.yml) |
| CURL              | [![curl](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_curl.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/build_single_culr.yml) |

# Additional status

[![Nightly integration tests](https://github.com/BareMetalEngine/thirdparty/actions/workflows/nightly_test.yml/badge.svg?branch=main)](https://github.com/BareMetalEngine/thirdparty/actions/workflows/nightly_test.yml)

# How To Build
Use the onion tool tool to build individual libraries:

```
onion library -library=zlib.onion
```

Use the -upload option to upload data to AWS S3:

```
onion library -library=zlib.onion -upload -awsKey=<key> -awsSecret=<secret>
```