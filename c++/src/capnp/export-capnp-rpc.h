#pragma once

#ifdef CAPNP_RPC_STATIC_DEFINE
#  define CAPNP_RPC_API
#  define CAPNP_RPC_CLASS
#else
#  ifndef CAPNP_RPC_API
#    ifdef _WIN32
#      ifdef CAPNP_RPC_BUILD
#        define CAPNP_RPC_API __declspec(dllexport)
#      else
#        define CAPNP_RPC_API __declspec(dllimport)
#      endif
#      define CAPNP_RPC_CLASS
#    else
#      define CAPNP_RPC_API   __attribute__((__visibility__("default")))
#      define CAPNP_RPC_CLASS __attribute__((__visibility__("default")))
#    endif
#  endif
#endif
