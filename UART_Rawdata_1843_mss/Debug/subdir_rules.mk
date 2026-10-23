################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
build-1823270497:
	@$(MAKE) --no-print-directory -Onone -f subdir_rules.mk build-1823270497-inproc

build-1823270497-inproc: ../mss.cfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: XDCtools'
	"C:/ti/xdctools_3_62_01_16_core/xs" --xdcpath="C:/ti/bios_6_73_01_01/packages;C:/ti/mathlib_c674x_3_1_2_1/packages;" xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.R4F -p ti.platforms.cortexR:IWR18XX:false:200 -r release -c "C:/ti/ti-cgt-arm_20.2.7.LTS" --compileOptions "--enum_type=int" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

configPkg/linker.cmd: build-1823270497 ../mss.cfg
configPkg/compiler.opt: build-1823270497
configPkg/: build-1823270497

%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7R4 --code_state=16 --float_support=VFPv3D16 -me --include_path="C:/ti/mmwave_sdk_03_06_00_00-LTS/packages" --include_path="D:/NewProject/DSP/mmWave/Template_xWR/UART_Rawdata_1843_mss" --include_path="C:/ti/bios_6_73_01_01/packages/ti/posix/ccs" --include_path="C:/ti/ti-cgt-arm_20.2.7.LTS/include" --define=SOC_XWR18XX --define=SUBSYS_MSS --define=DOWNLOAD_FROM_CCS --define=MMWAVE_L3RAM_NUM_BANK=6 --define=MMWAVE_SHMEM_BANK_SIZE=0x20000 --define=MMWAVE_SHMEM_TCMA_NUM_BANK=0 --define=MMWAVE_SHMEM_TCMB_NUM_BANK=0 --define=DebugP_ASSERT_ENABLED --define=_LITTLE_ENDIAN --define=OBJDET_NO_RANGE -g --diag_warning=225 --diag_wrap=off --display_error_number --enum_type=int --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


