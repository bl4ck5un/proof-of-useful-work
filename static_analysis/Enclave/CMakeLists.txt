set(SOURCE_FILES
        ../Common/Debug.c
        ../Common/Log.c
        printf.c
        VerifierEnclave_t.c
        disassembler.cpp
        parser/elfparser.cpp
        parser/parserfactory.cpp
        parser/section.cpp
        parser/enclave_creator_sign.cpp
        parser/loader.cpp
        parser/manage_metadata.cpp
        measure.cpp
        main.cpp
        )

FILE(GLOB DISTORM_SRC distorm/*.c)

set(SOURCE_FILES ${DISTORM_SRC} ${SOURCE_FILES})

set(CMAKE_VERBOSE_MAKEFILE False)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/VerifierEnclave_t.c
        COMMAND ${SGX_EDGER8R} --trusted ${CMAKE_CURRENT_SOURCE_DIR}/VerifierEnclave.edl
        --search-path ${CMAKE_CURRENT_SOURCE_DIR}
        --search-path ${SGX_SDK}/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(verifier.enclave SHARED ${SOURCE_FILES})

if (NOT SGX_MODE STREQUAL HW)
    set(TRTS_LIB_NAME sgx_trts_sim)
    set(SERVICE_LIB_NAME sgx_tservice_sim)
else()
    set(TRTS_LIB_NAME sgx_trts)
    set(SERVICE_LIB_NAME sgx_tservice)
endif()

set(CRYPTO_LIB_NAME sgx_tcrypto)

include_directories(
        ${SGX_SDK}/include
        ${SGX_SDK}/include/tlibc
        ${SGX_SDK}/include/stlport
        ../Common
        crypto_px/include
        common/inc
        common/inc/internal
        ../Common
        parser
        distorm
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DENCLAVE_STD_ALT ${SGX_COMMON_CFLAGS} \
    -nostdinc -fvisibility=hidden -fpie -fstack-protector")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++03 -nostdinc++")

set_target_properties(verifier.enclave PROPERTIES PREFIX "")

target_link_libraries(verifier.enclave "${SGX_COMMON_CFLAGS} \
    -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L${SGX_LIBRARY_PATH} \
    -Wl,--whole-archive -l${TRTS_LIB_NAME} -Wl,--no-whole-archive \
    -Wl,--start-group -lsgx_tstdc -lsgx_tstdcxx -l${CRYPTO_LIB_NAME} -l${SERVICE_LIB_NAME} -Wl,--end-group \
    -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
    -Wl,-pie,-eenclave_entry -Wl,--export-dynamic \
    -Wl,--defsym,__ImageBase=0")

add_custom_target(signed.verifier.enclave
        ${SGX_ENCLAVE_SIGNER} sign -key ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_private.pem
        -enclave ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/verifier.enclave.so
        -config ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.config.xml
        -out ${PROJECT_BINARY_DIR}/verifier.signed.so)

add_dependencies(signed.verifier.enclave verifier.enclave)
