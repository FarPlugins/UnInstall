project(UnInstall CXX)

set(SRC_CPP
    UnInstall.cpp
    UnInstall.hpp
    FarLang.h
    guid.hpp
    version.hpp
    UnInstall.rc)

set(SRC_DEF UnInstall.def)

#====================================================
# common flags
#====================================================
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  # default for msvc project
  # CMAKE_CXX_FLAGS= /DWIN32 /D_WINDOWS /GR /EHsc
  # CMAKE_CXX_FLAGS_DEBUG= /MDd /Zi /Ob0 /Od /RTC1
  # CMAKE_CXX_FLAGS_RELEASE= /MD /O2 /Ob2 /DNDEBUG

  if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
  else ()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
  endif ()

  set(FLAGS_CXX_DEFAULT /Zi)
  set(FLAGS_CXX_RELEASE /W3 /Oi /Ot /GL /GS-)
  set(FLAGS_CXX_DEBUG /W4 /GS)

  set(LINK_FLAGS "/MAP /MANIFEST:NO")

  # build with debug info in pdb
  if (CMAKE_BUILD_TYPE MATCHES "Release")
    set(LINK_FLAGS "${LINK_FLAGS} /incremental:no /OPT:REF /OPT:ICF /debug /ltcg")
  endif ()

  add_definitions(-DUNICODE -D_UNICODE)

  if (IS_x64_PLATFORM)
    set(CMAKE_RC_FLAGS -D_WIN64)
  else ()
    # support old CPU AMD on WinXP
    set(FLAGS_CXX_DEFAULT ${FLAGS_CXX_DEFAULT} /arch:IA32)
    set(FLAGS_C_DEFAULT ${FLAGS_C_DEFAULT} /arch:IA32)
  endif ()

  # for vc_crt_fix_impl.cpp
  set(FLAGS_CXX_DEFAULT ${FLAGS_CXX_DEFAULT} /Zc:threadSafeInit-)
  set(FLAGS_C_DEFAULT ${FLAGS_C_DEFAULT} /Zc:threadSafeInit-)

endif ()

# set options for target
set(MAIN_FLAGS_CXX_DEBUG ${FLAGS_CXX_DEBUG} ${FLAGS_CXX_DEFAULT})
set(MAIN_FLAGS_CXX_RELEASE ${FLAGS_CXX_RELEASE} ${FLAGS_CXX_DEFAULT})
set(MAIN_FLAGS_C_DEBUG ${FLAGS_C_DEBUG} ${FLAGS_C_DEFAULT})
set(MAIN_FLAGS_C_RELEASE ${FLAGS_C_RELEASE} ${FLAGS_C_DEFAULT})

#====================================================
# support win2k, winxp
#====================================================
if (MSVC)
  set(ASM_SOURCE ${PROJECT_SOURCE_DIR}/../farsdk/vc_crt_fix.asm)
  set(ASM_OBJECT ${CMAKE_CURRENT_BINARY_DIR}/vc_crt_fix.obj)
  set(ASM_OBJECTS ${ASM_OBJECTS} ${ASM_OBJECT})
  set(MASM_DEFINES "")
  if (${CMAKE_CL_64} STREQUAL "0")
    find_program(MASM_EXECUTABLE ml)
  else ()
    find_program(MASM_EXECUTABLE ml64)
    set(MASM_DEFINES "/DX64")
  endif ()

  add_custom_command(
      OUTPUT ${ASM_OBJECT}
      COMMAND ${MASM_EXECUTABLE}
      ARGS /c ${MASM_DEFINES} /Fo ${ASM_OBJECT} ${ASM_SOURCE}
      DEPENDS ${ASM_SOURCE}
  )

  set(SRC_CPP ${SRC_CPP}
      ${PROJECT_SOURCE_DIR}/../farsdk/vc_crt_fix_impl.cpp
      ${ASM_OBJECTS}
      )
endif ()

#====================================================
# build
#====================================================

set(SRC_FILES ${SRC_CPP} ${SRC_DEF})
add_library(UnInstall SHARED ${SRC_FILES})
target_include_directories(UnInstall
    PUBLIC ${PROJECT_SOURCE_DIR}/../farsdk/unicode
    )

set_target_properties(UnInstall PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )

if (${CMAKE_BUILD_TYPE} MATCHES Debug)
  target_compile_options(UnInstall
      PUBLIC $<$<COMPILE_LANGUAGE:C>:${MAIN_FLAGS_C_DEBUG}>
      PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${MAIN_FLAGS_CXX_DEBUG}>
      )
else ()
  target_compile_options(UnInstall
      PUBLIC $<$<COMPILE_LANGUAGE:C>:${MAIN_FLAGS_C_RELEASE}>
      PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${MAIN_FLAGS_CXX_RELEASE}>
      )
endif ()

set_target_properties(UnInstall
    PROPERTIES
    LINK_FLAGS "${LINK_FLAGS}"
    )

#====================================================
# build misc files
#====================================================
configure_file(File_ID.diz File_ID.diz)
configure_file(UnInstall_Eng.hlf UnInstall_Eng.hlf)
configure_file(UnInstall_Rus.hlf UnInstall_Rus.hlf)

#====================================================
# install
#====================================================
install(TARGETS UnInstall RUNTIME DESTINATION .)
install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.map DESTINATION .)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/UnInstall_Eng.hlf ${CMAKE_CURRENT_BINARY_DIR}/UnInstall_Rus.hlf UnInstall_Eng.lng UnInstall_Rus.lng DESTINATION .)
install(FILES ${PROJECT_SOURCE_DIR}/../WhatsNew.Rus.txt ${PROJECT_SOURCE_DIR}/../ReadMe.Rus.txt ${CMAKE_CURRENT_BINARY_DIR}/File_ID.diz DESTINATION .)