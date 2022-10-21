
namespace scl {
	
template <typename ClassT, typename ReturnT, typename... Args>
class class_function
{
public:
    typedef ReturnT (ClassT::*FuncT)(Args...);

    class_function() { }
    class_function(ClassT* _class, FuncT _func) : caller(_class), func(_func) { }
    void set(ClassT* _class, FuncT _func) { caller = _class; func = _func; }
    ReturnT operator()(Args... args)
    {
        return (caller->*func)(args...);
    }

private:
    ClassT*     caller  = nullptr;
    FuncT       func    = nullptr;

}; // class class_function

template <typename ReturnT, typename... Args>
class caller_function
{
public:
    typedef ReturnT (*FuncT)(void*, Args...);

    caller_function() { }
    caller_function(void* _caller, FuncT _func) : caller(_caller), func(_func) { }
    void set(void* _caller, FuncT _func) { caller = _class; func = _func; }
    ReturnT operator()(Args... args)
    {
        return (func)(caller, args...);
    }

private:
    void*   caller  = nullptr;
    FuncT   func    = nullptr;

}; // class caller_function


} // namespace scl


