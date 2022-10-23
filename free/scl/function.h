////////////////////////////////////
// 2022.10.23 caolei
////////////////////////////////////
#pragma once

#include <scl/ptr.h>

namespace scl {
	
template <typename class_t, typename return_t, typename... args_t>
class class_function
{
public:
    typedef return_t    (class_t::*func_t)(args_t...);

    class_function      () { }
    class_function      (class_t* _class, func_t _func) : m_class(_class), m_func(_func) { }
    void set            (class_t* _class, func_t _func) { m_class = _class; m_func = _func; }
    return_t operator   ()(args_t... args)
    {
        return (m_class->*m_func)(args...);
    }

private:
    class_t*     m_class    = nullptr;
    func_t       m_func     = nullptr;

}; // class class_function

template <typename return_t, typename... args_t>
class any_class_function
{
public:
    typedef return_t    (__any_class::*func_t)(args_t...);

    any_class_function  () { }
    any_class_function  (void* _class, class_function_ptr _func) : m_class(_class), m_func(_func) { }
    template<typename   extern_func_t>
    void set            (void* _class, extern_func_t _func) { m_class = _class; m_func = reinterpret_cast<class_function_ptr>(_func); }
    void set            (void* _class, class_function_ptr _func) { m_class = _class; m_func = _func; }
    bool operator==     (const any_class_function& other) const
    {
        return this->m_class == other.m_class && this->m_func == other.m_func;
    }
    return_t operator   ()(args_t... args)
    {
        __any_class*    typed_class = reinterpret_cast<__any_class*>(m_class);
        func_t           typed_func  = reinterpret_cast<func_t>(m_func);
        return (typed_class->*typed_func)(args...);
    }

private:
    void*               m_class  = nullptr;
    class_function_ptr  m_func    = nullptr;
}; // class any_class_function


template <typename return_t, typename... args_t>
class caller_function
{
public:
    typedef return_t    (*func_t)(void*, args_t...);

    caller_function     () { }
    caller_function     (void* _caller, func_t _func) : m_caller(_caller), m_func(_func) { }
    void set            (void* _caller, func_t _func) { m_caller = _class; m_func = _func; }
    return_t operator   ()(args_t... args)
    {
        return (m_func)(m_caller, args...);
    }

private:
    void*   m_caller  = nullptr;
    func_t  m_func    = nullptr;

}; // class caller_function


} // namespace scl


