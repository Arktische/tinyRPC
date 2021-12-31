//
// Created by tyx on 12/23/21.
//

#ifndef TINYRPC_DEFER_HPP
#define TINYRPC_DEFER_HPP
#include <functional>
namespace common {
#define CONCAT_(a, b) a ## b
#define CONCAT(a, b) CONCAT_(a,b)
#define defer(fn) Defer CONCAT(__defer__, __LINE__) = [&] ( ) { fn ; }
    class Defer {
    public:
        template<class Func>
        Defer(Func &&fn) : fn_(std::forward<Func>(fn)){}
        Defer(Defer &&other) noexcept : fn_(std::move(other.fn_))  {
            other.fn_ = nullptr;
        }
        ~Defer() {
            if(fn_) fn_();
        }

        Defer(const Defer &) = delete;
        void operator=(const Defer &) = delete;
    private:
        std::function<void()> fn_;
    };
}
#endif //TINYRPC_DEFER_HPP
