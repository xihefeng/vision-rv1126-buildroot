//
// Created by consti10 on 25.05.21.
//

#ifndef MEDIASEVER_IMX415_REGS_ROCKCHIP_H
#define MEDIASEVER_IMX415_REGS_ROCKCHIP_H

// just like nvidia does, move the register stuff into a .h file to make the rest more easy to read
// these are all definitions that were in the original rockchip driver

#define REG_NULL			0xFFFF

struct regval {
    u16 addr;
    u8 val;
};

#define IMX415_VMAX_L 0x3024
#define IMX415_VMAX_M 0x3025
#define IMX415_VMAX_H 0x3026
#define IMX415_HMAX_L 0x3028
#define IMX415_HMAX_H 0x3029
//
#define IMX415_SHR0_L 0x3050
#define IMX415_SHR0_M 0x3051
#define IMX415_SHR0_H 0x3052

#define IMX415_XMSTA 0x3002


// each of them is 2 byte though
#define IMX415_TCLKPOST 0x4018
#define IMX415_TCLKPREPARE 0x401A
#define IMX415_TCLKTRAIL 0x401C
#define IMX415_TCLKZERO_L 0x401E
#define IMX415_TCLKZERO_H 0x401F
#define IMX415_THSPREPARE 0x4020
#define IMX415_THSZERO 0x4022
#define IMX415_THSTRAIL 0x4024
#define IMX415_THSEXIT 0x4026
#define IMX415_TLPX 0x4028

// regarding cropping:
#define IMX415_WINMODE 0x301C
// 2 byte
#define IMX415_PIX_HST_L 0x3040
#define IMX415_PIX_HST_H 0x3041
#define IMX415_PIX_HWIDTH_L 0x3042
#define IMX415_PIX_HWIDTH_H 0x3043
#define IMX415_PIX_VST_L 0x3044
#define IMX415_PIX_VST_H 0x3045
#define IMX415_PIX_VWIDTH_L 0x3046
#define IMX415_PIX_VWIDTH_H 0x3047

// regarding "INCK"
#define IMX415_BCWAIT_TIME 0x3009
#define IMX415_CPWAIT_TIME 0x300B
#define IMX415_SYS_MODE 0x3033 // weird is there something wrong in the spec sheet ? also referred to as 0x034
#define IMX415_INCKSEL1 0x3115
#define IMX415_INCKSEL2 0x3116
#define IMX415_INCKSEL3_L 0x3118
#define IMX415_INCKSEL3_H 0x3119
#define IMX415_INCKSEL4_L 0x311A
#define IMX415_INCKSEL4_H 0x311B
#define IMX415_INCKSEL5 0x311E
#define IMX415_TXCLKESC_FREQ_L 0x4004
#define IMX415_TXCLKESC_FREQ_H 0x4005
#define IMX415_INCKSEL6 0x400C
#define IMX415_INCKSEL7 0x4074

//For 891Mbps & 37.125 | For 1485 & 37.125
// 07Fh                | 07Fh
// 05Bh                | 05Bh
// 5h                  | 8h               SYS_MODE
// 00h                 | 00h
// 24h                 | 24h
// 0C0h                | 0A0              INCKSEL3
// 0E0h                | 0E0h
// 24h                 | 24h
// 0948h               | 0948h
// 0h                  | 1h               INCKSEL6
// 1h                  | 0h               INCKSEL7

static __maybe_unused const struct regval imx415_global_10bit_3864x2192_regs[] = {
        {0x3002, 0x00},
        {0x3008, 0x7F}, //37.125[Mhz]
        {0x300A, 0x5B},
        {0x3031, 0x00},
        {0x3032, 0x00},
        {0x30C1, 0x00},
        {0x3116, 0x24},
        {0x311E, 0x24},
        {0x32D4, 0x21},
        {0x32EC, 0xA1},
        {0x3452, 0x7F},
        {0x3453, 0x03},
        {0x358A, 0x04},
        {0x35A1, 0x02},
        {0x36BC, 0x0C},
        {0x36CC, 0x53},
        {0x36CD, 0x00},
        {0x36CE, 0x3C},
        {0x36D0, 0x8C},
        {0x36D1, 0x00},
        {0x36D2, 0x71},
        {0x36D4, 0x3C},
        {0x36D6, 0x53},
        {0x36D7, 0x00},
        {0x36D8, 0x71},
        {0x36DA, 0x8C},
        {0x36DB, 0x00},
        {0x3701, 0x00},
        {0x3724, 0x02},
        {0x3726, 0x02},
        {0x3732, 0x02},
        {0x3734, 0x03},
        {0x3736, 0x03},
        {0x3742, 0x03},
        {0x3862, 0xE0},
        {0x38CC, 0x30},
        {0x38CD, 0x2F},
        {0x395C, 0x0C},
        {0x3A42, 0xD1},
        {0x3A4C, 0x77},
        {0x3AE0, 0x02},
        {0x3AEC, 0x0C},
        {0x3B00, 0x2E},
        {0x3B06, 0x29},
        {0x3B98, 0x25},
        {0x3B99, 0x21},
        {0x3B9B, 0x13},
        {0x3B9C, 0x13},
        {0x3B9D, 0x13},
        {0x3B9E, 0x13},
        {0x3BA1, 0x00},
        {0x3BA2, 0x06},
        {0x3BA3, 0x0B},
        {0x3BA4, 0x10},
        {0x3BA5, 0x14},
        {0x3BA6, 0x18},
        {0x3BA7, 0x1A},
        {0x3BA8, 0x1A},
        {0x3BA9, 0x1A},
        {0x3BAC, 0xED},
        {0x3BAD, 0x01},
        {0x3BAE, 0xF6},
        {0x3BAF, 0x02},
        {0x3BB0, 0xA2},
        {0x3BB1, 0x03},
        {0x3BB2, 0xE0},
        {0x3BB3, 0x03},
        {0x3BB4, 0xE0},
        {0x3BB5, 0x03},
        {0x3BB6, 0xE0},
        {0x3BB7, 0x03},
        {0x3BB8, 0xE0},
        {0x3BBA, 0xE0},
        {0x3BBC, 0xDA},
        {0x3BBE, 0x88},
        {0x3BC0, 0x44},
        {0x3BC2, 0x7B},
        {0x3BC4, 0xA2},
        {0x3BC8, 0xBD},
        {0x3BCA, 0xBD},
        {0x4004, 0x48},
        {0x4005, 0x09},
        {REG_NULL, 0x00},
};

/*
 * Xclk 37.125Mhz
 */
static __maybe_unused const struct regval imx415_global_12bit_3864x2192_regs[] = {
        {0x3002, 0x00},
        {0x3008, 0x7F},
        {0x300A, 0x5B},
        {0x30C1, 0x00},
        {0x3116, 0x24},
        {0x311E, 0x24},
        {0x32D4, 0x21},
        {0x32EC, 0xA1},
        {0x3452, 0x7F},
        {0x3453, 0x03},
        {0x358A, 0x04},
        {0x35A1, 0x02},
        {0x36BC, 0x0C},
        {0x36CC, 0x53},
        {0x36CD, 0x00},
        {0x36CE, 0x3C},
        {0x36D0, 0x8C},
        {0x36D1, 0x00},
        {0x36D2, 0x71},
        {0x36D4, 0x3C},
        {0x36D6, 0x53},
        {0x36D7, 0x00},
        {0x36D8, 0x71},
        {0x36DA, 0x8C},
        {0x36DB, 0x00},
        {0x3724, 0x02},
        {0x3726, 0x02},
        {0x3732, 0x02},
        {0x3734, 0x03},
        {0x3736, 0x03},
        {0x3742, 0x03},
        {0x3862, 0xE0},
        {0x38CC, 0x30},
        {0x38CD, 0x2F},
        {0x395C, 0x0C},
        {0x3A42, 0xD1},
        {0x3A4C, 0x77},
        {0x3AE0, 0x02},
        {0x3AEC, 0x0C},
        {0x3B00, 0x2E},
        {0x3B06, 0x29},
        {0x3B98, 0x25},
        {0x3B99, 0x21},
        {0x3B9B, 0x13},
        {0x3B9C, 0x13},
        {0x3B9D, 0x13},
        {0x3B9E, 0x13},
        {0x3BA1, 0x00},
        {0x3BA2, 0x06},
        {0x3BA3, 0x0B},
        {0x3BA4, 0x10},
        {0x3BA5, 0x14},
        {0x3BA6, 0x18},
        {0x3BA7, 0x1A},
        {0x3BA8, 0x1A},
        {0x3BA9, 0x1A},
        {0x3BAC, 0xED},
        {0x3BAD, 0x01},
        {0x3BAE, 0xF6},
        {0x3BAF, 0x02},
        {0x3BB0, 0xA2},
        {0x3BB1, 0x03},
        {0x3BB2, 0xE0},
        {0x3BB3, 0x03},
        {0x3BB4, 0xE0},
        {0x3BB5, 0x03},
        {0x3BB6, 0xE0},
        {0x3BB7, 0x03},
        {0x3BB8, 0xE0},
        {0x3BBA, 0xE0},
        {0x3BBC, 0xDA},
        {0x3BBE, 0x88},
        {0x3BC0, 0x44},
        {0x3BC2, 0x7B},
        {0x3BC4, 0xA2},
        {0x3BC8, 0xBD},
        {0x3BCA, 0xBD},
        {0x4004, 0x48},
        {0x4005, 0x09},
        {REG_NULL, 0x00},
};

static __maybe_unused const struct regval imx415_linear_12bit_3864x2192_891M_regs[] = {
        {0x3024, 0xCA},
        {0x3025, 0x08},
        {0x3028, 0x4C},
        {0x3029, 0x04},
        {0x302C, 0x00},
        {0x302D, 0x00},
        {0x3033, 0x05},
        {0x3050, 0x08},
        {0x3051, 0x00},
        {0x3054, 0x19},
        {0x3058, 0x3E},
        {0x3060, 0x25},
        {0x3064, 0x4A},
        {0x30CF, 0x00},
        {0x3260, 0x01},
        {0x400C, 0x00},
        {0x4018, 0x7F},
        {0x401A, 0x37},
        {0x401C, 0x37},
        {0x401E, 0xF7},
        {0x401F, 0x00},
        {0x4020, 0x3F},
        {0x4022, 0x6F},
        {0x4024, 0x3F},
        {0x4026, 0x5F},
        {0x4028, 0x2F},
        {0x4074, 0x01},
        {REG_NULL, 0x00},
};

static __maybe_unused const struct regval imx415_hdr2_12bit_3864x2192_1782M_regs[] = {
        {0x3024, 0xCA},
        {0x3025, 0x08},
        {0x3028, 0x26},
        {0x3029, 0x02},
        {0x302C, 0x01},
        {0x302D, 0x01},
        {0x3033, 0x04},
        {0x3050, 0x90},
        {0x3051, 0x0D},
        {0x3054, 0x09},
        {0x3058, 0x3E},
        {0x3060, 0x4D},
        {0x3064, 0x4A},
        {0x30CF, 0x01},
        {0x3260, 0x00},
        {0x400C, 0x01},
        {0x4018, 0xB7},
        {0x401A, 0x67},
        {0x401C, 0x6F},
        {0x401E, 0xDF},
        {0x401F, 0x01},
        {0x4020, 0x6F},
        {0x4022, 0xCF},
        {0x4024, 0x6F},
        {0x4026, 0xB7},
        {0x4028, 0x5F},
        {0x4074, 0x00},
        {REG_NULL, 0x00},
};

static __maybe_unused const struct regval imx415_hdr3_12bit_3864x2192_1782M_regs[] = {
        {0x3024, 0x96},
        {0x3025, 0x06},
        {0x3028, 0x26},
        {0x3029, 0x02},
        {0x302C, 0x01},
        {0x302D, 0x02},
        {0x3033, 0x04},
        {0x3050, 0x14},
        {0x3051, 0x01},
        {0x3054, 0x0D},
        {0x3058, 0x26},
        {0x3060, 0x19},
        {0x3064, 0x32},
        {0x30CF, 0x03},
        {0x3260, 0x00},
        {0x400C, 0x01},
        {0x4018, 0xB7},
        {0x401A, 0x67},
        {0x401C, 0x6F},
        {0x401E, 0xDF},
        {0x401F, 0x01},
        {0x4020, 0x6F},
        {0x4022, 0xCF},
        {0x4024, 0x6F},
        {0x4026, 0xB7},
        {0x4028, 0x5F},
        {0x4074, 0x00},
        {REG_NULL, 0x00},
};


static __maybe_unused const struct regval imx415_hdr3_10bit_3864x2192_1485M_regs[] = {
        {0x3024, 0xBD},
        {0x3025, 0x06},
        {0x3028, 0x1A},
        {0x3029, 0x02},
        {0x302C, 0x01},
        {0x302D, 0x02},
        {0x3033, 0x08},
        {0x3050, 0x90},
        {0x3051, 0x15},
        {0x3054, 0x0D},
        {0x3058, 0xA4},
        {0x3060, 0x97},
        {0x3064, 0xB6},
        {0x30CF, 0x03},
        {0x3118, 0xA0},
        {0x3260, 0x00},
        {0x400C, 0x01},
        {0x4018, 0xA7},
        {0x401A, 0x57},
        {0x401C, 0x5F},
        {0x401E, 0x97},
        {0x401F, 0x01},
        {0x4020, 0x5F},
        {0x4022, 0xAF},
        {0x4024, 0x5F},
        {0x4026, 0x9F},
        {0x4028, 0x4F},
        {0x4074, 0x00},
        {REG_NULL, 0x00},
};

static __maybe_unused const struct regval imx415_hdr3_10bit_3864x2192_1782M_regs[] = {
        {0x3024, 0xEA},
        {0x3025, 0x07},
        {0x3028, 0xCA},
        {0x3029, 0x01},
        {0x302C, 0x01},
        {0x302D, 0x02},
        {0x3033, 0x04},
        {0x3050, 0x3E},
        {0x3051, 0x01},
        {0x3054, 0x0D},
        {0x3058, 0x9E},
        {0x3060, 0x91},
        {0x3064, 0xC2},
        {0x30CF, 0x03},
        {0x3118, 0xC0},
        {0x3260, 0x00},
        {0x400C, 0x01},
        {0x4018, 0xB7},
        {0x401A, 0x67},
        {0x401C, 0x6F},
        {0x401E, 0xDF},
        {0x401F, 0x01},
        {0x4020, 0x6F},
        {0x4022, 0xCF},
        {0x4024, 0x6F},
        {0x4026, 0xB7},
        {0x4028, 0x5F},
        {0x4074, 0x00},
        {REG_NULL, 0x00},
};

static __maybe_unused const struct regval imx415_hdr2_10bit_3864x2192_1485M_regs[] = {
        {0x3024, 0xFC},
        {0x3025, 0x08},
        {0x3028, 0x1A},
        {0x3029, 0x02},
        {0x302C, 0x01},
        {0x302D, 0x01},
        {0x3033, 0x08},
        {0x3050, 0xA8},
        {0x3051, 0x0D},
        {0x3054, 0x09},
        {0x3058, 0x3E},
        {0x3060, 0x4D},
        {0x3064, 0x4a},
        {0x30CF, 0x01},
        {0x3118, 0xA0},
        {0x3260, 0x00},
        {0x400C, 0x01},
        {0x4018, 0xA7},
        {0x401A, 0x57},
        {0x401C, 0x5F},
        {0x401E, 0x97},
        {0x401F, 0x01},
        {0x4020, 0x5F},
        {0x4022, 0xAF},
        {0x4024, 0x5F},
        {0x4026, 0x9F},
        {0x4028, 0x4F},
        {0x4074, 0x00},
        {REG_NULL, 0x00},
};

static __maybe_unused const struct regval imx415_linear_10bit_3864x2192_891M_regs[] = {
        {0x3024, 0xCA},
        {0x3025, 0x08},
        {0x3028, 0x4C},
        {0x3029, 0x04},
        {0x302C, 0x00},
        {0x302D, 0x00},
        {0x3033, 0x05},
        {0x3050, 0x08},
        {0x3051, 0x00},
        {0x3054, 0x19},
        {0x3058, 0x3E},
        {0x3060, 0x25},
        {0x3064, 0x4a},
        {0x30CF, 0x00},
        {0x3118, 0xC0},
        {0x3260, 0x01},
        {0x400C, 0x00},
        {0x4018, 0x7F},
        {0x401A, 0x37},
        {0x401C, 0x37},
        {0x401E, 0xF7},
        {0x401F, 0x00},
        {0x4020, 0x3F},
        {0x4022, 0x6F},
        {0x4024, 0x3F},
        {0x4026, 0x5F},
        {0x4028, 0x2F},
        {0x4074, 0x01},
        // added for testing Consti10:
        {0x301C,0x00}, //WINMODE //0: All-pixel mode, Horizontal/Vertical 2/2-line binning 4: Window cropping mode
        {0x3020,0x00}, //HADD //0h: All-pixel mode 1h: Horizontal 2 binning
        {0x3021,0x00}, //VADD //0h: All-pixel mode 1h: Vertical 2 binning
        {0x3022,0x00}, //ADDMODE //0h: All-pixel mode 1h: Horizontal/Vertical 2/2-line binning
        //
        // to resolve:
        {0x3031,0x00}, //ADBIT //set by global to 0
        {0x3032,0x00}, //MDBIT //set by global to 0
        //
        {0x30D9,0x06}, //DIG_CLP_VSTART ? 0x02=binning 0x06=All-pixel scan mode , default 0x06
        {0x30DA,0x02}, //DIG_CLP_VNUM ? 0x01=binning 0x02=all-pixel scan mode, default 0x02
        // added for testing Consti10 end

        {REG_NULL, 0x00},
};

// 4K sensor res "binned down" to 1080p (2x2binning)
static __maybe_unused const struct regval imx415_linear_10bit_3864x2192_891M_regs_binning[] = {
        {0x3024, 0xCA},
        {0x3025, 0x08},
        {IMX415_HMAX_L, 0x4C},
        {IMX415_HMAX_H, 0x04},
        {0x302C, 0x00},
        {0x302D, 0x00},
        {0x3033, 0x05},
        {0x3050, 0x08},
        {0x3051, 0x00},
        {0x3054, 0x19},
        {0x3058, 0x3E},
        {0x3060, 0x25},
        {0x3064, 0x4a},
        {0x30CF, 0x00},
        {0x3118, 0xC0},
        {0x3260, 0x01},
        {0x400C, 0x00},
        {0x4018, 0x7F},
        {0x401A, 0x37},
        {0x401C, 0x37},
        {0x401E, 0xF7},
        {0x401F, 0x00},
        {0x4020, 0x3F},
        {0x4022, 0x6F},
        {0x4024, 0x3F},
        {0x4026, 0x5F},
        {0x4028, 0x2F},
        {0x4074, 0x01},
        // added for testing Consti10:
        {0x301C,0x00}, //WINMODE //0: All-pixel mode, Horizontal/Vertical 2/2-line binning 4: Window cropping mode
        {0x3020,0x01}, //HADD //0h: All-pixel mode 1h: Horizontal 2 binning
        {0x3021,0x01}, //VADD //0h: All-pixel mode 1h: Vertical 2 binning
        {0x3022,0x01}, //ADDMODE //0h: All-pixel mode 1h: Horizontal/Vertical 2/2-line binning
        //
        // to resolve:
        {0x3031,0x00}, //ADBIT //set by global to 0 , 0=10bit 1=12bit
        {0x3032,0x00}, //MDBIT //set by global to 0
        //
        {0x30D9,0x02}, //DIG_CLP_VSTAET ? 0x02=binning 0x06=All-pixel scan mode , default 0x06
        {0x30DA,0x01}, //DIG_CLP_VNUM ? 0x01=binning 0x02=all-pixel scan mode, default 0x02
        // added for testing Consti10 end

        {REG_NULL, 0x00},
};

// low and high (2x8 bit) forming 16bit number
// low == smaller reg number of both
#define IMX415_FETCH_16BIT_H(VAL)	(((VAL) >> 8) & 0x07)
#define IMX415_FETCH_16BIT_L(VAL)	((VAL) & 0xFF)

#define IMX415_EFFECTIVE_PIXEL_W 3864
#define IMX415_EFFECTIVE_PIXEL_H 2192
#define IMX415_RECOMMENDED_RECORDING_W 3840
#define IMX415_RECOMMENDED_RECORDING_H 2160

//3864-1920 = 1944 | 1944/2 = 972
//2192-1080 = 1112 | 1112/2 = 556

// 4k sensor cropped down to 1080p
static __maybe_unused const struct regval imx415_linear_10bit_3864x2192_891M_regs_cropping[] = {
        {IMX415_VMAX_L, 0xCA}, //maybe same
        {IMX415_VMAX_M, 0x08}, //maybe same
        {IMX415_HMAX_L,IMX415_FETCH_16BIT_L(0x44C)},
        {IMX415_HMAX_H,IMX415_FETCH_16BIT_H(0x44C)},
        {0x302C, 0x00}, //cannot find
        {0x302D, 0x00}, //cannot find
        {IMX415_SYS_MODE, 0x05},
        {IMX415_SHR0_L, 0x08},
        {IMX415_SHR0_M, 0x00},
        {0x3054, 0x19}, //cannot find in spec, but is IMX415_SF1_EXPO_REG_L in rockchip
        {0x3058, 0x3E}, //cannot find in spec, but is IMX415_SF2_EXPO_REG_L in rockchip
        {0x3060, 0x25}, //cannot find in spec, but is IMX415_RHS1_REG_L     in rockchip
        {0x3064, 0x4a}, //maybe same          ,but is IMX415_RHS2_REG_L     in rockchip
        {0x30CF, 0x00}, //cannot find
        {IMX415_INCKSEL3_L, 0xC0},
        {0x3260, 0x01}, //cannot find, but is mentioned in the rockchip comments (set to 0x01 in normal mode, something else in hdr)
        {IMX415_INCKSEL6, 0x00},

        {IMX415_TCLKPOST, 0x7F},   //here applies the 0x00xx workaround
        {IMX415_TCLKPREPARE, 0x37},//here applies the 0x00xx workaround
        {IMX415_TCLKTRAIL, 0x37},  //here applies the 0x00xx workaround
        {IMX415_TCLKZERO_L, 0xF7}, //why the heck is this the only one of all where the higher bits need to be set to 0 argh
        {IMX415_TCLKZERO_H, 0x00}, // -- " --
        {IMX415_THSPREPARE, 0x3F}, //here applies the 0x00xx workaround
        {IMX415_THSZERO, 0x6F},    //here applies the 0x00xx workaround
        {IMX415_THSTRAIL, 0x3F},   //here applies the 0x00xx workaround
        {IMX415_THSEXIT, 0x5F},    //here applies the 0x00xx workaround
        {IMX415_TLPX, 0x2F},       //here applies the 0x00xx workaround
        {IMX415_INCKSEL7, 0x01},
        // added for testing Consti10:
        {IMX415_WINMODE,0x04}, //WINMODE //0: All-pixel mode, Horizontal/Vertical 2/2-line binning 4: Window cropping mode
        {IMX415_PIX_HST_L,IMX415_FETCH_16BIT_L(972)}, //PIX_HST Effective pixel Start position (Horizontal direction) | Default in spec: 0x000
        {IMX415_PIX_HST_H,IMX415_FETCH_16BIT_H(972)}, //""
        {IMX415_PIX_HWIDTH_L,IMX415_FETCH_16BIT_L(1920)}, //PIX_HWIDTH Effective pixel Cropping width (Horizontal direction) | Default in spec: 0x0F18==3864
        {IMX415_PIX_HWIDTH_H,IMX415_FETCH_16BIT_H(1920)},  //""
        {IMX415_PIX_VST_L,IMX415_FETCH_16BIT_L(556*2)}, //PIX_VST Effective pixel Star position (Vertical direction) Designated in V units ( Line×2 ) | Default in spec: 0x000
        {IMX415_PIX_VST_H,IMX415_FETCH_16BIT_H(556*2)}, //""
        {IMX415_PIX_VWIDTH_L,IMX415_FETCH_16BIT_L(1080*2)}, //PIX_VWIDTH Effective pixel Cropping width (Vertical direction) Designated in V units ( Line×2 ) | Default in spec: 0x1120==4384
        {IMX415_PIX_VWIDTH_H,IMX415_FETCH_16BIT_H(1080*2)}, //""
        /*{IMX415_WINMODE,0x04}, //WINMODE //0: All-pixel mode, Horizontal/Vertical 2/2-line binning 4: Window cropping mode
        {IMX415_PIX_HST_L,IMX415_FETCH_16BIT_L(0)}, //PIX_HST Effective pixel Start position (Horizontal direction) | Default in spec: 0x000
        {IMX415_PIX_HST_H,IMX415_FETCH_16BIT_H(0)}, //""
        {IMX415_PIX_HWIDTH_L,IMX415_FETCH_16BIT_L(3864)}, //PIX_HWIDTH Effective pixel Cropping width (Horizontal direction) | Default in spec: 0x0F18==3864
        {IMX415_PIX_HWIDTH_H,IMX415_FETCH_16BIT_H(3864)},  //""
        {IMX415_PIX_VST_L,IMX415_FETCH_16BIT_L(0)}, //PIX_VST Effective pixel Star position (Vertical direction) Designated in V units ( Line×2 ) | Default in spec: 0x000
        {IMX415_PIX_VST_H,IMX415_FETCH_16BIT_H(0)}, //""
        {IMX415_PIX_VWIDTH_L,IMX415_FETCH_16BIT_L(4384)}, //PIX_VWIDTH Effective pixel Cropping width (Vertical direction) Designated in V units ( Line×2 ) | Default in spec: 0x1120==4384
        {IMX415_PIX_VWIDTH_H,IMX415_FETCH_16BIT_H(4384)}, //"" */
        // added for testing Consti10 end

        {REG_NULL, 0x00},
};

//             | For 891Mbps & 37.125 | For 1485 & 37.125          | For 1782 & 37.125
//BCWAIT_TIME  | 07Fh                 | 07Fh                       | 07Fh
//CPWAIT_TIME  | 05Bh                 | 05Bh                       | 05Bh
//SYS_MODE     | 5h                   | 8h               SYS_MODE  | 4h
//INCKSEL1     | 00h                  | 00h                        | 00h
//INCKSEL2     | 24h                  | 24h                        | 24h
//INCKSEL3     | 0C0h                 | 0A0              INCKSEL3  | 0C0h
//INCKSEL4     | 0E0h                 | 0E0h                       | 0E0h
//INCKSEL5     | 24h                  | 24h                        | 24h
//TXCLKESC_FREQ| 0948h                | 0948h                      | 0948h
//INCKSEL6     | 0h                   | 1h               INCKSEL6  | 1h
//INCKSEL7     | 1h                   | 0h               INCKSEL7  | 0h

// 4k but in 1782Mhz mode (max 60 fps) - YEAH, works
static __maybe_unused const struct regval imx415_linear_10bit_3864x2192_1782_regs[] = {
        {IMX415_VMAX_L, 0xCA}, //maybe same
        {IMX415_VMAX_M, 0x08}, //maybe same
        {IMX415_HMAX_L,IMX415_FETCH_16BIT_L(0x226)},
        {IMX415_HMAX_H,IMX415_FETCH_16BIT_H(0x226)},
        {0x302C, 0x00}, //cannot find
        {0x302D, 0x00}, //cannot find
        {IMX415_SYS_MODE, 0x04},
        {IMX415_SHR0_L, 0x08},
        {IMX415_SHR0_M, 0x00},
        {0x3054, 0x19}, //cannot find in spec, but is IMX415_SF1_EXPO_REG_L in rockchip
        {0x3058, 0x3E}, //cannot find in spec, but is IMX415_SF2_EXPO_REG_L in rockchip
        {0x3060, 0x25}, //cannot find in spec, but is IMX415_RHS1_REG_L     in rockchip
        {0x3064, 0x4a}, //maybe same          ,but is IMX415_RHS2_REG_L     in rockchip
        {0x30CF, 0x00}, //cannot find
        {IMX415_INCKSEL3_L, 0xC0},
        {0x3260, 0x01}, //cannot find, but is mentioned in the rockchip comments (set to 0x01 in normal mode, something else in hdr)
        {IMX415_INCKSEL6, 0x01}, //changed

        {IMX415_TCLKPOST, 0xB7},   //here applies the 0x00xx workaround
        {IMX415_TCLKPREPARE, 0x67},//here applies the 0x00xx workaround
        {IMX415_TCLKTRAIL, 0x6F},  //here applies the 0x00xx workaround
        {IMX415_TCLKZERO_L, 0xDF}, //why the heck is this the only one of all where the higher bits need to be set to 0 argh
        {IMX415_TCLKZERO_H, 0x01}, // -- " --
        {IMX415_THSPREPARE, 0x6F}, //here applies the 0x00xx workaround
        {IMX415_THSZERO, 0xCF},    //here applies the 0x00xx workaround
        {IMX415_THSTRAIL, 0x6F},   //here applies the 0x00xx workaround
        {IMX415_THSEXIT, 0xB7},    //here applies the 0x00xx workaround
        {IMX415_TLPX, 0x5F},       //here applies the 0x00xx workaround
        {IMX415_INCKSEL7, 0x00}, //changed
        // added for testing Consti10:

        // added for testing Consti10 end

        {REG_NULL, 0x00},
};

// hmm still doesn't work properly, even with the modified imx307 file.
static __maybe_unused const struct regval imx415_linear_10bit_binning2x2_1782_regs[] = {
        {IMX415_VMAX_L, 0xCA}, //maybe same
        {IMX415_VMAX_M, 0x08}, //maybe same
        {IMX415_HMAX_L,IMX415_FETCH_16BIT_L(0x226)},//0x16D
        {IMX415_HMAX_H,IMX415_FETCH_16BIT_H(0x226)},
        {0x302C, 0x00}, //cannot find
        {0x302D, 0x00}, //cannot find
        {IMX415_SYS_MODE, 0x04},
        {IMX415_SHR0_L, 0x08},
        {IMX415_SHR0_M, 0x00},
        {0x3054, 0x19}, //cannot find in spec, but is IMX415_SF1_EXPO_REG_L in rockchip
        {0x3058, 0x3E}, //cannot find in spec, but is IMX415_SF2_EXPO_REG_L in rockchip
        {0x3060, 0x25}, //cannot find in spec, but is IMX415_RHS1_REG_L     in rockchip
        {0x3064, 0x4a}, //maybe same          ,but is IMX415_RHS2_REG_L     in rockchip
        {0x30CF, 0x00}, //cannot find
        {IMX415_INCKSEL3_L, 0xC0},
        {0x3260, 0x01}, //cannot find, but is mentioned in the rockchip comments (set to 0x01 in normal mode, something else in hdr)
        {IMX415_INCKSEL6, 0x01}, //changed

        {IMX415_TCLKPOST, 0xB7},   //here applies the 0x00xx workaround
        {IMX415_TCLKPREPARE, 0x67},//here applies the 0x00xx workaround
        {IMX415_TCLKTRAIL, 0x6F},  //here applies the 0x00xx workaround
        {IMX415_TCLKZERO_L, 0xDF}, //why the heck is this the only one of all where the higher bits need to be set to 0 argh
        {IMX415_TCLKZERO_H, 0x01}, // -- " --
        {IMX415_THSPREPARE, 0x6F}, //here applies the 0x00xx workaround
        {IMX415_THSZERO, 0xCF},    //here applies the 0x00xx workaround
        {IMX415_THSTRAIL, 0x6F},   //here applies the 0x00xx workaround
        {IMX415_THSEXIT, 0xB7},    //here applies the 0x00xx workaround
        {IMX415_TLPX, 0x5F},       //here applies the 0x00xx workaround
        {IMX415_INCKSEL7, 0x00}, //changed
        // added for testing Consti10:
        {0x301C,0x00}, //WINMODE //0: All-pixel mode, Horizontal/Vertical 2/2-line binning 4: Window cropping mode
        {0x3020,0x01}, //HADD //0h: All-pixel mode 1h: Horizontal 2 binning
        {0x3021,0x01}, //VADD //0h: All-pixel mode 1h: Vertical 2 binning
        {0x3022,0x01}, //ADDMODE //0h: All-pixel mode 1h: Horizontal/Vertical 2/2-line binning
        //
        // to resolve:
        {0x3031,0x00}, //ADBIT //set by global to 0 , 0=10bit 1=12bit
        {0x3032,0x00}, //MDBIT //set by global to 0
        //
        {0x30D9,0x02}, //DIG_CLP_VSTAET ? 0x02=binning 0x06=All-pixel scan mode , default 0x06
        {0x30DA,0x01}, //DIG_CLP_VNUM ? 0x01=binning 0x02=all-pixel scan mode, default 0x02
        // added for testing Consti10 end

        {REG_NULL, 0x00},
};

// the "image" schematics
//    res     | Horizontal   | Vertical
// 3840x2160  | 12+12=24     | 1+12+8+8+2+1= 32
// 1920x1080  | 6+6+12=24    | 1+6+4+4+1+1=  17

// PIX_VWIDTH
//V TTL (1farame line length or VMAX) ≥ (PIX_VWIDTH / 2) + 46
// (4384 /2) +46 = 2238

//Also: In all pixel mode,, 4 lane, 891 mbpp
// VMAX: 0x8CA = 2250
// and rockchip used .vts_def = 0x08ca
// HMAX: 44Ch = 1100
// and rockchip uses .hts_def = 0x044c * IMX415_4LANES * 2

// Whereas in Horizontal/Vertical 2/2-line binning mode, 4 lane:
// VMAX: 0x8CA (SAME!)
// HMAX:44Ch (SAME!)

// Frame rate on Window cropping mode
// Frame rate [frame/s] = 1 / (V TTL × (1H period))
/// 1H period (unit: [s]) : Set "1H period" or more in the table of "Operating mode" before cropping mode.
// 1/2250*14.9*1e-6= 6.62222222222
// 1/4384*(14.9*10^6)=3398.72262774
#endif //MEDIASEVER_IMX415_REGS_ROCKCHIP_H
