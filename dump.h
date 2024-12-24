#ifndef DP_DUMP
#define DP_DUMP

/*
*	A dump tool, in line with the proposed P2879R0 paper
*	The intention is that this will work via pure standard C++ from C++20 and later
*	For earlier versions we will depend on libfmt
*	But, this means that we can't depend on certain modern tools since the code might be
*	called in C++11 mode
*/


#include <string>
#include <algorithm>
#include <cstdio> //for std::FILE

#ifdef _MSVC_LANG
#define DP_DUMP_CPLUSPLUS _MSVC_LANG
#else
#define DP_DUMP_CPLUSPLUS __cplusplus
#endif

//Check for the availablility of __has_include, as it makes a great many of our checks and potential error messages
//cleaner.
//It was formalised into standard C++ as part of C++17, but existed in certain implementations for a lot longer before that
#if DP_DUMP_CPLUSPLUS >= 201703L
#define DP_DUMP_HAS_INCLUDE
#elif defined(__GNUC__)
#ifdef __has_include 
#define DP_DUMP_HAS_INCLUDE
#endif
#endif


#ifdef DP_DUMP_HAS_INCLUDE
#if __has_include(<version>)
#include <version>
#endif
#endif


#include <ostream> 
#include <type_traits>
#include <utility>

#ifdef DP_DUMP_USE_FMTLIB
#define DP_DUMP_USING_FMT
//Generate a nicer error message if possible
	#ifdef DP_DUMP_HAS_INCLUDE
		#if ! __has_include("fmt/format.h")
		#error "Using dp::dump and fmtlib but could not find fmtlib headers. Please check your include path"
		#endif
	#endif
#include "fmt/format.h"
#include "fmt/ostream.h"
#elif defined (__cpp_lib_print)
#include <print>
#define DP_DUMP_USING_STD_PRINT
#elif defined(__cpp_lib_format)
#include <format>
#define DP_DUMP_USING_STD_FORMAT
#endif

namespace dp{
namespace detail{
	template<typename... Args>
	using dump_first_type = typename std::tuple_element<0, std::tuple<Args...>>::type;
}
}

//Now for the main SFINAE to resolve an issue where a class derived from std::ofstream would more readily
//bind as a pack paramter than to std::ostream&
#if defined(__cpp_concepts)
//Since we might be in a world where a person has <concepts> but not <version>, we handspin
namespace dp::detail{
	template<typename T>
	concept dump_not_derived_from_ostream = !std::is_base_of_v<std::ostream, std::remove_reference_t<T>>;
}
#define DP_DUMP_VARIADIC_PREAMBLE template<typename... Args> requires dp::detail::dump_not_derived_from_ostream<dp::detail::dump_first_type<Args...>>
#else
//Nested namespace declarations are not C++11 so we have to do this by the book
namespace dp{
namespace detail{
	template<typename... Args>
	struct dump_not_derived_from_ostream{
		//Let's not get too deep in nesting where it can be avoided
		using first_t = dump_first_type<Args...>;
		static const bool value = ! std::is_base_of<std::ostream, typename std::remove_reference<first_t>::type>::value;
	};
}}
#define DP_DUMP_VARIADIC_PREAMBLE template<typename... Args, typename std::enable_if<dp::detail::dump_not_derived_from_ostream<Args...>::value, void>::type* = nullptr>
#endif



namespace dp{
namespace detail{
	//Since std::size is not available in earlier C++ versions
	template<typename T, std::size_t N>
	constexpr std::size_t size(T (&)[N]){
		return N;
	}

	//A class which can generate an appropriate format string at comptime
	template<std::size_t N>
	class dump_string_buf{

		char buf[N] = {};
	public:
		constexpr dump_string_buf(){
			std::fill(std::begin(buf), std::end(buf), '\0');
		}	
	
		constexpr dump_string_buf(const char (&arr)[N]){
			//std::copy isn't constexpr in all targets
			for(std::size_t i = 0; i < N; ++i){
				buf[i] = arr[i];
			}
		}

		constexpr dump_string_buf(const std::string& arr){
			std::fill(std::begin(buf), std::end(buf), '\0');
			std::copy_n(std::begin(arr), N, std::begin(buf));
		}

		constexpr const char* c_str() const{
			return buf;
		}

		constexpr const auto* begin() const {
			return std::begin(buf);
		}
		constexpr const auto* end() const {
			return std::end(buf);
		}

		constexpr auto size() const{
			return N;
		}

	};

	template<std::size_t N>
	#ifdef __cpp_consteval
	consteval
	#else
	constexpr
	#endif
	auto generate_format_string(){
		//3N + 1 to cover "{} " for each term plus a null terminator	
		char array[3*N + 1] = {};
		for(std::size_t i = 0; i + 3 < dp::detail::size(array); i += 3){
			array[i] = '{';
			array[i + 1] = '}';
			array[i + 2] = ' ';
		}
		array[3*N] = '\0';
		return dump_string_buf<3*N + 1>{array};
	}



}	//namespace dp::detail


	template<typename... Args>
	void dump(std::FILE* fl, Args&&... args){
		static constexpr auto format_string {detail::generate_format_string<sizeof...(Args)>()};
		#ifdef DP_DUMP_USING_STD_PRINT
		std::print(fl, static_cast<std::format_string<Args...>>(format_string.c_str()), std::forward<Args>(args)...);
		#elif defined(DP_DUMP_USING_STD_FORMAT)
 		std::fputs(std::format(static_cast<std::format_string<Args...>>(format_string.c_str()), std::forward<Args>(args)...).c_str(), fl);
		#elif defined(DP_DUMP_USING_FMT)
		fmt::print(fl, static_cast<fmt::format_string<Args...>>(format_string.c_str()), std::forward<Args>(args)...);
		#else
		#error "Attempt to use dump without a valid formatting handler"
		#endif
	}
	DP_DUMP_VARIADIC_PREAMBLE
	void dump(Args&&... args){
		dump(stdout, std::forward<Args>(args)...);
	}
	template<typename... Args>
	void dump(std::ostream& os, Args&&... args){
		static constexpr auto format_string{detail::generate_format_string<sizeof...(Args)>()};
		#ifdef DP_DUMP_USING_STD_PRINT
		std::print(os, static_cast<std::format_string<Args...>>(format_string.c_str()), std::forward<Args>(args)...);
		#elif defined(DP_DUMP_USING_STD_FORMAT)
		os << std::format(static_cast<std::format_string<Args...>>(format_string.c_str()), std::forward<Args>(args)...);
		#elif defined(DP_DUMP_USING_FMT)
		fmt::print(os, static_cast<fmt::format_string<Args...>>(format_string.c_str()), std::forward<Args>(args)...);
		#else
		#error "Attempt to use dump without a valid formatting handler"
		#endif
	}
	

	
	template<typename... Args>
	void dumpln(std::FILE* fl, Args&&... args){
		dump(fl, std::forward<Args>(args)...);
		std::fputc('\n', fl);
	}

	DP_DUMP_VARIADIC_PREAMBLE
	void dumpln(Args&&... args){
		dumpln(stdout, std::forward<Args>(args)...);
	}
	
	template<typename... Args>
	void dumpln(std::ostream& os, Args&&... args){
		dump(os, std::forward<Args>(args)...);
		os << '\n';
	}

	
	
}	//namespace dp


#undef DP_DUMP_USING_STD_PRINT
#undef DP_DUMP_USING_STD_FORMAT
#undef DP_DUMP_CPLUSPLUS
#undef DP_DUMP_VARIADIC_PREAMBLE
#undef DP_DUMP_HAS_INCLUDE
#undef DP_DUMP_USING_FMT

#endif
