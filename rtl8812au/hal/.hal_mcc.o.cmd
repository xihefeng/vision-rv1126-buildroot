cmd_/media/ssd/FireFly-RV1126JD4/rv1126_rv1109/rtl8812au/hal/hal_mcc.o := ./scripts/gcc-wrapper.py ../prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc -Wp,-MD,/media/ssd/FireFly-RV1126JD4/rv1126_rv1109/rtl8812au/hal/.hal_mcc.o.d  -nostdinc -isystem /media/ssd/FireFly-RV1126JD4/rv1126_rv1109/prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/../lib/gcc/arm-linux-gnueabihf/6.3.1/include -I./arch/arm/include -I./arch/arm/include/generated  -I./include -I./arch/arm/include/uapi -I./arch/arm/include/generated/uapi -I./include/uapi -I./include/generated/uapi -include ./include/linux/kconfig.h -include ./include/linux/compiler_types.h -D__KERNEL__ -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -fno-PIE -fno-dwarf2-cfi-asm -fno-ipa-sra -mabi=aapcs-linux -mfpu=vfp -funwind-tables -marm -Wa,-mno-warn-deprecated -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -fno-delete-null-pointer-checks -Wno-frame-address -O2 --param=allow-store-data-races=0 -Wframe-larger-than=1024 -fstack-protector-strong -Wno-unused-but-set-variable -Wno-unused-const-variable -fomit-frame-pointer -fno-var-tracking-assignments -g -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-merge-all-constants -fmerge-constants -fno-stack-check -fconserve-stack -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -Werror=incompatible-pointer-types -Werror=designated-init -fno-pie -O1 -Wall -Wextra -Wno-unused-variable -Wno-unused-value -Wno-unused-label -Wno-unused-parameter -Wno-unused-function -Wno-unused -Wno-cast-function-type -Wno-date-time -Wno-uninitialized -Wno-sign-compare -Wno-type-limits -Wno-date-time -Wno-error=date-time -Wno-vla -I/media/ssd/FireFly-RV1126JD4/rv1126_rv1109/rtl8812au/include -DCONFIG_DISABLE_REGD_C -DDBG=0 -DDRV_NAME=\"rtl88xxau_wfb\" -DCONFIG_USE_EXTERNAL_POWER -I/media/ssd/FireFly-RV1126JD4/rv1126_rv1109/rtl8812au/hal/btc -I/media/ssd/FireFly-RV1126JD4/rv1126_rv1109/rtl8812au/hal/phydm -DCONFIG_RTL8812A -DCONFIG_RTL8821A -DCONFIG_RTL8814A -DCONFIG_MP_INCLUDED -DCONFIG_EFUSE_CONFIG_FILE -DEFUSE_MAP_PATH=\"/system/etc/wifi/wifi_efuse_8814au.map\" -DWIFIMAC_PATH=\"/data/wifimac.txt\" -DCONFIG_TXPWR_BY_RATE_EN=0 -DCONFIG_TXPWR_LIMIT_EN=0 -DCONFIG_CALIBRATE_TX_POWER_TO_MAX -DCONFIG_RTW_ADAPTIVITY_EN=0 -DCONFIG_RTW_ADAPTIVITY_MODE=0 -DCONFIG_BR_EXT '-DCONFIG_BR_EXT_BRNAME="'br0'"' -DCONFIG_WIFI_MONITOR -DCONFIG_RTW_NAPI -DCONFIG_RTW_GRO -DCONFIG_RTW_WIFI_HAL -DCONFIG_RTW_CFGVEDNOR_LLSTATS -DCONFIG_VHT_EXTRAS -DCONFIG_LED_CONTROL -DCONFIG_LED_ENABLE -DDM_ODM_SUPPORT_TYPE=0x04 -DCONFIG_LITTLE_ENDIAN -DCONFIG_IOCTL_CFG80211 -DRTW_USE_CFG80211_STA_EVENT  -DMODULE  -DKBUILD_BASENAME='"hal_mcc"' -DKBUILD_MODNAME='"88XXau_wfb"' -c -o /media/ssd/FireFly-RV1126JD4/rv1126_rv1109/rtl8812au/hal/hal_mcc.o /media/ssd/FireFly-RV1126JD4/rv1126_rv1109/rtl8812au/hal/hal_mcc.c

source_/media/ssd/FireFly-RV1126JD4/rv1126_rv1109/rtl8812au/hal/hal_mcc.o := /media/ssd/FireFly-RV1126JD4/rv1126_rv1109/rtl8812au/hal/hal_mcc.c

deps_/media/ssd/FireFly-RV1126JD4/rv1126_rv1109/rtl8812au/hal/hal_mcc.o := \
    $(wildcard include/config/mcc/mode.h) \
    $(wildcard include/config/mcc/mode/debug.h) \
  include/linux/kconfig.h \
    $(wildcard include/config/cpu/big/endian.h) \
    $(wildcard include/config/booger.h) \
    $(wildcard include/config/foo.h) \
  include/linux/compiler_types.h \
    $(wildcard include/config/have/arch/compiler/h.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/retpoline.h) \
    $(wildcard include/config/arch/use/builtin/bswap.h) \

/media/ssd/FireFly-RV1126JD4/rv1126_rv1109/rtl8812au/hal/hal_mcc.o: $(deps_/media/ssd/FireFly-RV1126JD4/rv1126_rv1109/rtl8812au/hal/hal_mcc.o)

$(deps_/media/ssd/FireFly-RV1126JD4/rv1126_rv1109/rtl8812au/hal/hal_mcc.o):
