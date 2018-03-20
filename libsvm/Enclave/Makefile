include ../Makefile.defs

Enclave_Name := enclave.so
Signed_Enclave_Name := enclave.signed.so
Enclave_Config_File := Enclave.config.xml

Enclave_C_Files := \
	$(wildcard *.c)\
	$(wildcard ../Common/*.c)\
	$(wildcard external/*.c)
Enclave_C_Objects := $(Enclave_C_Files:.c=.o)
Enclave_Cpp_Files := \
	$(wildcard *.cpp)\
	$(wildcard ../Common/*.cpp)\
	$(wildcard protein/*.cpp)

Enclave_Cpp_Objects := $(Enclave_Cpp_Files:.cpp=.o)

Enclave_Objects := $(Enclave_C_Objects) $(Enclave_Cpp_Objects)

ASM_REWRITER = python instrumentation.py
ASM_REWRITER = cat


.PHONY: all

all: $(Signed_Enclave_Name)

Enclave_t.c: $(SGX_EDGER8R) Enclave.edl
	@$(SGX_EDGER8R) --trusted Enclave.edl \
			--search-path . \
			--search-path $(SGX_SDK)/include
	@echo "GEN => $@"

Enclave_t.o: Enclave_t.c
	@$(CC) $(Enclave_C_Flags) -c $< -o $@
	@echo "CC <= $<"

stub.o: stub.c
	@$(CC) $(Enclave_C_Flags) -c $< -o $@
	@echo "CC <= $<"

%.o: %.cpp
	@$(CXX) -S $(Enclave_Cpp_Flags) -fverbose-asm $< -o $(@:.o=.S)
	@cp -f $(@:.o=.S) $(@:.o=.S).orig
	@${ASM_REWRITER} $(@:.o=.S).orig > $(@:.o=.S)
	@$(CXX) -c $(Enclave_Cpp_Flags) $(@:.o=.S) -o $@
	@echo "CXX* <= $<"

%.o: %.c
	@$(CC) -S $(Enclave_C_Flags) -fverbose-asm $< -o $(@:.o=.S)
	@cp -f $(@:.o=.S) $(@:.o=.S).orig
	@${ASM_REWRITER} $(@:.o=.S).orig > $(@:.o=.S)
	@$(CC) -c $(Enclave_C_Flags) $(@:.o=.S) -o $@
	@echo "CC* <= $<"

$(Enclave_Name): Enclave_t.o $(Enclave_Objects)
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
