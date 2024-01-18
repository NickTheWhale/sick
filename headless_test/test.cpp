#include "pch.h"

#include "..\headless\headless\src\main.cpp"
//#include "..\common\common\include\common\filter_pipeline.h"
//#include "common/include/common/filter_pipeline.h"

TEST(Main_parse_filters, parse_filters)
{
	filter::filter_pipeline pipeline;
	parse_filters("./resources/all_filters_valid.json", pipeline);
}
TEST(TestCaseName, TestName) {
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
  EXPECT_TRUE(false) << "hey dumbass you put false";
}