cmake_minimum_required(VERSION 3.0)

project(PoUW)

set(SGX_SDK /opt/intel/sgxsdk)
set(SGX_MODE HW)
set(SGX_ARCH x64)
set(SGX_DEBUG 1)

set(POUW_ROOT_DIR ${CMAKE_SOURCE_DIR})

set(CMAKE_VERBOSE_MAKEFILE FALSE)

execute_process(COMMAND getconf LONG_BIT OUTPUT_VARIABLE VAR_LONG_BIT)
if (VAR_LONG_BIT STREQUAL 32)
    set(SGX_ARCH x86)
elseif(CMAKE_CXX_FLAGS MATCHES -m32)
    set(SGX_ARCH x86)
endif()


if (SGX_ARCH STREQUAL x86)
    set(SGX_COMMON_CFLAGS -m32)
    set(SGX_LIBRARY_PATH ${SGX_SDK}/lib)
    set(SGX_ENCLAVE_SIGNER ${SGX_SDK}/bin/x86/sgx_sign)
    set(SGX_EDGER8R ${SGX_SDK}/bin/x86/sgx_edger8r)
else()
    set(SGX_COMMON_CFLAGS -m64)
    set(SGX_LIBRARY_PATH ${SGX_SDK}/lib64)
    set(SGX_ENCLAVE_SIGNER ${SGX_SDK}/bin/x64/sgx_sign)
    set(SGX_EDGER8R ${SGX_SDK}/bin/x64/sgx_edger8r)
endif()

if (SGX_DEBUG STREQUAL 1 AND SGX_PRERELEASE STREQUAL 1)
    message(error Cannot set SGX_DEBUG and SGX_PRERELEASE at the same time!!)
endif()

if (SGX_DEBUG EQUAL 1)
    set(SGX_COMMON_CFLAGS "${SGX_COMMON_CFLAGS} -O0 -g")
else()
    set(SGX_COMMON_CFLAGS "${SGX_COMMON_CFLAGS} -O2")
endif()

message(STATUS "SGX_COMMON_CFLAGS: ${SGX_COMMON_CFLAGS}")
message(STATUS "SGX_SDK: ${SGX_SDK}")
message(STATUS "SGX_ARCH: ${SGX_ARCH}")
message(STATUS "SGX_MODE: ${SGX_MODE}")
message(STATUS "SGX_LIBRARY_PATH: ${SGX_LIBRARY_PATH}")
message(STATUS "SGX_ENCLAVE_SIGNER: ${SGX_ENCLAVE_SIGNER}")
message(STATUS "SGX_EDGER8R: ${SGX_EDGER8R}")

add_subdirectory(Enclave)

set(SOURCE_FILES
    test.cpp
    pouw.cpp
    Enclave_u.c
    ocalls.cpp
    Utils.cpp
    base64.cpp
    Common/Debug.c
    Common/Log.c
)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_u.c
        COMMAND ${SGX_EDGER8R} --untrusted ${POUW_ROOT_DIR}/Enclave/Enclave.edl
                               --search-path ${POUW_ROOT_DIR}/Enclave
                               --search-path ${SGX_SDK}/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)


add_executable(pouw-test ${SOURCE_FILES})

if (NOT SGX_MODE STREQUAL HW)
    set(Urts_Library_Name sgx_urts_sim)
else()
    set(Urts_Library_Name sgx_urts)
endif()

include_directories(
    Common
    ${SGX_SDK}/include
)


if(SGX_DEBUG STREQUAL 1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG -UNDEBUG -UEDEBUG")
elseif(SGX_PRERELEASE STREQUAL 1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -UDEBUG -DNDEBUG -DEDEBUG")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -UDEBUG -DNDEBUG -UEDEBUG")
endif()

if(SGX_MODE STREQUAL HW)
    set(UAE_SERVICE_LIB sgx_uae_service)
else()
    set(UAE_SERVICE_LIB sgx_uae_service_sim)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SGX_COMMON_CFLAGS} -fPIC -Wno-attributes")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++11")

target_link_libraries(pouw-test
    -L${SGX_LIBRARY_PATH} -l${Urts_Library_Name} -l${UAE_SERVICE_LIB})

add_dependencies(pouw-test signed.enclave)
