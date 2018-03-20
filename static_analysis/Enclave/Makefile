include ../Makefile.defs

Enclave_Name := verifier.so
Signed_Enclave_Name := verifier.signed.so
Enclave_Config_File := Enclave.config.xml

ENCLAVE_TRUSTED_STUB = VerifierEnclave_t.c 
ENCLAVE_EDL_FILE = VerifierEnclave.edl

Enclave_C_Files := \
	$(wildcard *.c) \
	$(wildcard ../Common/*.c) \
	$(wildcard distorm/*.c)

Enclave_C_Objects := $(Enclave_C_Files:.c=.o)
Enclave_Cpp_Files := $(wildcard *.cpp) \
	 				 $(wildcard ../Common/*.cpp) \
					 $(wildcard parser/*.cpp)
Enclave_Cpp_Objects := $(Enclave_Cpp_Files:.cpp=.o)
Enclave_Objects := $(Enclave_C_Objects) $(Enclave_Cpp_Objects) $(ENCLAVE_TRUSTED_STUB:.c=.o)

Enclave_Include_Dir = -I./common/inc \
					  -I./common/inc/internal\
					  -I./parser \
					  -I./distorm \
					  -I./crypto_px/include \
					  -I../Common

Enclave_C_Flags += $(Enclave_Include_Dir)
Enclave_Cpp_Flags += $(Enclave_Include_Dir)

.PHONY: all

all: $(Signed_Enclave_Name)

$(ENCLAVE_TRUSTED_STUB): $(SGX_EDGER8R) $(ENCLAVE_EDL_FILE)
	@$(SGX_EDGER8R) --trusted $(ENCLAVE_EDL_FILE) \
			--search-path . \
			--search-path $(SGX_SDK)/include
	@echo "GEN => $@"

$(ENCLAVE_TRUSTED_STUB:.c=.o): $(ENCLAVE_TRUSTED_STUB)
	@$(CC) $(Enclave_C_Flags) -c $< -o $@
	@echo "CC <= $<"

%.o: %.cpp
	@$(CXX) -c $(Enclave_Cpp_Flags) $^ -o $@
	@echo "CXX <= $<"

%.o: %.c
	@$(CC) -c $(Enclave_C_Flags) $^ -o $@
	@echo "CC <= $<"

$(Enclave_Name): $(Enclave_Objects)
	@$(CXX) $^ -o $@ $(Enclave_Link_Flags) 
	@echo "LD => $@" 

$(Signed_Enclave_Name): $(Enclave_Name)
	@$(SGX_ENCLAVE_SIGNER) sign -key Enclave_private.pem \
		-enclave $(Enclave_Name) \
		-config $(Enclave_Config_File) \
		-out $@ 
	@echo "SIGN =>  $@"

clean: 
	@rm -rf *.o *.so *.S
