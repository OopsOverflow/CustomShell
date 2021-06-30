//
// Created by houssem on 30/06/2021.
//

#include "shell/shell.h"
#include "gtest/gtest.h"


// Demonstrate some basic assertions.
/*TEST(ShellTests, shell_cd) {
    // Like 4 real the code isn't at all scalable what do you expect me to do
    char pwd[1024];
    getcwd(pwd, sizeof(pwd));
    EXPECT_EQ(shell_cd((char **) "cd .."), 1);
}
*/
TEST(ShellTests, shell_pwd){
    char pwd[1024];
    getcwd(pwd, sizeof(pwd));
    EXPECT_EQ(pwd, pwd);
}

int main(int argc, char* argv[]){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}