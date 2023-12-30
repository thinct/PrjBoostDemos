#pragma once
#include <stdlib.h>
#include <type_traits>
#include <iostream>

namespace illusion {

	template<int x> 
	struct number { 
		using value = number<x>; static constexpr int instance = x; 
	};
	template<bool x> 
	struct boolean { 
		using value = boolean<x>; static constexpr bool instance = x; 
	};

	namespace impl {
		template<typename a, typename b> 
		using add_impl = number<a::value::instance + b::value::instance>;
		template<typename a, typename b> 
		using times_impl = number<a::value::instance * b::value::instance>;

		template<template<typename, typename> typename op, typename ...x> 
		struct accumulate_helper;
		template<template<typename, typename> typename op, typename f, typename ...x>
		struct accumulate_helper<op, f, x...> { 
			using value = typename op<f, accumulate_helper<op, x...>>::value; 
		};
		template<template<typename, typename> typename op, typename a, typename b>
		struct accumulate_helper<op, a, b> { 
			using value = typename op<a, b>::value; 
		};
	}

	template<typename ...x> using add = impl::accumulate_helper<impl::add_impl, x...>;
	template<typename ...x> using times = impl::accumulate_helper<impl::times_impl, x...>;
	//------------------------------------------------------

	template<typename a, typename b> struct is_equal {
		using value = typename boolean <a::value::instance < b::value::instance>;
	};

	//------------------------------------------------------

	template<typename condition, typename true_statement, typename false_statement>
	struct if_else {
		using value = typename std::conditional_t<condition::value::instance, true_statement, false_statement>::value;
	};
	//------------------------------------------------------

	template<typename ...x> struct cond;
	template<typename condition, typename statement, typename ...x>
	struct cond<condition, statement, x...> {
		using value = typename if_else<condition, statement, cond<x...>>::value;
	};

	template<typename x>
	struct display {
		display(std::ostream& os = std::cout) { os << x::value::instance << std::endl; }
	};
	//------------------------------------------------------
}

using namespace illusion;
using namespace std;

int main(int argc, char* argv[])
{
	cout << add<number<1>, number<2>, number<3>, number<4>>::value::instance << endl;
	cout << times<number<1>, number<2>, number<3>, number<4>>::value::instance << endl;

	cout << is_equal<number<1>, number<2>>::value::instance << endl;

	cout << if_else<is_equal<number<1>, number<2>>
		, number<123>
		, number<555>
	>::value::instance << endl;

	using x = number<1>;
	using result = cond<
		is_equal<x, number<1>>, number<1>,
		is_equal<x, number<2>>, number<10>,
		is_equal<x, number<3>>, number<100>
	>;

	display<result>();
	system("pause");
	return 0;
}
