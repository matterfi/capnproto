#pragma once

#ifdef CAPNP_WEBSOCKET_STATIC_DEFINE
#  define CAPNP_WEBSOCKET_API
#  define CAPNP_WEBSOCKET_CLASS
#else
#  ifndef CAPNP_WEBSOCKET_API
#    ifdef _WIN32
#      ifdef CAPNP_WEBSOCKET_BUILD
#        define CAPNP_WEBSOCKET_API __declspec(dllexport)
#      else
#        define CAPNP_WEBSOCKET_API __declspec(dllimport)
#      endif
#      define CAPNP_WEBSOCKET_CLASS
#    else
#      define CAPNP_WEBSOCKET_API   __attribute__((__visibility__("default")))
#      define CAPNP_WEBSOCKET_CLASS __attribute__((__visibility__("default")))
#    endif
#  endif
#endif
