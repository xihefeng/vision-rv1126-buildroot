cmd_tpl/arch/arm/lib/div0.o := /media/ssd/FireFly-RV1126JD4/rv1126_rv1109_linux_release_20211022/prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc -Wp,-MD,tpl/arch/arm/lib/.div0.o.d  -nostdinc -isystem /media/ssd/FireFly-RV1126JD4/rv1126_rv1109_linux_release_20211022/prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/../lib/gcc/arm-linux-gnueabihf/6.3.1/include -Iinclude    -I./arch/arm/include -include ./include/linux/kconfig.h -D__KERNEL__ -D__UBOOT__ -DCONFIG_SPL_BUILD -DCONFIG_TPL_BUILD -Wall -Wstrict-prototypes -Wno-format-security -fno-builtin -ffreestanding -fshort-wchar -Werror -Os -fno-stack-protector -fno-delete-null-pointer-checks -g -fstack-usage -Wno-format-nonliteral -Werror=date-time -ffunction-sections -fdata-sections -D__ARM__ -Wa,-mimplicit-it=always -mthumb -mthumb-interwork -mabi=aapcs-linux -mno-unaligned-access -ffunction-sections -fdata-sections -fno-common -ffixed-r9 -msoft-float -pipe -march=armv7-a -D__LINUX_ARM_ARCH__=7 -I./arch/arm/mach-rockchip/include    -D"KBUILD_STR(s)=$(pound)s" -D"KBUILD_BASENAME=KBUILD_STR(div0)"  -D"KBUILD_MODNAME=KBUILD_STR(div0)" -c -o tpl/arch/arm/lib/div0.o arch/arm/lib/div0.c

source_tpl/arch/arm/lib/div0.o := arch/arm/lib/div0.c

deps_tpl/arch/arm/lib/div0.o := \

tpl/arch/arm/lib/div0.o: $(deps_tpl/arch/arm/lib/div0.o)

$(deps_tpl/arch/arm/lib/div0.o):
