#pragma once

#if TEST_ENABLED
//#include "doctest.h"

#define TEST_INTERGRATION_TEST(name)\
TEST_SUITE("INTERGRATION" * doctest::description(#name))

#define TEST_UNIT_TEST(name)

#else

#define TEST_INTERGRATING_TEST(name)
#define TEST_UNIT_TEST(name)

#endif
