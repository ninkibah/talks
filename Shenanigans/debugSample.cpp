#include <iostream>
#include <string>

template<int Version>
struct Person;

template<>
struct Person<0> {
  std::string firstName, lastName;
};

template<>
struct Person<1> {
  std::string firstName, lastName;
  int yob;
  Person<0> downgrade() const {
    return Person<0>{firstName, lastName};
  }
};

template<>
struct Person<2> {
  std::string firstName, lastName;
  int yob;
  std::string idNumber;

  Person<1> downgrade() const {
    return Person<1>{firstName, lastName, yob};
  }
};

template<typename>
constexpr inline bool always_false = false;

template<int Version>
auto downgrade(const Person<Version>& current) {
  if constexpr (requires { current.downgrade(); }) {
    return downgrade( current.downgrade() );
  } else if constexpr (Version == 0) {
    return current;
  } else {
    static_assert(always_false<Person<Version>>, "Please add a downgrade() member");
  }
}

int main() {
  Person<2> p2{"Jonathan", "O'Connor", 1963, "12341234ABCD"};
  auto person0 = downgrade(p2);
  std::cout << person0.firstName << ' ' << person0.lastName << '\n';
  return 0;
}