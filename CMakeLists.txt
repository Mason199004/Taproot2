cmake_minimum_required(VERSION 3.26)
project(Taproot2 C)

set(CMAKE_C_STANDARD 23)

add_library(Taproot2 Taproot.c
        Taproot.h)
