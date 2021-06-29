//
// Created by esteban on 29/06/2021.
//

#include "/lib/googletest-master/googletest/include/gtest/gtest.h"

extern "C" {
    #include "../src/shell/shell.h"
}

TEST(shell_add){
    int result = 4;
    int test = shell_add(2,2);
    ASSERT_EQ(result, test);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}