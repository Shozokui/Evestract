cmake_minimum_required(VERSION 3.22)

project(Evestract)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)

if(MSVC)
    add_compile_options(/W4 /wd4201 /wd4996 /wd4100)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -W)
endif()

add_executable(Evestract src/main.c src/readfile.c src/ftable.c src/datid.c src/text.c src/dialog.c src/event.c src/npc.c src/script.c src/firstpass.c)
add_executable(schedgen src/schedgen.c src/readfile.c src/ftable.c)
add_executable(npc src/npcmain.c src/readfile.c src/ftable.c src/datid.c src/text.c src/npc.c)
add_executable(dialog src/dialogmain.c src/readfile.c src/ftable.c src/datid.c src/text.c src/dialog.c)
add_executable(ftable src/ftablemain.c src/readfile.c src/ftable.c)

add_executable(pivotable src/pivotable.c src/readfile.c src/ftable.c src/datid.c)
