/***
* Inferno Engine Static Lib Initialization Code
* Auto generated, do not modify
* Build system source code licensed under MIP license
***/

#include "build.h"

#include "gtest/gtest.h"

int main(int argc, char** argv) {
    extern void InitModule_test_zlib(void*);
    InitModule_test_zlib(nullptr);


  int ret = 0;

  testing::InitGoogleTest(&argc, argv);
  ret = RUN_ALL_TESTS();

  return ret;
}
