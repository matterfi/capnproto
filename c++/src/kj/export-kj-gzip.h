#pragma once

#ifdef KJ_GZIP_STATIC_DEFINE
#  define KJ_GZIP_API
#  define KJ_GZIP_CLASS
#else
#  ifndef KJ_GZIP_API
#    ifdef _WIN32
#      ifdef KJ_GZIP_BUILD
#        define KJ_GZIP_API __declspec(dllexport)
#      else
#        define KJ_GZIP_API __declspec(dllimport)
#      endif
#      define KJ_GZIP_CLASS
#    else
#      define KJ_GZIP_API   __attribute__((__visibility__("default")))
#      define KJ_GZIP_CLASS __attribute__((__visibility__("default")))
#    endif
#  endif
#endif
