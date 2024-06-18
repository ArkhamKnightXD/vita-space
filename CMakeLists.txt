cmake_minimum_required(VERSION 3.2)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(space)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(SDL2_DIR ${VITASDK}/arm-vita-eabi/lib/cmake/SDL2/)
find_package(SDL2 REQUIRED)

set(VITA_APP_NAME "Vita Space")
set(VITA_TITLEID  "VSDK00050")
set(VITA_VERSION  "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(${PROJECT_NAME}
  src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
  SDL2
  SDL2_image
  png
  png16
  jpeg
  z
  SceGxm_stub
  SceCtrl_stub
  SceAudio_stub
  SceMotion_stub
  SceCommonDialog_stub
  SceIme_stub
  SceHid_stub
  SceTouch_stub
  SceDisplay_stub
  SceAudio_stub
  SceAudioIn_stub
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
  FILE sprites/alien_1.png sprites/alien_1.png 
  FILE sprites/alien_2.png sprites/alien_2.png 
  FILE sprites/alien_3.png sprites/alien_3.png 
  FILE sprites/mystery.png sprites/mystery.png 
  FILE sprites/spaceship.png sprites/spaceship.png 
  FILE sprites/structure.png sprites/structure.png 
)
