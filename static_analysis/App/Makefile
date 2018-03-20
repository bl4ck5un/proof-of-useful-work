include ../Makefile.defs

ifneq ($(SGX_MODE), HW)
	Urts_Library_Name := sgx_urts_sim
else
	Urts_Library_Name := sgx_urts
endif

App_Include_Paths := \
	-I../Common \
	-I./tinyxml \
	-I$(SGX_SDK)/include 

App_C_Flags := $(SGX_COMMON_CFLAGS) -fPIC -Wno-attributes $(App_Include_Paths)
App_Cpp_Flags := $(App_C_Flags) -std=c++11

App_C_Files := $(wildcard *.c)
App_Cpp_Files := App.cpp $(wildcard *.cpp) $(wildcard tinyxml/*.cpp)
APP_OBJS := $(App_Cpp_Files:.cpp=.o) $(App_C_Files:.c=.o)

COMMON_C_FILES := $(wildcard ../Common/*.c)
COMMON_CXX_FILES := $(wildcard ../Common/*.cpp)
COMMON_OBJS := $(COMMON_C_FILES:.c=.a.o) $(COMMON_CXX_FILES:.cpp=.a.o)



ifeq ($(SGX_DEBUG), 1)
        App_C_Flags += -DDEBUG -UNDEBUG -UEDEBUG
else ifeq ($(SGX_PRERELEASE), 1)
        App_C_Flags += -DNDEBUG -DEDEBUG -UDEBUG
else
        App_C_Flags += -DNDEBUG -UEDEBUG -UDEBUG
endif

App_Link_Flags := $(SGX_COMMON_CFLAGS) -L$(SGX_LIBRARY_PATH) -l$(Urts_Library_Name) \
	-lboost_system \
	-lboost_filesystem \
	-lboost_program_options \
	-lpthread 

ifneq ($(SGX_MODE), HW)
	App_Link_Flags += -lsgx_uae_service_sim
else
	App_Link_Flags += -lsgx_uae_service
endif

App_Name := verifier

.PHONY: all

all: $(App_Name)

VerifierEnclave_u.c: ../Enclave/VerifierEnclave.edl $(SGX_EDGER8R)
	@$(SGX_EDGER8R) --untrusted  ../Enclave/VerifierEnclave.edl \
		--search-path ../Enclave \
		--search-path ../Common \
		--search-path $(SGX_SDK)/include

	@echo "GEN  =>  $@"

VerifierEnclave_u.o: VerifierEnclave_u.c
	@$(CC) $(App_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

%.o: %.cpp
	@$(CXX) $(App_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

%.o: %.c
	@$(CC) $(App_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

# Compile common files different
# because they are conditionally compiled
# depending on ENCLAVE_STD_ALT

%.a.o: %.cpp
	@$(CXX) $(App_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

%.a.o: %.c
	@$(CC) $(App_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

$(App_Name): VerifierEnclave_u.o $(APP_OBJS) $(COMMON_OBJS)
	@$(CXX) $^ -o $@ $(App_Link_Flags)
	@echo "LINK =>  $@"

clean:
	@rm -rf $(App_Name) $(APP_OBJS) $(COMMON_OBJS)

