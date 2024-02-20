#pragma once

// template <class tClass>
// class is_reversible {
//     template <class tInnerClass, int dummy = (&tInnerClass::__reversed__, 0)>
//     static std::true_type check(tInnerClass*);
//     static std::false_type check(...);
//     static tClass* mClass;

//    public:
//     static const bool value = decltype(check(mClass))::value;
// }
