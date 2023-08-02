cmake_minimum_required(VERSION 3.22)

add_executable(decoder main.c alsa.c wave.c)

target_include_directories (decoder PUBLIC ${CMAKE_HOME_DIRECTORY}/include)
target_include_directories (decoder PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_directories(decoder PRIVATE ${CMAKE_HOME_DIRECTORY}/platforms/${PLATFORM_OPTION}/external/fftw/lib)
target_link_directories(decoder PRIVATE ${CMAKE_HOME_DIRECTORY}/platforms/${PLATFORM_OPTION}/prebuilt/lib/GCC)

if(${PLATFORM_OPTION} STREQUAL "aarch64")
    target_include_directories (decoder PUBLIC ${CMAKE_HOME_DIRECTORY}/platforms/${PLATFORM_OPTION}/external/sndfile/include)
    target_include_directories (decoder PUBLIC ${CMAKE_HOME_DIRECTORY}/platforms/${PLATFORM_OPTION}/external/alsa_lib/include)
    target_link_directories(decoder PRIVATE ${CMAKE_HOME_DIRECTORY}/platforms/${PLATFORM_OPTION}/external/sndfile/lib)
    target_link_directories(decoder PRIVATE ${CMAKE_HOME_DIRECTORY}/platforms/${PLATFORM_OPTION}/external/alsa_lib/lib)
endif()

target_link_libraries(decoder trill_core_sdk fftw3f sndfile asound m dl pthread)

install(TARGETS decoder DESTINATION tools/decoder)
