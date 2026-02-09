#pragma once

#ifdef CAPNP_CAPNPC_STATIC_DEFINE
#  define CAPNP_CAPNPC_API
#  define CAPNP_CAPNPC_CLASS
#else
#  ifndef CAPNP_CAPNPC_API
#    ifdef _WIN32
#      ifdef CAPNP_CAPNPC_BUILD
#        define CAPNP_CAPNPC_API __declspec(dllexport)
#      else
#        define CAPNP_CAPNPC_API __declspec(dllimport)
#      endif
#      define CAPNP_CAPNPC_CLASS
#    else
#      define CAPNP_CAPNPC_API   __attribute__((__visibility__("default")))
#      define CAPNP_CAPNPC_CLASS __attribute__((__visibility__("default")))
#    endif
#  endif
#endif
