set(SOURCE_FILES
        ../Common/Debug.c
        ../Common/Log.c
        Enclave_t.c
        keccak.c
        PoUW.cpp
        printf.c)

set(CMAKE_VERBOSE_MAKEFILE False)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_t.c
        COMMAND ${SGX_EDGER8R} --trusted ${POUW_ROOT_DIR}/Enclave/Enclave.edl
        --search-path ${POUW_ROOT_DIR}/Enclave
        --search-path ${SGX_SDK}/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(enclave SHARED ${SOURCE_FILES})

if (NOT SGX_MODE STREQUAL HW)
    set(TRTS_LIB_NAME sgx_trts_sim)
    set(SERVICE_LIB_NAME sgx_tservice_sim)
else()
    set(TRTS_LIB_NAME sgx_trts)
    set(SERVICE_LIB_NAME sgx_tservice)
endif()

set(CRYPTO_LIB_NAME sgx_tcrypto)

include_directories(
    ${POUW_ROOT_DIR}/Common
    ${SGX_SDK}/include
    ${SGX_SDK}/include/tlibc
    ${SGX_SDK}/include/stlport
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DENCLAVE_STD_ALT ${SGX_COMMON_CFLAGS} \
    -nostdinc -fvisibility=hidden -fpie -fstack-protector")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++03 -nostdinc++")

set_target_properties(enclave PROPERTIES PREFIX "")

target_link_libraries(enclave "${SGX_COMMON_CFLAGS} \
    -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L${SGX_LIBRARY_PATH} \
    -Wl,--whole-archive -l${TRTS_LIB_NAME} -Wl,--no-whole-archive \
    -Wl,--start-group -lsgx_tstdc -lsgx_tstdcxx -l${CRYPTO_LIB_NAME} -l${SERVICE_LIB_NAME} -Wl,--end-group \
    -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
    -Wl,-pie,-eenclave_entry -Wl,--export-dynamic \
    -Wl,--defsym,__ImageBase=0")

add_custom_target(signed.enclave
        ${SGX_ENCLAVE_SIGNER} sign -key ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_private.pem
        -enclave enclave.so
        -config ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.config.xml -out enclave.signed.so)

add_dependencies(signed.enclave enclave)

add_custom_command(TARGET signed.enclave
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${POUW_ROOT_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy enclave.signed.so ${POUW_ROOT_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy enclave.so ${POUW_ROOT_DIR}/bin
        COMMENT "Installing signed enclave files")
