cmake_minimum_required(VERSION 3.10)

project(Evestract)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)


add_executable(Evestract src/main.c src/readfile.c src/ftable.c src/datid.c src/text.c src/dialog.c src/event.c src/npc.c src/script.c src/firstpass.c)
add_executable(Weather src/weather.c src/readfile.c src/ftable.c src/datid.c)
add_executable(npc src/npcmain.c src/readfile.c src/ftable.c src/datid.c src/text.c src/npc.c)
add_executable(dialog src/dialogmain.c src/readfile.c src/ftable.c src/datid.c src/text.c src/dialog.c)
add_executable(ftable src/ftablemain.c src/readfile.c src/ftable.c)
add_executable(pivotable src/pivotable.c src/readfile.c src/ftable.c src/datid.c)
