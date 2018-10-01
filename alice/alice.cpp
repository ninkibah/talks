#include <map>
#include <functional>


struct Person {
  std::string firstName;
  std::string lastName;
  int age;
  int birthYear() const;
  std::string fullName() const;
  void setAge(int age);
};

template<typename>
struct TypeExtractor;

template<typename Key, typename Model>
struct TypeExtractor<Key Model::*> {
  using Model_t = Model;
  using Key_t = std::decay_t<
    std::invoke_result_t<Key Model::*, Model>
  >;
  //using Key_t = std::conditional_t<
  //  std::is_function_v<Key>,
  //  std::invoke_result_t<Key Model::*, Model>,
  //  Key>;
};

template <typename Key, typename Model>
struct TypeExtractor<Key (*)(Model const&)> {
  using Model_t = Model;
  using Key_t = Key;
};

template<auto...>
struct model_type;

template<auto V1>
struct model_type<V1> {
  static constexpr bool value = true;
  using type = typename TypeExtractor<decltype(V1)>::Model_t;
};

template<auto V1, auto... Vn>
struct model_type<V1, Vn...> {
  static constexpr bool value =
      (std::is_same_v<TypeExtractor<decltype(V1)>::Model_t, TypeExtractor<decltype(Vn)>::Model_t> && ...);
  using type = typename TypeExtractor<decltype(V1)>::Model_t;
};

template<auto...>
struct return_type;

template<auto V1>
struct return_type<V1> {
  using type = typename TypeExtractor<decltype(V1)>::Key_t;
};

template<auto V1, auto... Vn>
struct return_type<V1, Vn...> {
  using type = std::tuple<typename TypeExtractor<decltype(V1)>::Key_t,
    typename TypeExtractor<decltype(Vn)>::Key_t...>;
};

template<auto... pMembers>
using Key_t = typename return_type<pMembers...>::type;

template<auto... pMembers>
using Model_t = typename model_type<pMembers...>::type;

template<auto... pMembers>
struct Index {
  using Key = Key_t<pMembers...>;
  using Model = Model_t<pMembers...>;
  std::map<Key, int> data;

  static Key extractKey(Model const& model) {
    if constexpr (sizeof...(pMembers) == 1)  
      return std::invoke(pMembers..., model);
    else
      return std::make_tuple(std::invoke(pMembers, model)...);  
  }
};

struct PersonIndexByAge : public Index<&Person::age> {};
static_assert(std::is_same_v<Person, Index<&Person::age>::Model>);
static_assert(std::is_same_v<int, Index<&Person::age>::Key>);

