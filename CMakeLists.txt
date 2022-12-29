cmake_minimum_required(VERSION 3.22)

project(Evestract)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS True)

if(MSVC)
    add_compile_options(/W4 /wd4201 /wd4996 /wd4100)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -W)
endif()

set(BYTESLIB src/bytes/bytes.h)
set(BYTESDIR src/bytes)

set(RESOURCELIB ${BYTESLIB} src/resource/resource.c src/resource/resource.h)
set(RESOURCEDIR ${BYTESDIR} src/resource)

set(PARSERLIB ${RESOURCELIB} src/parser/parser.c
                             src/parser/parser.h
                             src/parser/parsers/bmp2.c
                             src/parser/parsers/bmp2.h
                             src/parser/parsers/generator.c
                             src/parser/parsers/generator.h
                             src/parser/parsers/meb.c
                             src/parser/parsers/meb.h
                             src/parser/parsers/mld.c
                             src/parser/parsers/mld.h
                             src/parser/parsers/msb.c
                             src/parser/parsers/msb.h
                             src/parser/parsers/scheduler.c
                             src/parser/parsers/scheduler.h)
set(PARSERDIR ${RESOURCEDIR} src/parser)

set(MD5LIB src/thirdparty/md5.c src/thirdparty/md5.h)
set(MD5DIR src/thirdparty)

set(PS2TEXTURELIB src/thirdparty/PS2Texture.c src/thirdparty/PS2Texture.h)
set(PS2TEXTUREDIR src/thirdparty)

add_executable(Evestract ${BYTESLIB} src/main.c src/readfile.c src/ftable.c src/datid.c src/text.c src/dialog.c src/event.c src/npc.c src/script.c src/firstpass.c)
target_include_directories(Evestract PRIVATE ${BYTESDIR})

add_executable(texstract ${PARSERLIB} ${MD5LIB} ${PS2TEXTURELIB}
    src/texstract/texstract.c
    src/texstract/bmp2.c
    src/texstract/bmp2.h
    src/texstract/mtx.c
    src/texstract/mtx.h
    src/texstract/tga.c
    src/texstract/tga.h
    src/readfile.c
    src/readfile.h)
target_include_directories(texstract PRIVATE ${PARSERDIR} ${MD5DIR} ${PS2TEXTUREDIR})

add_executable(mapstract ${PARSERLIB} ${MD5LIB}
    src/mapstract/mapstract.c
    src/readfile.c
    src/readfile.h)
target_include_directories(mapstract PRIVATE ${PARSERDIR} ${MD5DIR})

add_executable(schedgen ${PARSERLIB} src/schedgen.c src/readfile.c src/ftable.c)
target_include_directories(schedgen PRIVATE ${PARSERDIR})

add_executable(mebmsb ${PARSERLIB} src/mebmsb.c src/readfile.c src/ftable.c)
target_include_directories(mebmsb PRIVATE ${PARSERDIR})

add_executable(npc ${BYTESLIB} src/npcmain.c src/readfile.c src/ftable.c src/datid.c src/text.c src/npc.c)
target_include_directories(npc PRIVATE ${BYTESDIR})

add_executable(dialog ${BYTESLIB} src/dialogmain.c src/readfile.c src/ftable.c src/datid.c src/text.c src/dialog.c)
target_include_directories(dialog PRIVATE ${BYTESDIR})

add_executable(ftable ${BYTESLIB} src/ftablemain.c src/readfile.c src/ftable.c)
target_include_directories(ftable PRIVATE ${BYTESDIR})

add_executable(locresources ${BYTESLIB} src/utils/locresources.c src/readfile.c src/ftable.c src/datid.c)
target_include_directories(locresources PRIVATE ${BYTESDIR})

add_executable(pivotable ${BYTESLIB} src/pivotable.c src/readfile.c src/ftable.c src/datid.c)
target_include_directories(pivotable PRIVATE ${BYTESDIR})



###########

# etc.
add_executable(fontpatch ${PARSERLIB} src/experiments/fontpatch.c src/experiments/tga.c src/experiments/tga.h src/readfile.c src/ftable.c)
target_include_directories(fontpatch PRIVATE ${PARSERDIR})
