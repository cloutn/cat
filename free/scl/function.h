////////////////////////////////////
// 2022.10.23 caolei
////////////////////////////////////
#pragma once

#include <scl/ptr.h>

namespace scl {


template <typename return_t, typename... args_t>
class class_function_impl
{
public:
    using func_t = return_t (__any_class::*)(args_t...);

    class_function_impl              () { }
    class_function_impl              (void* _class, class_function_ptr _func) : m_caller(_class), m_func(_func) { }
    template<typename   extern_func_t>
    void                set         (void* _class, extern_func_t _func) { m_caller = _class; m_func = reinterpret_cast<class_function_ptr>(_func); }
    void                set         (void* _class, class_function_ptr _func) { m_caller = _class; m_func = _func; }
    void*               caller      () { return m_caller; }
    class_function_ptr  func        () { return m_func; }
    bool            operator==      (const class_function_impl& other) const
    {
        return this->m_caller == other.m_caller && this->m_func == other.m_func;
    }
    return_t        operator()      (args_t... args)
    {
        if (nullptr == m_caller || nullptr == m_func)
            return return_t();
        __any_class*    typed_class = reinterpret_cast<__any_class*>(m_caller);
        func_t          typed_func  = reinterpret_cast<func_t>(m_func);
        return (typed_class->*typed_func)(args...);
    }

private:
    void*               m_caller     = nullptr;
    class_function_ptr  m_func      = nullptr;

}; // class any_class_function



//template <typename class_t, typename return_t, typename... args_t>
//class class_def_function
//{
//public:
//    typedef return_t    (class_t::*func_t)(args_t...);
//
//    class_def_function      () { }
//    class_def_function      (class_t* _class, func_t _func) : m_class(_class), m_func(_func) { }
//    void set            (class_t* _class, func_t _func) { m_class = _class; m_func = _func; }
//    return_t operator   ()(args_t... args)
//    {
//        if (nullptr == m_class || nullptr == m_func)
//            return return_t();
//        return (m_class->*m_func)(args...);
//    }
//
//private:
//    class_t*     m_class    = nullptr;
//    func_t       m_func     = nullptr;
//
//}; // class class_function
//
//
//template <typename return_t, typename... args_t>
//class caller_function
//{
//public:
//    typedef return_t            (*func_t)(void*, args_t...);
//
//    caller_function             () { }
//    caller_function             (void* _caller, func_t _func) : m_caller(_caller), m_func(_func) { }
//    void            set         (void* _caller, func_t _func) { m_caller = _class; m_func = _func; }
//    const void*     caller      () const { return m_caller; }
//    const func_t    func        () const { return m_func; }
//    return_t        operator()  (args_t... args) 
//    {
//        if (nullptr == m_class || nullptr == m_func)
//            return return_t();
//        return (m_func)(m_caller, args...);
//    }
//
//private:
//    void*   m_caller  = nullptr;
//    func_t  m_func    = nullptr;
//
//}; // class caller_function

template <typename T>
struct get_func_base_type {};

template <typename return_t, typename... args_t>
struct get_func_base_type<return_t (args_t...)>
{
	using type = scl::class_function_impl<return_t, args_t...>;
};

template <typename T>
class class_function : public get_func_base_type<T>::type
{
public:
	using base_type = typename get_func_base_type<T>::type;
    class_function  () { }
    class_function  (void* _class, class_function_ptr _func) : base_type(_class, _func) { }
    void set(base_type func)
    {
        base_type::set(func.caller(), func.func());
    }
};

template <typename class_t, typename return_t, typename... args_t> inline class_function<return_t (args_t...)> 
bind(void* caller, return_t (class_t::*pfunc)(args_t...))
{
    return { caller, reinterpret_cast<class_function_ptr>(pfunc) };
}

} // namespace scl


