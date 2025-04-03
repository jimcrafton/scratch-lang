#pragma once

#define VERSION_MAJOR  1
#define VERSION_MINOR  0
#define VERSION_RELEASE  0
#define VERSION_BUILD  12



#define vers_str_(x) #x
#define vers_str(x) vers_str_(x)

#define VERSION_STR  vers_str(VERSION_MAJOR)"."vers_str(VERSION_MINOR)"."vers_str(VERSION_RELEASE)"."vers_str(VERSION_BUILD)