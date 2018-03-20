include Makefile.defs

.PHONY: all run

INSTALL_PATH=$(PWD)

ifeq ($(Build_Mode), HW_RELEASE)
all: 
	@echo "The project has been built in release hardware mode."
	@echo "Please sign the $(Enclave_Name) first with your signing key before you run the $(App_Name) to launch and access the enclave."
	@echo "To sign the enclave use the command:"
	@echo "   $(SGX_ENCLAVE_SIGNER) sign -key <your key> -enclave $(Enclave_Name) -out <$(Signed_Enclave_Name)> -config $(Enclave_Config_File)"
	@echo "You can also sign the enclave using an external signing tool. See User's Guide for more details."
	@echo "To build the project in simulation mode set SGX_MODE=SIM. To build the project in prerelease mode set SGX_PRERELEASE=1 and SGX_MODE=HW."
else
all: pouw_enclave 
	@make -C App
	@mkdir -p $(INSTALL_PATH)
	@mv App/verifier $(INSTALL_PATH)
	@mv Enclave/verifier.signed.so $(INSTALL_PATH)

pouw_enclave:
	@make -C Enclave

endif

.PHONY: clean

clean:
	make -C App clean
	make -C Enclave clean
