#ifndef LIBCOVE_UTILS_TOOLS_H_
#define LIBCOVE_UTILS_TOOLS_H_

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type, member) );})

#endif