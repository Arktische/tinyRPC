#pragma once
#include <common/trait.hpp>
template<typename F>
class RemoteProcedureCall {
    common::
};

#define rpc(request,response,handler)\
template<>\
class RemoteProcedureCall<request,response,handler>{\
\
}\