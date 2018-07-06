#ifndef DSTORM_HELPERS_MAKE_UNIQUE_HPP
#define DSTORM_HELPERS_MAKE_UNIQUE_HPP

#include <memory>

// This is a hack to provide std::make_unique functionality until C++14
// arrives. It can be safely replaced by `using std::make_unique`.
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#endif
