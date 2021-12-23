//
// Created by tyx on 12/23/21.
//

#ifndef TINYRPC_NON_COPYABLE_H
#define TINYRPC_NON_COPYABLE_H
class NonCopyable {
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};
#endif //TINYRPC_NON_COPYABLE_H
