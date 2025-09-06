#pragma once

namespace scl {

//////////////////////////////////////////////////////////////////////////
/// remove_const 
//////////////////////////////////////////////////////////////////////////
template<class T>
struct remove_const {
	using type = T;
};

template<class U>
struct remove_const<const U> {
	using type = U;
};

template<class T>
using remove_const_t = typename remove_const<T>::type;


//////////////////////////////////////////////////////////////////////////
/// remove_pointer
//////////////////////////////////////////////////////////////////////////
template<class T>
struct remove_pointer {
	using type = T;
};

template<class U>
struct remove_pointer<U*> {
	using type = U;
};

template<class T>
using remove_pointer_t = typename remove_pointer<T>::type;


//////////////////////////////////////////////////////////////////////////
/// conditional
//////////////////////////////////////////////////////////////////////////
template<bool B>
struct bool_constant {
    static constexpr bool value = B;
};

template<bool B, class T, class F>
struct conditional { using type = T; };

template<class T, class F>
struct conditional<false, T, F> { using type = F; };

template<bool B, class T, class F>
using conditional_t = typename conditional<B, T, F>::type;


//////////////////////////////////////////////////////////////////////////
/// is_function
//////////////////////////////////////////////////////////////////////////
template<class T> struct is_function : bool_constant<false> {};

template<class R, class... Args>
struct is_function<R(Args...)> : bool_constant<true> {};

template<class R, class... Args>
struct is_function<R (*)(Args...)> : bool_constant<true> {};

template<class R, class... Args>
struct is_function<R (&)(Args...)> : bool_constant<true> {};

//////////////////////////////////////////////////////////////////////////
/// is_void
//////////////////////////////////////////////////////////////////////////
template<class T> struct is_void             : bool_constant<false> {};
template<>        struct is_void<void>       : bool_constant<true> {};
template<>        struct is_void<const void> : bool_constant<true> {};
template<>        struct is_void<volatile void>       : bool_constant<true> {};
template<>        struct is_void<const volatile void> : bool_constant<true> {};

//////////////////////////////////////////////////////////////////////////
/// is_integral
//////////////////////////////////////////////////////////////////////////
template<class T> struct is_int: bool_constant<false> {};
template<> struct is_int<bool>              : bool_constant<true> {};
template<> struct is_int<char>              : bool_constant<true> {};
template<> struct is_int<signed char>       : bool_constant<true> {};
template<> struct is_int<unsigned char>     : bool_constant<true> {};
template<> struct is_int<wchar_t>           : bool_constant<true> {};
template<> struct is_int<char16_t>          : bool_constant<true> {};
template<> struct is_int<char32_t>          : bool_constant<true> {};
template<> struct is_int<short>             : bool_constant<true> {};
template<> struct is_int<unsigned short>    : bool_constant<true> {};
template<> struct is_int<int>               : bool_constant<true> {};
template<> struct is_int<unsigned int>      : bool_constant<true> {};
template<> struct is_int<long>              : bool_constant<true> {};
template<> struct is_int<unsigned long>     : bool_constant<true> {};
template<> struct is_int<long long>         : bool_constant<true> {};
template<> struct is_int<unsigned long long>: bool_constant<true> {};

//////////////////////////////////////////////////////////////////////////
/// is_float
//////////////////////////////////////////////////////////////////////////
template<class T> struct is_float: bool_constant<false> {};
template<>        struct is_float<float>       : bool_constant<true> {};
template<>        struct is_float<double>      : bool_constant<true> {};
template<>        struct is_float<long double> : bool_constant<true> {};

//////////////////////////////////////////////////////////////////////////
/// is_pointer
//////////////////////////////////////////////////////////////////////////
template<class T>
struct is_pointer : bool_constant<false> {};

template<class T>
struct is_pointer<T*> : bool_constant<true> {};

//////////////////////////////////////////////////////////////////////////
/// is_class
//////////////////////////////////////////////////////////////////////////
template<class...> using void_t = void;

template<class T, class = void>
struct is_class : bool_constant<false> {};

template<class T>
struct is_class<T, void_t<int T::*> > : bool_constant<true> {};

template<typename, typename>		struct is_same_      { enum { value = 0 }; };
template<typename T>				struct is_same_<T,T> { enum { value = 1 }; };
template<typename T, typename U>	constexpr bool is_same_v = is_same_<T,U>::value;

} // namespace scl

