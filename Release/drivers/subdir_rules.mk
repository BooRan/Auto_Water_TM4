################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
drivers/DS1307.obj: ../drivers/DS1307.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --include_path="C:/ti/ccsv5/tools/compiler/arm_5.1.1/include" --include_path="C:/TI/TivaWare_C_Series_1_1" --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="drivers/DS1307.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/I2C.obj: ../drivers/I2C.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --include_path="C:/ti/ccsv5/tools/compiler/arm_5.1.1/include" --include_path="C:/TI/TivaWare_C_Series_1_1" --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="drivers/I2C.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/UART.obj: ../drivers/UART.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --include_path="C:/ti/ccsv5/tools/compiler/arm_5.1.1/include" --include_path="C:/TI/TivaWare_C_Series_1_1" --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="drivers/UART.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


