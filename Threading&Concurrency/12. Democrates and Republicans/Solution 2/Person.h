#pragma once
#include <string>

struct Person {
    std::string name;
    long millis;

    Person(const std::string& n, long m) : name(n), millis(m) {}
};
