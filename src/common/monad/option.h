#ifndef COMMON_MONAD_OPTION
#define COMMON_MONAD_OPTION

template<typename T>
class Option {
public:

private:
  static struct None {
    void* aligned = nullptr;
  } static_none;
};

#endif
