#include <protocol/archive.hpp>
#include <gtest/gtest.h>
#include <vector>

struct A {
    int a{2};
    char b{'c'};
    char* c{nullptr};
    std::vector<int> vec{1,2,3,4,5,5,6};
};

struct foo {
    unsigned char i0;
    unsigned int i1;
    unsigned short i2;
    unsigned long long i3;
    unsigned char ar[2];
    int q;
    std::size_t w;
    int* p1;
    const void* p2;
    int const**const**** p_crazy;
    const double d;
};

TEST(protocol_test, test_struct_schema) {
    
    foo f {10, 11, 12, 13, {14, 15}, 16, 17, 0, 0, 0, 30.0};
    std::cout << common::get<11>(f);
}

TEST(protocol_test, test_archive) {
    codec::CodecArchive encoder(std::cout);
    foo f {10, 11, 12, 13, {14, 15}, 16, 17, 0, 0, 0, 30.0};
    encoder<<f;
}