// Copyright 2022-2025 Stuart Scott
#ifndef TEST_INCLUDE_WINKTEST_UTILS_H_
#define TEST_INCLUDE_WINKTEST_UTILS_H_

#include <gtest/gtest.h>

#define ASSERT_ARRAY_EQ(length, expected, actual) \
  for (int i = 0; i < length; i++)                \
    ASSERT_EQ(expected[i], actual[i]) << "Index: " << i;

#endif  // TEST_INCLUDE_WINKTEST_UTILS_H_
