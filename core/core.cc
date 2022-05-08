#include "queue.hpp"
#include "coroutine_pool.hpp"

class A {
    public:
    A() = default;
    A(int a):a_(a){}

    int a_;
};

void test() {
    A a(1);
    core::ThreadSafeQueue<A> q;
    q.push(a);
    q.push(A(88));
}