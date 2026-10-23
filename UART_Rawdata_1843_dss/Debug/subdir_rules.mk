################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
build-1428328191:
	@$(MAKE) --no-print-directory -Onone -f subdir_rules.mk build-1428328191-inproc

build-1428328191-inproc: ../dss.cfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: XDCtools'
	"C:/ti/xdctools_3_62_01_16_core/xs" --xdcpath="C:/ti/bios_6_73_01_01/packages;C:/ti/mathlib_c674x_3_1_2_1/packages;" xdc.tools.configuro -o configPkg -t ti.targets.elf.C674 -p ti.platforms.c6x:IWR18XX:false:600 -r release -c "C:/ti/ti-cgt-c6000_8.3.12" --compileOptions "-g" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

configPkg/linker.cmd: build-1428328191 ../dss.cfg
configPkg/compiler.opt: build-1428328191
configPkg/: build-1428328191

%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ti-cgt-c6000_8.3.12/bin/cl6x" -mv6740 -Ooff --opt_for_speed=4 --include_path="C:/ti/mmwave_sdk_03_06_00_00-LTS/packages" --include_path="D:/NewProject/DSP/mmWave/Template_xWR/UART_Rawdata_1843_dss" --include_path="C:/ti/bios_6_73_01_01/packages/ti/posix/ccs" --include_path="C:/ti/ti-cgt-c6000_8.3.12/include" --define=SOC_XWR18XX --define=SUBSYS_DSS --define=MMWAVE_L3RAM_NUM_BANK=6 --define=MMWAVE_SHMEM_TCMA_NUM_BANK=0 --define=MMWAVE_SHMEM_TCMB_NUM_BANK=0 --define=MMWAVE_SHMEM_BANK_SIZE=0x20000 --define=DOWNLOAD_FROM_CCS --define=DebugP_ASSERT_ENABLED --define=_LITTLE_ENDIAN --define=OBJDET_NO_RANGE -g --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


