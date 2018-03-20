set(SOURCE_FILES
        # App.cpp
        Benchmark.cpp
        Enclave_u.c
        ocalls.cpp
        Utils.cpp
        base64.cpp
        svm-predict.cpp
        svm_ocalls.cpp
        sha3_test.cpp
        ../Common/Debug.c
        ../Common/Log.c)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_u.c
        COMMAND ${SGX_EDGER8R} --untrusted ${POUW_ROOT_DIR}/Enclave/Enclave.edl
                               --search-path ${POUW_ROOT_DIR}/Enclave
                               --search-path ${SGX_SDK}/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(pouw ${SOURCE_FILES})

if (NOT SGX_MODE STREQUAL HW)
    set(Urts_Library_Name sgx_urts_sim)
else()
    set(Urts_Library_Name sgx_urts)
endif()

include_directories(
    ../Common
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

target_link_libraries(pouw
        -L${SGX_LIBRARY_PATH}
        -l${Urts_Library_Name} -l${UAE_SERVICE_LIB} -lpthread
        -lboost_program_options
        -lboost_system
        -lboost_filesystem
        )

add_dependencies(pouw signed.enclave)

install(TARGETS pouw DESTINATION ${PROJECT_BINARY_DIR})
