#pragma once

#ifdef CAPNP_JSON_STATIC_DEFINE
#  define CAPNP_JSON_API
#  define CAPNP_JSON_CLASS
#else
#  ifndef CAPNP_JSON_API
#    ifdef _WIN32
#      ifdef CAPNP_JSON_BUILD
#        define CAPNP_JSON_API __declspec(dllexport)
#      else
#        define CAPNP_JSON_API __declspec(dllimport)
#      endif
#      define CAPNP_JSON_CLASS
#    else
#      define CAPNP_JSON_API   __attribute__((__visibility__("default")))
#      define CAPNP_JSON_CLASS __attribute__((__visibility__("default")))
#    endif
#  endif
#endif
