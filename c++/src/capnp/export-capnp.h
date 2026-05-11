#pragma once

#ifdef CAPNP_STATIC_DEFINE
#  define CAPNP_API
#  define CAPNP_CLASS
#else
#  ifndef CAPNP_API
#    ifdef _WIN32
#      ifdef CAPNP_BUILD
#        define CAPNP_API __declspec(dllexport)
#      else
#        define CAPNP_API __declspec(dllimport)
#      endif
#      define CAPNP_CLASS
#    else
#      define CAPNP_API   __attribute__((__visibility__("default")))
#      define CAPNP_CLASS __attribute__((__visibility__("default")))
#    endif
#  endif
#endif
