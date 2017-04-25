#include "gtest/gtest.h"
#include "lib/stream.h"
#include <iostream>
#include <vector>

#define LOG_TRACE printf("Entering %s() (%s:%d)\n", __FUNCTION__, __FILE__, __LINE__);


TEST(StreamTest, Map) {
	LOG_TRACE
	auto stream = makeStream({3, 3});
	auto result = stream | map([](auto x) { return x * x; }) | to_vector();
	EXPECT_EQ(result, std::vector<int>({9, 9}));
}

TEST(StreamTest, Filter) {
	auto stream = makeStream({2, 3});

	auto result = stream | filter([](auto x) { if (x == 2) return false; else return true; }) | to_vector();
	EXPECT_EQ(result, std::vector<int>({3}));
}

TEST(StreamTest, Reduce){
	auto stream = makeStream({2, 3});

	auto result = stream | reduce([](auto acc, auto val) {return acc * val;});
	EXPECT_EQ(result, 6);
}


TEST(StreamTest, ReduceTwo){

	auto stream = makeStream({3, 3});

	
	auto result = stream | reduce([](auto first) {std::vector<decltype(first)> v; v.push_back(first); return v;},
		[](auto acc, auto val) {acc.push_back(val); return acc;});

	
	EXPECT_EQ(result, std::vector<int>({3, 3}));
}



TEST(StreamTest, Sum){
	auto stream = makeStream({2, 3});

	auto result = stream | sum();
	EXPECT_EQ(result, 5);
}

TEST(StreamTest, Print){
	auto stream = makeStream({3,3,3});
	

	(stream | print_to(std::cout, " ")) << std::endl;
	SUCCEED();
	

}

TEST(StreamTest, Choose){
	auto stream = makeStream({3, 2});

	auto result = stream | nth(size_t(1));
	EXPECT_EQ(result, 2);
}

TEST(StreamTest, Group){
	auto stream = makeStream({3, 3});
	auto&& result = stream | group(2) |  to_vector();
	std::cout << "IMHERE " << result.size() << std::endl;
	EXPECT_EQ(result.at(0), std::vector<int>({3, 3}));
}

// int main(int argc, char *argv[])
// {
// 	::testing::InitGoogleTest(&argc, argv);
// 	return RUN_ALL_TESTS();
// }
