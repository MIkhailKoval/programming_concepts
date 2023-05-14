#include <exception>
#include <iostream>
#include <unordered_map>
#include <cassert>
#include <functional>

namespace {

#define EnablingRTTIBase(Base)\
    struct Base {\
        virtual std::string toString() const {\
            return #Base;\
        };\
        bool __canCast__(const std::string& target) const {\
            std::cout << toString() << " vs "  << target << std::endl;\
            return #Base == target;\
        }

#define TYPEID(Class)\
    Class.__to_string__();

#define ClassEnd(Base)\
    };

#define EnablingRTTIDerived(Derived, Bases...)\
    struct Derived: Bases {\
        virtual std::string toString() const {\
            return #Derived;\
        };\
        template <typename Base, typename Next, typename... Other>\
        bool __canCast__(const std::string& target) {\
            if (this->Base::__canCast__(target)) {\
                return true;\
            }\
            return __canCast__<Next, Other...>(target);\
        }\
        template <typename Base>\
        bool __canCast__(const std::string& target) {\
            return this->Base::__canCast__(target);\
        }\
        virtual bool __canCast__(const std::string& target) {\
            std::cout << #Derived << " vs "  << target << std::endl;\
            if (target == #Derived) {\
                return true;\
            }\
            return __canCast__<Bases>(target);\
        }

#define DynamicCast(d, b, TargetType)\
    {\
        if (!d) {\
            b = nullptr;\
        }\
        std::string str_type = #TargetType;\
        if (d->__canCast__(str_type)) {\
            std::cout << "succesfully casted" << std::endl;\
            b = reinterpret_cast<TargetType*>(d);\
        } else {\
            std::cout << "can not cast to type " << str_type;\
            std::cout << std::endl;\ 
        }\
    }
} // namespace

EnablingRTTIBase(Employer)
    virtual std::string position() const {
        return "employer";
    }
ClassEnd(Employer)

EnablingRTTIDerived(HR, Employer)
    virtual std::string position() const {
        return "HR";
    }
ClassEnd(HR)

EnablingRTTIDerived(Proj_Manager, Employer)
    virtual std::string position() const {
        return "Proj_Manager";
    }
ClassEnd(Proj_Manager)

EnablingRTTIDerived(Programmer, Employer)
    virtual std::string position() const {
        return "Programmer";
    }
ClassEnd(Programmer)

EnablingRTTIDerived(Team_Leader, Programmer)
    virtual std::string position() const {
        return "Team_Leader";
    }
ClassEnd(Team_Leader)

//
EnablingRTTIBase(Base)
ClassEnd(Base)

EnablingRTTIBase(Another)
ClassEnd(Another)


// final example
EnablingRTTIBase(Man)
    virtual std::string name() const {
        return "man";
    }
ClassEnd(Man)

EnablingRTTIBase(Profession)
    virtual std::string name() const {
        return "profession";
    }
ClassEnd(Profession)

EnablingRTTIDerived(Worker, Man, Profession)
    virtual std::string name() const {
        return "worker";
    }
ClassEnd(Worker)

int main() {
    Base b;
    Another a;

    // simple tests
    assert(a.__canCast__(a.toString()) == true);
    assert(a.__canCast__(b.toString()) == false);
    assert(b.__canCast__(a.toString()) == false);

    Team_Leader team_leader;
    Programmer* programmer_ptr = nullptr;
    Proj_Manager* proj_manager_ptr = nullptr;
    HR* hr_ptr = nullptr;
    Employer* empl_ptr = nullptr;

    DynamicCast((&team_leader), proj_manager_ptr, Proj_Manager);
    assert (proj_manager_ptr == nullptr);

    DynamicCast((&team_leader), hr_ptr, HR)
    assert (hr_ptr == nullptr);

    DynamicCast((&team_leader), programmer_ptr, Programmer);
    assert (programmer_ptr != nullptr);
    assert (programmer_ptr->position() == "Team_Leader");

    DynamicCast((&team_leader), empl_ptr, Employer);
    assert (empl_ptr != nullptr);
    assert (empl_ptr->position() == "Team_Leader");

    Worker worker;
    Profession* pr = nullptr;
    Man* m = nullptr;
    Another* aa = nullptr;
    DynamicCast((&worker), pr, Profession);
    assert (pr != nullptr);
    assert (pr->name() == "worker");

    DynamicCast((&worker), m, Man);
    assert (pr != nullptr);
    assert (m->name() == "worker");

    DynamicCast((&worker), aa, Another);
    assert (aa == nullptr);

}
