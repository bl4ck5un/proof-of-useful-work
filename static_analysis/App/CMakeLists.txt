set(SOURCE_FILES
        App.cpp
        VerifierEnclave_u.c
        ocalls.cpp
        Utils.cpp
        ../Common/Debug.c
        ../Common/Log.c
        xmlparser.cpp
        tinyxml/tinystr.cpp
        tinyxml/tinyxml.cpp
        tinyxml/tinyxmlerror.cpp
        tinyxml/tinyxmlparser.cpp
        )

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/VerifierEnclave_u.c
        COMMAND ${SGX_EDGER8R} --untrusted ${CMAKE_CURRENT_SOURCE_DIR}/../Enclave/VerifierEnclave.edl
        --search-path ${SGX_SDK}/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)


add_executable(verifier ${SOURCE_FILES})

if (NOT SGX_MODE STREQUAL HW)
    set(Urts_Library_Name sgx_urts_sim)
else()
    set(Urts_Library_Name sgx_urts)
endif()

include_directories(
        ../Common
        ${SGX_SDK}/include
        tinyxml
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

target_link_libraries(verifier
        -L${SGX_LIBRARY_PATH} -l${Urts_Library_Name} -l${UAE_SERVICE_LIB}
        -lpthread
        -lboost_system
        -lboost_filesystem
        -lboost_program_options
        )

add_dependencies(verifier signed.verifier.enclave)

install(TARGETS verifier DESTINATION ${PROJECT_BINARY_DIR})
