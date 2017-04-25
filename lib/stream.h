#ifndef __STREAM_H__
#define __STREAM_H__

#include <iostream>
#include <iterator>
#include <vector>
#include <exception>
#include <initializer_list>
#include <functional>
#include <memory>
#include <type_traits>

#define LOG_TRACE printf("Entering %s() (%s:%d)\n", __FUNCTION__, __FILE__, __LINE__);

class emptyexception: public std::exception
{
  virtual const char* what() const throw()
  {
    return "Элементы закончились";
  }
} emptyex;




template<class ChildStream>
struct Stream {

	auto get()   {
		return std::move(static_cast<ChildStream*>(this)->get());
	};
};



template<class Iterator>
class ProviderStream : public Stream<ProviderStream<Iterator>>{
protected:
	Iterator beginIter;
	Iterator endIter;
	Iterator iter;
	
public:
	ProviderStream(Iterator& begin, Iterator& end)
		:beginIter(begin), endIter(end){
		iter = begin;	
		std::cout << "Lvalue for ProviderStream" << std::endl;
	};	
	ProviderStream(Iterator&& begin, Iterator&& end)
		:beginIter(std::move(begin)), endIter(std::move(end)){
			iter = begin;
			std::cout << "Rvalue for ProviderStream" << std::endl;
	}
	auto get() {
		if (iter == endIter) {
			return std::shared_ptr<std::remove_const_t<std::remove_reference_t<decltype(*beginIter)>>>();
		}
		auto value = std::make_shared<std::remove_const_t<std::remove_reference_t<decltype(*beginIter)>>>(std::move(*iter));
	
		iter++;
		return (value);
	};
};

template<class Container>
class ContainerProviderStream : public ProviderStream<typename Container::iterator>
{
	Container container;
	typedef typename Container::iterator container_iterator;
public:
	ContainerProviderStream();
	ContainerProviderStream(const ContainerProviderStream& c) : ProviderStream<container_iterator>(c.ProviderStream<container_iterator>::beginIter, 
		c.ProviderStream<container_iterator>::endIter), container(c.container) {};
	ContainerProviderStream(ContainerProviderStream& c) : ProviderStream<container_iterator>(c.ProviderStream<container_iterator>::beginIter, 
		c.ProviderStream<container_iterator>::endIter), container(c.container) {};
	ContainerProviderStream(ContainerProviderStream&& c) : ProviderStream<container_iterator>(c.ProviderStream<container_iterator>::beginIter, 
		c.ProviderStream<container_iterator>::endIter), container(std::move(c.container)) {};
	ContainerProviderStream(Container&& c)
		: ProviderStream<container_iterator>(c.begin(), c.end()), container(std::move(c))
		{};
	ContainerProviderStream(Container& c)
		: ProviderStream<container_iterator>(c.begin(), c.end()), container(c)
		{};
	ContainerProviderStream(const Container& c)
		: ProviderStream<container_iterator>(c.begin(), c.end()), container(c)
		{};
	auto get() {return std::move(ProviderStream<container_iterator>::get());};
	
};








template<class Function>
class OperatorSec{
	
	Function function;
public:
	OperatorSec(Function&& f)
		:function(std::move(f)) {};

	template<class StreamT>
	auto operator()(StreamT&& stream) -> std::result_of_t<Function(StreamT&&)>
	{
		return function(std::forward<StreamT>(stream));
	}

};


template<class Function>
auto makeOperator(Function&& f){
	//LOG_TRACE
	return OperatorSec<Function>(std::forward<Function>(f));
}


auto print_to(std::ostream& os, std::string delimiter = " "){
	return makeOperator([&os, delimiter](auto&& stream) -> std::ostream& {
		while (true){
			auto ref = stream.get();
			if (ref.get() == nullptr) break;
			os << *(ref) << delimiter;
		}
        
		return os;
	});
};

template<class Stream, class Function>
auto operator|(Stream&& stream, OperatorSec<Function>&& op) -> std::result_of_t<decltype(op)(Stream&&)>{
	return op(std::forward<Stream>(stream));
};
template<class Iterator>
auto makeStream(Iterator begin, Iterator end){
	std::cout<< "Iterator factory" << std::endl;
	return ProviderStream<Iterator>(begin, end);
};
template<class Container>
auto makeStream(const Container& c){
	std::cout<< "const Lvalue factory" << std::endl;
	return makeStream(c.begin(), c.end());	
};
template<class Container>
auto makeStream(Container& c){
	std::cout<< "Lvalue factory" << std::endl;
	return makeStream(c.begin(), c.end());
};
template<class T>
auto makeStream(std::initializer_list<T> init){
	std::cout<< "std::initializer_list factory" << std::endl;
	return makeStream(init.begin(), init.end());
}


template<class Container>
auto makeStream(Container&& c){
	std::cout<< "Rvalue factory" << std::endl;
	return ContainerProviderStream<Container>(std::move(c));
}

auto sum(){
	return makeOperator([](auto&& stream) -> auto {
		
		auto res_ref = stream.get();
		using R = std::remove_const_t<std::remove_reference_t<decltype(*stream.get())>>;
		R res;
		if (res_ref.get() == nullptr) return res;
		res = *(res_ref);
		while(true){
			auto tmp_ref = stream.get();
			if (tmp_ref.get() == nullptr) return res;
			res += *(tmp_ref);
		}
	});
}

template<class StreamT, class Function>
class MapStream : public Stream<MapStream<StreamT, Function>>
{
	StreamT stream;
	Function function;
public:
	MapStream(StreamT&& theStream, Function&& f)
		:stream(std::forward<StreamT>(theStream)), function(std::forward<Function>(f)){};

	auto get(){
			//auto&& v = std::move(stream.get());
			//auto&& v = std::move(stream.get());
			//std::cout<< "BEFORE: " << v << std::endl;
			//auto&& res = std::move(function(v));
			//std::cout<< "AFTER: "  << res << std::endl;
		auto ref = stream.get();
		if (ref.get() == nullptr) return ref;
		return std::make_shared< typename decltype(ref)::element_type >(std::move(function(std::move(*ref))));
	};
};

template<class Function>
auto map(Function&& f){
	//return MapOperator<Function>(std::move(f));
	return makeOperator([&f](auto&& stream) -> auto{
		return MapStream<decltype(stream), Function>(std::forward<decltype(stream)>(stream), std::forward<Function>(f));
	});
}


//FILTER
template<class StreamT, class Predicate>
class FilterStream : public Stream<FilterStream<StreamT, Predicate>>
{
	StreamT stream;
	Predicate predicate;
public:
	FilterStream(StreamT&& theStream, Predicate&& p)
		:stream(std::forward<StreamT>(theStream)), predicate(std::forward<Predicate>(p)){
	};

	auto get(){
		while(true){
			auto ref = stream.get();
			if (ref.get() == nullptr) return ref;
			if (predicate(std::move(*ref)) == true) return ref;
		}
			
	};
};

template<class Predicate>
auto filter(Predicate&& p){
	//return FilterOperator<Predicate>(std::move(p));
	return makeOperator([&p](auto&& stream) -> auto {
		return FilterStream<decltype(stream), Predicate>(std::forward<decltype(stream)>(stream), std::forward<Predicate>(p));
	});
}

template<class Accumulator>
auto reduce(Accumulator&& acc){
	return makeOperator([&acc](auto&& stream){
		using R = std::remove_const_t<std::remove_reference_t<decltype(*(stream.get()))>>;
		auto ref = stream.get();
		R res;
		if (ref.get() == nullptr) return res;
		res = *(ref);
		while(true){
			ref = stream.get();
			if (ref.get() == nullptr) return res;
			
			res = acc(res, *(ref));
				
		}
		return res;
	});
}

template<class Accumulator, class IdentifyFn>
auto reduce(IdentifyFn&& identity, Accumulator&& acc){
	return makeOperator([&acc, &identity](auto&& stream){
		//using StreamR = decltype(stream.get());
		using R = std::remove_const_t<std::remove_reference_t<decltype(identity(*(stream.get())))>>;
		
		R res; //= identity(stream.get());
		auto ref = stream.get();
		if (ref.get() == nullptr) return res;
		res = identity(*ref);
		while(true){
			ref = stream.get();
			if (ref.get() == nullptr) return res;

			res = acc(res, *ref);
			
		}
		return res;
	});
}

template<class StreamT>
class SkipStream : public Stream<SkipStream<StreamT>>
{
	StreamT stream;
	const size_t N;
public:
	SkipStream(StreamT&& theStream, const size_t theN)
		:stream(std::forward<StreamT>(theStream)), N(theN){
	};

	auto get(){
		for (auto i = 0; i < N; i++){
			stream.get();
		}


		
		return stream.get();
		
	};
};


auto skip(size_t n){
	return makeOperator([n](auto&& stream){
		
		return SkipStream<decltype(stream)>(std::forward<decltype(stream)>(stream), n);
	});
}

auto to_vector(){
	return makeOperator([](auto&& stream) -> auto{
		using R = std::remove_reference_t<std::remove_const_t<decltype(*(stream.get()))>>;
		std::vector<R> v;
		while(true){
				//auto&& a = stream.get();
				//std::cout << "TO_VEC " << a.size()  << " " << typeid(a).name() << std::endl;
				///std::cout << "WE ARE HERE" << std::endl;
			auto ref = stream.get();
			if (ref.get() == nullptr) return v;
			v.emplace_back(std::move(*ref));
		}
			
		return (v);
	});
}

template<class StreamT> 
class GroupStream : public Stream<GroupStream<StreamT>>
{
	StreamT stream;
	const size_t N;
public:
	using in_type = std::remove_const_t<std::remove_reference_t<decltype(*(stream.get()))>>;
	GroupStream(StreamT&& theStream, const size_t theN)
		:stream(std::forward<StreamT>(theStream)), N(theN) {};

	std::shared_ptr<std::vector<in_type>> get()  {
		std::vector<in_type> v;
		while(v.size() < N){
			auto ref = stream.get();
			if (ref.get() == nullptr) return std::shared_ptr<std::vector<in_type>>();
			v.emplace_back(std::move(*ref));
		}
		std::cout << "IN GROUP" << std::endl;
		for (size_t i = 0; i < v.size(); i++) {
			std::cout << v.at(i) << std::endl;
		}
		std::cout << "OUT GROUP" << std::endl;			
		return std::make_shared<std::vector<in_type>>(std::move(v));
	};
};
auto group(const size_t N){
	return makeOperator([N](auto&& stream) {
		using R = std::remove_const_t<std::remove_reference_t<decltype(stream)>>;
		return GroupStream<R>(std::forward<R>(stream), N);
	});
}

auto nth(size_t index){
	return makeOperator([index](auto&& stream){
		for (size_t i = 0; i < index; ++i){
			stream.get();
		}
		auto ref = stream.get();
		
		return *ref;
	});
}

// struct A {
// public:
// 	A() {
// 		std::cout << "A ctor\n";
// 	}

// 	A (const A& other){
// 		std::cout << "A copy ctor\n";
// 	}

// 	A(A&& other){
// 		std::cout << "A move ctor\n";
// 	}

// 	~A() {
// 		std::cout << "A d tor\n";
// 	}
// };


#endif

