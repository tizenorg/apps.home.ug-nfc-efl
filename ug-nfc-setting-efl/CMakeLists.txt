CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(ug-setting-nfc-efl C)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR "\${prefix}/lib")
SET(INCLUDEDIR "\${prefix}/include")
SET(VERSION 0.1.0)
SET(RESDIR "${PREFIX}/res")
SET(LOCALEDIR "${RESDIR}/locale")
SET(EDJDIR "${RESDIR}/edje/${PROJECT_NAME}")
SET(ICONDIR "${RESDIR}/icons/${PROJECT_NAME}")

ADD_DEFINITIONS("-DPACKAGE=\"ug-setting-nfc-efl\"")
ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")
ADD_DEFINITIONS("-DEDJDIR=\"${EDJDIR}\"")

SET(SRCS
src/ug-nfc-setting-main.c
src/ug-nfc-setting-predefined-item.c
src/ug-nfc-setting-popup.c
src/ug-nfc-setting-db.c
)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs_ug_nfc_setting REQUIRED
ecore-x
ecore-evas
elementary
ui-gadget-1
dlog
vconf
capi-appfw-application
capi-network-nfc
sqlite3
)

FOREACH(flag ${pkgs_ug_nfc_setting_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror-implicit-function-declaration")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_C_FLAGS_RELEASE "-O2")

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" STREQUAL "arm")
	ADD_DEFINITIONS("-DTARGET")
	MESSAGE("add -DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DEDJDIR=\"${EDJDIR}\"")
ADD_DEFINITIONS("-DRESDIR=\"${RESDIR}\"")

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRCS})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION 0.1.0)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_ug_nfc_setting_LDFLAGS})

ADD_CUSTOM_TARGET(ug-setting-nfc-efl.edj
                COMMAND edje_cc
                ${CMAKE_CURRENT_SOURCE_DIR}/data/edc/ug-setting-nfc-efl.edc ${CMAKE_BINARY_DIR}/ug-setting-nfc-efl.edj
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/data/edc/ug-setting-nfc-efl.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} ug-setting-nfc-efl.edj)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${LIBDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/ug-setting-nfc-efl.edj DESTINATION ${EDJDIR})

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/icons/setting-nfc.png DESTINATION ${ICONDIR})

# install smack file
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ug.setting-nfc-efl.include DESTINATION /etc/smack/accesses2.d)

# install xml file
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/sysinfo-nfc-ug.xml DESTINATION /etc/config/nfc/)

ADD_SUBDIRECTORY(po)
