#ifndef DP_DUMP
#define DP_DUMP

#include <string>
#include <algorithm>
#include <cstdio> //for std::FILE

#if __has_include(<version>)
#include <version>
#else
#include <ostream>  //Specified to contain the include __cpp_lib_print
#endif

#ifdef __cpp_lib_print
#include <print>
#define DP_DUMP_USING_STD_PRINT
#elif defined(__cpp_lib_format)
#include <format>
#define DP_DUMP_USING_STD_FORMAT
#endif

namespace dp{
namespace detail{


	//A class which can generate an appropriate format string at comptime
	template<std::size_t N>
	class dump_string_buf{

		char buf[N];
	public:

	
		constexpr dump_string_buf(const char (&arr)[N]){
			std::copy(std::begin(arr), std::end(arr), std::begin(buf));
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
		char array[3*N + 1];
		for(std::size_t i = 0; i + 3 < std::size(array); i += 3){
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
		#else
		#error "Attempt to use dump without a valid formatting handler"
		#endif
	}
	template<typename... Args>
	void dump(Args&&... args){
		dump(stdout, std::forward<Args>(args)...);
	}

	template<typename... Args>
	void dumpln(std::FILE* fl, Args&&... args){
		static constexpr auto format_string{detail::generate_format_string<sizeof...(Args)>()};
		#ifdef DP_DUMP_USING_STD_PRINT
		std::println(fl, static_cast<std::format_string<Args...>>(format_string.c_str()), std::forward<Args>(args)...);
		#elif defined(DP_DUMP_USING_STD_FORMAT)
		std::fputs(std::format(static_cast<std::format_string<Args...>>(format_string.c_str()), std::forward<Args>(args)...).c_str(), fl);
		#else
		#error "Attempt to use dumpln without a valid formatting handler"
		#endif
	}
	template<typename... Args>
	void dumpln(Args&&... args){
		dumpln(stdout, std::forward<Args>(args)...);
	}

	
	
}	//namespace dp


#undef DP_DUMP_USING_STD_PRINT
#undef DP_DUMP_USING_STD_FORMAT


#endif
