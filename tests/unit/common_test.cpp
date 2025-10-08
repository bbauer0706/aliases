#include <gtest/gtest.h>
#include "aliases/common.h"

using namespace aliases;

// Test fixture for common utilities
class CommonTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// String utility tests
TEST_F(CommonTest, TrimRemovesLeadingWhitespace) {
    EXPECT_EQ(trim("   hello"), "hello");
    EXPECT_EQ(trim("\t\nhello"), "hello");
}

TEST_F(CommonTest, TrimRemovesTrailingWhitespace) {
    EXPECT_EQ(trim("hello   "), "hello");
    EXPECT_EQ(trim("hello\t\n"), "hello");
}

TEST_F(CommonTest, TrimRemovesBothSides) {
    EXPECT_EQ(trim("  hello  "), "hello");
    EXPECT_EQ(trim("\t\nhello\t\n"), "hello");
}

TEST_F(CommonTest, TrimHandlesEmptyString) {
    EXPECT_EQ(trim(""), "");
    EXPECT_EQ(trim("   "), "");
}

TEST_F(CommonTest, TrimHandlesNoWhitespace) {
    EXPECT_EQ(trim("hello"), "hello");
}

TEST_F(CommonTest, SplitBasicDelimiter) {
    auto result = split("a,b,c", ',');
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "b");
    EXPECT_EQ(result[2], "c");
}

TEST_F(CommonTest, SplitEmptyString) {
    auto result = split("", ',');
    EXPECT_TRUE(result.empty() || (result.size() == 1 && result[0].empty()));
}

TEST_F(CommonTest, SplitNoDelimiter) {
    auto result = split("hello", ',');
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "hello");
}

TEST_F(CommonTest, SplitMultipleConsecutiveDelimiters) {
    auto result = split("a,,b", ',');
    EXPECT_GE(result.size(), 2);
}

TEST_F(CommonTest, StartsWithTrue) {
    EXPECT_TRUE(starts_with("hello world", "hello"));
    EXPECT_TRUE(starts_with("test", "test"));
}

TEST_F(CommonTest, StartsWithFalse) {
    EXPECT_FALSE(starts_with("hello world", "world"));
    EXPECT_FALSE(starts_with("test", "testing"));
}

TEST_F(CommonTest, StartsWithEmptyPrefix) {
    EXPECT_TRUE(starts_with("hello", ""));
}

TEST_F(CommonTest, StartsWithEmptyString) {
    EXPECT_FALSE(starts_with("", "hello"));
}

TEST_F(CommonTest, EndsWithTrue) {
    EXPECT_TRUE(ends_with("hello world", "world"));
    EXPECT_TRUE(ends_with("test", "test"));
}

TEST_F(CommonTest, EndsWithFalse) {
    EXPECT_FALSE(ends_with("hello world", "hello"));
    EXPECT_FALSE(ends_with("test", "testing"));
}

TEST_F(CommonTest, EndsWithEmptySuffix) {
    EXPECT_TRUE(ends_with("hello", ""));
}

TEST_F(CommonTest, EndsWithEmptyString) {
    EXPECT_FALSE(ends_with("", "hello"));
}

// Result type tests
TEST_F(CommonTest, ResultSuccessCreation) {
    auto result = Result<int>::success_with(42);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.value, 42);
    EXPECT_TRUE(result.error_message.empty());
}

TEST_F(CommonTest, ResultErrorCreation) {
    auto result = Result<int>::error("Something went wrong");
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_message, "Something went wrong");
}

TEST_F(CommonTest, ResultBoolConversion) {
    auto success_result = Result<int>::success_with(42);
    auto error_result = Result<int>::error("error");

    EXPECT_TRUE(static_cast<bool>(success_result));
    EXPECT_FALSE(static_cast<bool>(error_result));
}

// Directory tests (basic checks that don't require specific setup)
TEST_F(CommonTest, GetHomeDirectoryNotEmpty) {
    auto home = get_home_directory();
    EXPECT_FALSE(home.empty());
}

TEST_F(CommonTest, GetCurrentDirectoryNotEmpty) {
    auto cwd = get_current_directory();
    EXPECT_FALSE(cwd.empty());
}
