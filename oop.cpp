#include <exception>
#include <iostream>
#include <unordered_map>

namespace {
struct VirtualTable {
    public:
        using Method =  void(*)();
        VirtualTable(VirtualTable* ptr): parent(ptr) {}
        std::unordered_map<std::string, Method> method;
        VirtualTable* parent;
};

#define VIRTUAL_CLASS(Base)\
    struct Base {\
        static VirtualTable vt;\
        VirtualTable* vt_table = &vt;

#define END(Base)\
    };\
    VirtualTable Base::vt(nullptr);

#define DECLARE_METHOD(Base, methodName)\
    void __ ##Base## _  ##methodName## __() {\
        std::cout << #Base << " " << #methodName << std::endl;\
    }\
    int __initer_ ##Base## _ ##methodName## __() {\
        Base::vt.method[#methodName] = & __ ##Base## _  ##methodName## __;\
        return 0;\
    }\
    /*have to add additional field to initialize map*/\ 
    int __ ##Base## _ ##methodName## _init__ = __initer_ ##Base## _ ##methodName## __();

#define VIRTUAL_CALL(ptr, methodName)\
    {\
        VirtualTable* current = (ptr)->vt_table;\
        bool is_called = false;\
        while (current != nullptr) {\
            if (current->method.count(#methodName)) {\
                current->method[#methodName]();\
                is_called = true;\
                break;\
            }\
            current = current->parent;\
        }\
        if (!is_called) {\
            throw std::logic_error("can not find suitable method");\
        }\
    }

#define VIRTUAL_CLASS_DERIVED(Derived, Base)\
    struct Derived: Base {\
        static VirtualTable vt;\
        Derived() {vt_table = &vt;}

#define END_DERIVE(Derived, Base)\
    };\
    VirtualTable Derived::vt(Base::vt);


} // namespace

VIRTUAL_CLASS( Base )
	int a;
END( Base )

DECLARE_METHOD( Base, Both )
DECLARE_METHOD( Base, OnlyBase )

VIRTUAL_CLASS_DERIVED( Derived, Base )
	int b;
END_DERIVE( Derived, Base )

DECLARE_METHOD( Derived, Both )
DECLARE_METHOD( Derived, OnlyDerived )

int main() {
    Base base;
    base.a = 0;
    Derived derived;

    Base* reallyDerived = reinterpret_cast<Base*>(&derived);
    VIRTUAL_CALL(&base, Both);
    VIRTUAL_CALL(reallyDerived, Both);
    VIRTUAL_CALL(reallyDerived, OnlyBase);
    VIRTUAL_CALL(reallyDerived, OnlyDerived);

    try {
        VIRTUAL_CALL(&base, UnexpectedMethod);
    } catch (std::exception& exc) {
        std::cout << "method expectedly not found" << std::endl;
    }
    try {
        VIRTUAL_CALL(reallyDerived, UnexpectedMethod);
    } catch (std::exception& exc) {
        std::cout << "method expectedly not found" << std::endl;
    }
}
