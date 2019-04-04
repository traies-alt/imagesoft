//
// Created by juanfra on 04/04/19.
//

#ifndef IMAGESOFT_FILESYSTEMADAPTER_H
#define IMAGESOFT_FILESYSTEMADAPTER_H

#ifdef _WIN32

#include <filesystem>

namespace fs = std::filesystem;


#elif defined __unix__

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

#endif

#endif //IMAGESOFT_FILESYSTEMADAPTER_H
