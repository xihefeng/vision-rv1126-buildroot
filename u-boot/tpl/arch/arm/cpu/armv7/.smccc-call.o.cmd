cmd_tpl/arch/arm/cpu/armv7/smccc-call.o := /media/ssd/FireFly-RV1126JD4/rv1126_rv1109_linux_release_20211022/prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc -Wp,-MD,tpl/arch/arm/cpu/armv7/.smccc-call.o.d  -nostdinc -isystem /media/ssd/FireFly-RV1126JD4/rv1126_rv1109_linux_release_20211022/prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/../lib/gcc/arm-linux-gnueabihf/6.3.1/include -Iinclude    -I./arch/arm/include -include ./include/linux/kconfig.h -D__KERNEL__ -D__UBOOT__ -DCONFIG_SPL_BUILD -DCONFIG_TPL_BUILD -D__ASSEMBLY__ -g -D__ARM__ -Wa,-mimplicit-it=always -mthumb -mthumb-interwork -mabi=aapcs-linux -mno-unaligned-access -ffunction-sections -fdata-sections -fno-common -ffixed-r9 -msoft-float -pipe -march=armv7-a -D__LINUX_ARM_ARCH__=7 -I./arch/arm/mach-rockchip/include   -c -o tpl/arch/arm/cpu/armv7/smccc-call.o arch/arm/cpu/armv7/smccc-call.S

source_tpl/arch/arm/cpu/armv7/smccc-call.o := arch/arm/cpu/armv7/smccc-call.S

deps_tpl/arch/arm/cpu/armv7/smccc-call.o := \
  include/linux/linkage.h \
  arch/arm/include/asm/linkage.h \
  arch/arm/include/asm/opcodes-sec.h \
  arch/arm/include/asm/opcodes.h \
    $(wildcard include/config/cpu/endian/be8.h) \
    $(wildcard include/config/cpu/endian/be32.h) \
    $(wildcard include/config/thumb2/kernel.h) \
  include/linux/stringify.h \
  arch/arm/include/asm/opcodes-virt.h \

tpl/arch/arm/cpu/armv7/smccc-call.o: $(deps_tpl/arch/arm/cpu/armv7/smccc-call.o)

$(deps_tpl/arch/arm/cpu/armv7/smccc-call.o):
