#include "lib/stream.h"
#include <iostream>
#include <string>
#include <memory>

#define LOG_TRACE printf("Entering %s() (%s:%d)\n", __FUNCTION__, __FILE__, __LINE__);


struct A {
public:
	A() {
		std::cout << "A ctor\n";
	}

	A (const A& other){

		//LOG_TRACE
		std::cout << "A copy ctor\n";
	}

	A(A&& other) noexcept{
		std::cout << "A move ctor\n";
	}

	~A() {
		std::cout << "A d tor\n";
	}
};

std::ostream& operator<<(std::ostream& os, A a){
	os << "A here";
	return os;
}

int main(int argc, char** argv) {
	

	std::vector<A> v(3);
	auto stream = makeStream(v);
    auto result = stream | map([](auto a) -> auto {return a;} ) | to_vector();
        //std::cout << "IMHERE " << result.size() << std::endl;
        //auto& vec = result.at(0);
	//std::cout << vec.size() << std::endl;
	//for (int i = 0; i < 2; i++){
	//	std::cout << vec.at(i);
	//}

	//std::vector<A> v(4);
	//auto stream = makeStream(v);
	//stream | map([](auto a) {return a;}) | print_to(std::cout, " ");
	
	//for (auto i = result.begin(); i != result.end(); i++){
	//	std::cout << *i << std::endl;
	//}
	

  	return 0;
}
