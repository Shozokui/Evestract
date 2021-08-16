cmake_minimum_required(VERSION 3.10)

project(Evestract)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)


add_executable(Evestract src/main.c src/dialog.c src/event.c src/npc.c src/script.c)
