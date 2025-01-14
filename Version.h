#ifndef __VERSION_H__
#define __VERSION_H__

#define VERSION_MAJOR    0
#define VERSION_MINOR    2
#define VERSION_BUILD    88

#define DEF_TO_STR_(X)    #X
#define DEF_TO_STR(X)     DEF_TO_STR_(X)
#define SW_VERSION_STR    DEF_TO_STR(VERSION_MAJOR) + "." \
                          DEF_TO_STR(VERSION_MINOR) + "." \
                          DEF_TO_STR(VERSION_BUILD)

#endif
