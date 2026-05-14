/**
 * pugixml parser - version 1.14
 * --------------------------------------------------------
 * Copyright (C) 2006-2023, by Arseny Kapoulkine (arseny.kapoulkine@gmail.com)
 * Report bugs and download new versions at https://pugixml.org/
 *
 * This library is distributed under the MIT License. See notice at the end
 * of this file.
 *
 * This work is based on the pugxml parser, which is:
 * Copyright (C) 2003, by Kristen Wegner (kristen@tima.net)
 */

#ifndef HEADER_PUGICONFIG_HPP
#define HEADER_PUGICONFIG_HPP

// Uncomment this to enable wchar_t mode
// #define PUGIXML_WCHAR_MODE

// Uncomment this to enable compact mode
// #define PUGIXML_COMPACT

// Uncomment this to disable XPath
// #define PUGIXML_NO_XPATH

// Uncomment this to disable STL
// #define PUGIXML_NO_STL

// Uncomment this to disable exceptions
// #define PUGIXML_NO_EXCEPTIONS

// Set this to control the maximum parsing depth to prevent stack overflow
#ifndef PUGIXML_MAX_DEPTH
#   define PUGIXML_MAX_DEPTH 1000
#endif

// Set this to control the maximum number of attributes per node
#ifndef PUGIXML_MAX_ATTRIBUTES
#   define PUGIXML_MAX_ATTRIBUTES 10000
#endif

// Set this to control the alignment of automatically allocated memory
#ifndef PUGIXML_MEMORY_ALIGNMENT
#   define PUGIXML_MEMORY_ALIGNMENT 4
#endif

// Set this to override the default memory management functions
// #define PUGIXML_MEMORY_ALLOCATE(size) malloc(size)
// #define PUGIXML_MEMORY_DEALLOCATE(ptr) free(ptr)

// Set this to enable custom string type
// #define PUGIXML_CUSTOM_STRING_TYPE my_string

#endif
