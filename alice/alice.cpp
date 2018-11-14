#include <map>
#include <functional>
#include <string>
#include <iostream>

/**
 * The Person struct is used in all the example code.
 */
struct Person {
  std::string firstName;
  std::string lastName;
  int age;

  int birthYear() const {
    return 2018 - age;
  }

  std::string fullName() const {
    return firstName + " " + lastName;
  }

  void setAge(int age_) {
    this->age = age_;
  }
};

// TypeExtractor needs to be forward declared, before it can be specialized
// It takes a single template parameter which is a type.
template<typename>
struct TypeExtractor;

// This specialization is used to match pointers to data members or member functions.
// I'm not sure why this works, as the template argument, Key Model::*, looks like a value to me, and not a type.
// However, changing the formward declaration from template<typename> to template<auto> just causes compiler errors.
template<typename Key, typename Model>
struct TypeExtractor<Key Model::*> {
  // Alias template to get hold of the Model type
  using Model_t = Model;
  // using Key_t = Key;
  // Alias template to get hold of the Key type
  using Key_t = std::decay_t<
    std::invoke_result_t<Key Model::*, Model>
  >;
  // Stack overflow Barry suggested the more complicated solution below
  // Daniel Frey suggested using std::decay to remove any consts, volatiles and references.
  //
  //using Key_t = std::conditional_t<
  //  std::is_function_v<Key>,
  //  std::invoke_result_t<Key Model::*, Model>,
  //  Key>;
};

// This specialization is used to match pointers to functions taking a Model const&..
// I should have tried writing this specialization first, as it is very simple.
template <typename Key, typename Model>
struct TypeExtractor<Key (*)(Model const&)> {
  // Alias template to get hold of the Model type
  using Model_t = Model;
  // Alias template to get hold of the Key type
  using Key_t = Key;
};

// Now we start implementing a variadic template version of TypeExtractor
// model_type is a variadic template taking 0 or more non-type template parameters
// As usual we forward declare the template type, before defining partial specializations
template<auto...>
struct model_type;

// If we needed to handle the special case with zero template arguments, we could write the following:
// template<>
// struct model_type<> {
//   static constexpr bool value = true;
//   using type = void;
// };

// Now the partial specialization for a single template argument
template<auto V1>
struct model_type<V1> {
  // We build on our single parameter TypeExtractor template.
  // As TypeExtractor expects a typename, we have to convert from
  // our non-type template argument to a type.
  // C++ provides decltype(expr) to do exactly that
  using type = typename TypeExtractor<decltype(V1)>::Model_t;
};

// Now the partial specialization for 1 or more template arguments.
// As we have already a partial specialization for 1 argument, this will only be chosen
// by the compiler when we give 2 or more arguments.
template<auto V1, auto... Vn>
struct model_type<V1, Vn...> {
  // For production code, we should check that the other values produce the same model type.
  using type = typename TypeExtractor<decltype(V1)>::Model_t;
};

// We use the same pattern for the variadic return_type template.
template<auto...>
struct return_type;

template<auto V1>
struct return_type<V1> {
  using type = typename TypeExtractor<decltype(V1)>::Key_t;
};

template<auto V1, auto... Vn>
struct return_type<V1, Vn...> {
  // The old way of implementing these variadic templates was to use recursion, and have extra specializations.
  // But template parameter pack expansions are much more efficient
  using type = std::tuple<typename TypeExtractor<decltype(V1)>::Key_t,
    typename TypeExtractor<decltype(Vn)>::Key_t...>;
};

// Alias template for making it easier to get the Key type.
template<auto... pMembers>
using Key_t = typename return_type<pMembers...>::type;

// Alias template for making it easier to get the Model type.
template<auto... pMembers>
using Model_t = typename model_type<pMembers...>::type;

// Now we can declare a variadic index class
template<auto... pMembers>
struct Index {
  // I love alias templates! Have you noticed yet?
  using Key = Key_t<pMembers...>;
  using Model = Model_t<pMembers...>;

  // data is a map from key to a table row id
  // The real implementation has methods to manipulate data.
  std::map<Key, int> data;

  static Key extractKey(Model const& model) {
    // sizeof...(pMembers) returns the size of the parameter pack
    // if the size is 1, we don't need a tuple.
    // This optimization is not strictly necessary, as the compiler optimizes away a single member tuple
    // But we are being kind to the machine
    if constexpr (sizeof...(pMembers) == 1)
      return std::invoke(pMembers..., model);
    else
      // The ... is a fold expression, and it expands arguments to make_tuple to be a call to std::invoke for each of the pMembers
      // Thus for 3 arguments:
      // return std::make_tuple(std::invoke(pMember0, model), std::invoke(pMember1, model), std::invoke(pMember2, model));
      return std::make_tuple(std::invoke(pMembers, model)...);  
  }
};

int main() {
  struct PersonIndexByAge : public Index<&Person::age> {};
  static_assert(std::is_same_v<Person, Index<&Person::age>::Model>);
  static_assert(std::is_same_v<int, Index<&Person::age>::Key>);

  using UniquePersonIndex = Index<&Person::firstName, &Person::lastName, &Person::age>;
  static_assert(std::is_same_v<std::tuple<std::string, std::string, int>, UniquePersonIndex::Key>);

  using MemberFunctionIndex = Index<&Person::fullName>;

  Person alice{"Alice", "Hargreaves", 2018 - 1852};

  std::cout << "Alice is now " << PersonIndexByAge::extractKey(alice) << " years old.\n";
  std::cout << "Alice's full name is " << MemberFunctionIndex::extractKey(alice) << "\n";
}