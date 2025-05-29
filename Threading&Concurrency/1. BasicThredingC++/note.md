- Ask chatgpt:
  - does thread in c++ accepts lambda only? No! Following are accepted when creating thread

| Callable Type                      | Supported by `std::thread`? |
| ---------------------------------- | --------------------------- |
| Lambda                             | ✅ Yes                       |
| Regular function                   | ✅ Yes                       |
| Function with args                 | ✅ Yes                       |
| Functor (object with `operator()`) | ✅ Yes                       |
| Member function                    | ✅ Yes (with object ptr)     |
| `std::bind` result                 | ✅ Yes                       |
