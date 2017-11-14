################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
third_party/fatfs/src/ff.obj: ../third_party/fatfs/src/ff.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"/Applications/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --include_path="/Applications/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/include" --include_path="/Users/tauanmarinho/Desktop/VERSÕES/trabalho5/trabalhoJunto" --include_path="/Users/tauanmarinho/ti/TivaWare_C_Series-2.1.3.156/examples/boards/ek-tm4c129exl" --include_path="/Users/tauanmarinho/ti/TivaWare_C_Series-2.1.3.156" --include_path="/Users/tauanmarinho/ti/TivaWare_C_Series-2.1.3.156/third_party/lwip-1.4.1/src/include" --include_path="/Users/tauanmarinho/ti/TivaWare_C_Series-2.1.3.156/third_party/lwip-1.4.1/src/include/ipv4" --include_path="/Users/tauanmarinho/ti/TivaWare_C_Series-2.1.3.156/third_party/lwip-1.4.1/apps" --include_path="/Users/tauanmarinho/ti/TivaWare_C_Series-2.1.3.156/third_party/lwip-1.4.1/ports/tiva-tm4c129/include" --include_path="/Users/tauanmarinho/ti/TivaWare_C_Series-2.1.3.156/third_party" --advice:power=all -g --gcc --define=ccs="ccs" --define=PART_TM4C129ENCPDT --define=TARGET_IS_TM4C129_RA0 --define=LWIP_1_4_1 --define=USE_HTTP_1_0 --define=UART_BUFFERED --diag_wrap=off --diag_warning=225 --display_error_number --gen_func_subsections=on --ual --preproc_with_compile --preproc_dependency="third_party/fatfs/src/ff.d" --obj_directory="third_party/fatfs/src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


