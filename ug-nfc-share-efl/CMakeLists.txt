CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(ug-share-nfc-efl C)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR "\${prefix}/lib")
SET(INCLUDEDIR "\${prefix}/include")
SET(VERSION 0.1.0)
SET(RESDIR "${PREFIX}/res")
SET(LOCALEDIR "${RESDIR}/locale")
SET(EDJDIR "${RESDIR}/edje/${PROJECT_NAME}")
SET(IMAGEDIR "${RESDIR}/images/${PROJECT_NAME}")

SET(SRCS
src/ug-nfc-share-main.c
src/ug-nfc-share-tag.c
src/ug-nfc-share-common-util.c
src/ug-nfc-share-popup.c
)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs_ug_nfc_share REQUIRED
ecore-x
elementary
ui-gadget-1
dlog
vconf
status
capi-appfw-application
capi-network-nfc
capi-content-mime-type
)

FOREACH(flag ${pkgs_ug_nfc_share_CFLAGS})
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
ADD_DEFINITIONS("-DIMAGEDIR=\"${IMAGEDIR}\"")

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRCS})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION 0.1.0)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_ug_nfc_share_LDFLAGS})

ADD_CUSTOM_TARGET(ug-nfc-share-efl.edj
                COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/data/images
                ${CMAKE_CURRENT_SOURCE_DIR}/data/edc/ug-nfc-share-efl.edc ${CMAKE_BINARY_DIR}/ug-nfc-share-efl.edj
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/data/edc/ug-nfc-share-efl.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} ug-nfc-share-efl.edj)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /opt/ug/lib)
INSTALL(FILES ${CMAKE_BINARY_DIR}/ug-nfc-share-efl.edj DESTINATION ${EDJDIR})

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/images/ DESTINATION ${IMAGEDIR} FILES_MATCHING PATTERN "*.png")
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/images/ DESTINATION ${IMAGEDIR} FILES_MATCHING PATTERN "*.jpg")

ADD_SUBDIRECTORY(po)
