cmd_spl/drivers/misc/rockchip-secure-otp-v2.o := /media/ssd/FireFly-RV1126JD4/rv1126_rv1109_linux_release_20211022/prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc -Wp,-MD,spl/drivers/misc/.rockchip-secure-otp-v2.o.d  -nostdinc -isystem /media/ssd/FireFly-RV1126JD4/rv1126_rv1109_linux_release_20211022/prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/../lib/gcc/arm-linux-gnueabihf/6.3.1/include -Iinclude    -I./arch/arm/include -include ./include/linux/kconfig.h -D__KERNEL__ -D__UBOOT__ -DCONFIG_SPL_BUILD -D__ASSEMBLY__ -g -D__ARM__ -Wa,-mimplicit-it=always -mthumb -mthumb-interwork -mabi=aapcs-linux -mno-unaligned-access -ffunction-sections -fdata-sections -fno-common -ffixed-r9 -msoft-float -pipe -march=armv7-a -D__LINUX_ARM_ARCH__=7 -I./arch/arm/mach-rockchip/include   -c -o spl/drivers/misc/rockchip-secure-otp-v2.o drivers/misc/rockchip-secure-otp-v2.S

source_spl/drivers/misc/rockchip-secure-otp-v2.o := drivers/misc/rockchip-secure-otp-v2.S

deps_spl/drivers/misc/rockchip-secure-otp-v2.o := \

spl/drivers/misc/rockchip-secure-otp-v2.o: $(deps_spl/drivers/misc/rockchip-secure-otp-v2.o)

$(deps_spl/drivers/misc/rockchip-secure-otp-v2.o):
