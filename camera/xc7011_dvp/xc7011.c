#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "xc7011.h"

static u32 reset_gpio[2] = {-1, -1};
static u32 pwdn_gpios[2] = {-1, -1};
static void *iic = NULL;
typedef struct {
    u16 addr;
    u8 value;
} Sensor_reg_ini;

typedef struct {
    u8 addr;
    u8 value;
} Sensor_reg_inix;

#define  XC7011_WRCMD  0x36
#define  XC7011_RDCMD  0x37
#define  H42_WRCMD  0x60
#define  H42_RDCMD  0x61

unsigned char wrXC7011Reg(unsigned short regID, unsigned char regDat)
{
    u8 ret = 1;
    u8 high, low;

    high = regID >> 8;
    low = regID & 0xff;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT,	XC7011_WRCMD)) {
        ret = 0;
        puts("wr err 0\n");
        goto __wend;
    }
    if (dev_ioctl(iic, IIC_IOCTL_TX, high)) {
        ret = 0;
        puts("wr err 1\n");
        goto __wend;
    }

    if (dev_ioctl(iic, IIC_IOCTL_TX, low)) {
        ret = 0;
        puts("wr err 2\n");
        goto __wend;
    }
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        ret = 0;
        puts("wr err 3\n");
        goto __wend;
    }

__wend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;
}

unsigned char rdXC7011Reg(unsigned short regID, unsigned char *regDat)
{
    u8 ret = 1;
    u8 high, low;

    high = regID >> 8;
    low = regID & 0xff;
    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, XC7011_WRCMD)) {
        ret = 0;
        puts("rd err 0\n");
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX, high)) {
        puts("rd err 1\n");
        ret = 0;
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX, low)) {
        ret = 0;
        puts("rd err 2\n");
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, XC7011_RDCMD)) {
        ret = 0;
        puts("rd err 3\n");
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat)) {
        ret = 0;
        puts("rd err 4\n");
        goto __rend;
    }
__rend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;

}


static unsigned char wrH42Reg(unsigned char regID, unsigned char regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, H42_WRCMD)) {
        ret = 0;
        goto __wend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID)) {
        ret = 0;
        goto __wend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        ret = 0;
        goto __wend;
    }
__wend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;
}

static unsigned char rdH42Reg(unsigned char regID, unsigned char *regDat)
{
    u8 ret = 1;
    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, H42_WRCMD)) {
        ret = 0;
        goto __rend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regID)) {
        ret = 0;
        goto __rend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, H42_RDCMD)) {
        ret = 0;
        goto __rend;
    }

    delay(100);

    dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat);

__rend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);

    return ret;

}



Sensor_reg_ini XC7011_default_regs[] = {



    0xfffd, 0x80,
    0xfffe, 0x50,
    0x001c, 0xff,
    0x001d, 0xff,
    0x001e, 0xff,
    0x001f, 0xff, //clk en
    0x0018, 0x00,
    0x0019, 0x00,
    0x001a, 0x00,
    0x001b, 0x00,  // reset
    0x00bc, 0x11,
    0x00bd, 0x00,
    0x00be, 0x00,
    0x00bf, 0x00,

    0x0030, 0x09, //
    0x0032, 0x0c,
    0x0027, 0x06,
    0x0026, 0x01,
    0x0021, 0x0e,
    0x0020, 0x02,

    0xfffe, 0x50,
    0x0028, 0x08,
    0x0029, 0x06,
    0xfffe, 0x25,
    0x200b, 0x33, //de-glitch
    0x0002, 0x40,

    0xfffe, 0x50,
    0x0200, 0x03,
    0x0204, 0x03,
    0x0208, 0x02,
    0x0208, 0x01,


    0xfffd, 0x80, //datapath start
    0xfffe, 0x50,
    0x00bc, 0x11,
    0x001b, 0x0,
    0x0090, 0x29,
    /* 0x0090,0x2b, *///color bar

    0xfffe, 0x20,
    0x0000, 0x20,
    0x0004, 0x5,
    0x0005, 0x0,
    0x0006, 0x2,
    0x0007, 0xD0,

    0xfffe, 0x26,
    0x4000, 0xF9,
    0x6001, 0x14,
    0x6005, 0xc4,
    0x6006, 0xA,
    0x6007, 0x8C,
    0x6008, 0x9,
    0x6009, 0xFC,

    0x8000, 0x3f,
    0x8001, 0x0,
    0x8002, 0x5,
    0x8003, 0xD0,
    0x8004, 0x2,
    0x8005, 0x3,
    0x8006, 0x5,
    0x8007, 0x99,

    0x8010, 0x4,
    0x8012, 0x0,
    0x8013, 0x5,
    0x8014, 0xD0,
    0x8015, 0x2,
    0x8016, 0x0,
    0x8017, 0x0,
    0x8018, 0x0,
    0x8019, 0x0,

    0xfffe, 0x30,
    0x0001, 0x85,
    0x0004, 0x18,
    0x0006, 0x5,
    0x0007, 0x0,
    0x0008, 0x2,
    0x0009, 0xD0,
    0x000a, 0x5,
    0x000b, 0x0,
    0x000c, 0x2,
    0x000d, 0xD0,
    0x0027, 0xF1,
    0x005e, 0x4,
    0x005f, 0xFF,
    0x0060, 0x2,
    0x0061, 0xCF,
    0x1908, 0x0,
    0x1900, 0x0,
    0x1901, 0x0,
    0x1902, 0x0,
    0x1903, 0x0,
    0x1904, 0x5,
    0x1905, 0x0,
    0x1906, 0x2,
    0x1907, 0xD0, //datapath end

    0xfffe, 0x30, //AVG
    0x1f00, 0x00,
    0x1f01, 0x00,
    0x1f02, 0x00,
    0x1f03, 0x00,
    0x1f04, 0x05,
    0x1f05, 0x00,
    0x1f06, 0x02,
    0x1f07, 0xd0,


    0xfffe, 0x14,
    0x0026, 0x01,
    0x00be, 0x60,  	//camera i2c id
    0x00bf, 0x00,  	//camera i2c bits
    0x00c0, 0x06,  	//sensor type gain
    0x00c1, 0x02,  	//sensor type exposure
    0x00c3, 0x00,   //sensor mode select


    0x00c4, 0x00, //write camera exposure variable [15:12]
    0x00c5, 0x02,
    0x00c6, 0x00, //write camera exposure variable [11:4]
    0x00c7, 0x01,
    0x00c8, 0x00, //write camera exposure variable [3:0]
    0x00c9, 0x00,
    0x00cc, 0x00, //camera exposure addr mask 1
    0x00cd, 0xff,
    0x00ce, 0x00, //camera exposure addr mask 2
    0x00cf, 0xff,
    0x00d0, 0x00, //camera exposure addr mask 3
    0x00d1, 0x00,
    0x00d2, 0x00, //camera exposure addr mask 4
    0x00d3, 0x00,


    0x00e4, 0x00, //camera gain addr
    0x00e5, 0x00,
    0x00ec, 0x00, //camera gain addr mask 1
    0x00ed, 0xff,
    0x00ee, 0x00, //camera gain addr mask 2
    0x00ef, 0x00,
    0x00f0, 0x00, //camera gain addr mask 3
    0x00f1, 0x00,
    0x00f2, 0x00, //camera gain addr mask 4
    0x00f3, 0x00,


    0x00b4, 0x02,   // banding mode
    0x00b8, 0x0e,   // banding  exposure lines
    0x00b9, 0x10,

    0x0035, 0x01,


    0xfffe, 0x14,
    0x00aa, 0x01,
    0x00ab, 0x30, //017f //max gain
    0x00ba, 0x0,  //low light mode
    0x006a, 0x01,
    0x006b, 0x40, //0140 //night target
    0x006c, 0x01,
    0x006d, 0x40, //0120 //day target
    0x0073, 0x08,  //AttAvgReviseDiff
    0x0092, 0x3e,
    0x0093, 0x00,


    0xfffe, 0x14,
    0x0039, 0x0,  //DETECT enable
    0x0060, 0x0,  //smart mode 03

    0x0076, 0x1,   // delay frame
    0x0079, 0x01,
    0x007a, 0x00,
    0x007b, 0x28,		//1. threshold low
    0x007c, 0x00,
    0x007d, 0xc0,  //2. threshold high
    0x0077, 0x10,   //3. finally thr
    0x0078, 0x01,   //total speed
    0x007f, 0x80, // jump threshold
    0x0087, 0x60, // jump ratio
    0x0088, 0x02, // jump times
    0x008c, 0x02,
    0x008d, 0xc0,


    0x3f, 0x01,
    0x40, 0x01,
    0x41, 0x01,
    0x42, 0x01,
    0x43, 0x01,
    0x44, 0x01,
    0x45, 0x01,
    0x46, 0x01,
    0x47, 0x01,
    0x48, 0x01,
    0x49, 0x01,
    0x4a, 0x40,
    0x4b, 0x40,
    0x4c, 0x40,
    0x4d, 0x01,
    0x4e, 0x01,
    0x4f, 0x40,
    0x50, 0x40,
    0x51, 0x40,
    0x52, 0x01,
    0x53, 0x01,
    0x54, 0x01,
    0x55, 0x01,
    0x56, 0x01,
    0x57, 0x01,
    0x3a, 0x01,


    0x0110, 0x01,
    0x0112, 0x03,  //Att Detect Speed
    0x0113, 0x3,
    0x0114, 0x3, 	// below or above cnt
    0x011a, 0x00,
    0x011b, 0x60,  //day min attention  avg thr
    0x011c, 0x02,
    0x011d, 0x80,  //day max attention  avg thr
    0x011e, 0x00,
    0x011f, 0x40,  //day min avg thr
    0x0120, 0x02,
    0x0121, 0xc0,  //day max avg thr
    0x0062, 0x00,
    0x0063, 0x60,   // D-N
    0x0064, 0x00,
    0x0065, 0x40,   // N-D
    0x0066, 0x01,
    0x0067, 0x00,   //dark thr
    0x0074, 0x20,   //diff ratio low
    0x0075, 0x40,   //diff ratio high
    0x006e, 0x6,   //magnituede offset
    0x006f, 0x1,   //magnituede MIN
    0x012a, 0x05,  //variance thr promote luma
    0x012b, 0x00,
    0x012c, 0x00,  //variance ratio
    0x012d, 0x30,

    ///////////////////////////////////////
    /////   patch   start   ///////////////
    ///////////////////////////////////////
    0xfffd, 0x80,
    0xfffe, 0x50,
    0x000e, 0x54,
    0xfffe, 0x14,
    0x0006, 0x1,
    0x0007, 0xef,
    0x0014, 0x00,
    0x0015, 0x14,
    0x0016, 0x5,
    0x0017, 0x8c,
    0x3e0, 0x9c,
    0x3e1, 0x21,
    0x3e2, 0xff,
    0x3e3, 0xf8,
    0x3e4, 0xd4,
    0x3e5, 0x1,
    0x3e6, 0x48,
    0x3e8, 0xd4,
    0x3e9, 0x1,
    0x3ea, 0x50,
    0x3eb, 0x4,
    0x3ec, 0x18,
    0x3ed, 0x60,
    0x3ef, 0x14,
    0x3f0, 0xa8,
    0x3f1, 0x63,
    0x3f2, 0x1,
    0x3f3, 0xd0,
    0x3f4, 0x19,
    0x3f5, 0x60,
    0x3f7, 0x14,
    0x3f8, 0xa9,
    0x3f9, 0x6b,
    0x3fa, 0x3,
    0x3fb, 0xd4,
    0x3fc, 0x94,
    0x3fd, 0xa3,
    0x400, 0x19,
    0x401, 0xa0,
    0x403, 0x14,
    0x404, 0xa9,
    0x405, 0xad,
    0x406, 0x3,
    0x407, 0xdc,
    0x408, 0x85,
    0x409, 0xb,
    0x40c, 0x18,
    0x40d, 0x60,
    0x40f, 0x14,
    0x410, 0xa8,
    0x411, 0x63,
    0x412, 0x2,
    0x413, 0xb0,
    0x414, 0xb8,
    0x415, 0xe5,
    0x417, 0x44,
    0x418, 0x85,
    0x419, 0x43,
    0x41c, 0x9c,
    0x41d, 0x80,
    0x41f, 0x3,
    0x420, 0x8c,
    0x421, 0xaa,
    0x423, 0x3a,
    0x424, 0x84,
    0x425, 0x6d,
    0x428, 0xe0,
    0x429, 0xe7,
    0x42a, 0x28,
    0x42b, 0x2,
    0x42c, 0x8c,
    0x42d, 0xca,
    0x42f, 0x3c,
    0x430, 0x18,
    0x431, 0xa0,
    0x433, 0x14,
    0x434, 0xa8,
    0x435, 0xa5,
    0x436, 0x3,
    0x437, 0xd8,
    0x438, 0xd4,
    0x439, 0xb,
    0x43a, 0x18,
    0x43c, 0xe5,
    0x43d, 0xa6,
    0x43e, 0x38,
    0x440, 0x10,
    0x443, 0x3,
    0x444, 0xd4,
    0x445, 0x5,
    0x446, 0x40,
    0x448, 0xa8,
    0x449, 0xc7,
    0x44c, 0x8c,
    0x44d, 0xaa,
    0x44f, 0x3b,
    0x450, 0xe5,
    0x451, 0x66,
    0x452, 0x28,
    0x454, 0x10,
    0x457, 0x3,
    0x458, 0x15,
    0x45c, 0xa8,
    0x45d, 0xc5,
    0x460, 0xe0,
    0x461, 0x66,
    0x462, 0x18,
    0x464, 0xd4,
    0x465, 0xd,
    0x466, 0x30,
    0x468, 0x7,
    0x469, 0xfb,
    0x46a, 0x1c,
    0x46b, 0xd1,
    0x46c, 0xe0,
    0x46d, 0x63,
    0x46e, 0x40,
    0x470, 0x9c,
    0x471, 0x60,
    0x472, 0x1,
    0x474, 0xdc,
    0x475, 0xa,
    0x476, 0x58,
    0x477, 0xfa,
    0x478, 0xe0,
    0x479, 0x83,
    0x47a, 0x58,
    0x47b, 0x2,
    0x47c, 0x18,
    0x47d, 0xc0,
    0x47f, 0x14,
    0x480, 0xa8,
    0x481, 0xc6,
    0x482, 0x1,
    0x483, 0xe0,
    0x484, 0x94,
    0x485, 0xa6,
    0x488, 0x94,
    0x489, 0x66,
    0x48b, 0x2,
    0x48c, 0xe0,
    0x48d, 0x84,
    0x48e, 0x2b,
    0x48f, 0x6,
    0x490, 0xe1,
    0x491, 0x63,
    0x492, 0x5b,
    0x493, 0x6,
    0x494, 0xe1,
    0x495, 0x6b,
    0x496, 0x20,
    0x498, 0xb9,
    0x499, 0x6b,
    0x49b, 0x48,
    0x49c, 0xdc,
    0x49d, 0xa,
    0x49e, 0x58,
    0x49f, 0xc4,
    0x4a0, 0x85,
    0x4a1, 0x21,
    0x4a4, 0x85,
    0x4a5, 0x41,
    0x4a7, 0x4,
    0x4a8, 0x44,
    0x4aa, 0x48,
    0x4ac, 0x9c,
    0x4ad, 0x21,
    0x4af, 0x8,
    0x4b0, 0x18,
    0x4b1, 0x80,
    0x4b3, 0x14,
    0x4b4, 0xa8,
    0x4b5, 0x84,
    0x4b6, 0x2,
    0x4b7, 0xcc,
    0x4b8, 0x84,
    0x4b9, 0x64,
    0x4bc, 0xbc,
    0x4bd, 0x23,
    0x4bf, 0x41,
    0x4c0, 0xc,
    0x4c3, 0x6,
    0x4c4, 0xbc,
    0x4c5, 0x23,
    0x4c7, 0x3f,
    0x4c8, 0x10,
    0x4cb, 0x6,
    0x4cc, 0x9c,
    0x4cd, 0x60,
    0x4cf, 0x3e,
    0x4d3, 0x4,
    0x4d4, 0xd4,
    0x4d5, 0x4,
    0x4d6, 0x18,
    0x4d8, 0x9c,
    0x4d9, 0x60,
    0x4db, 0x42,
    0x4dc, 0xd4,
    0x4dd, 0x4,
    0x4de, 0x18,
    0x4e0, 0x44,
    0x4e2, 0x48,
    0x4e4, 0x15,
    0x4e8, 0x9c,
    0x4e9, 0x21,
    0x4ea, 0xff,
    0x4eb, 0xfc,
    0x4ec, 0xd4,
    0x4ed, 0x1,
    0x4ee, 0x48,
    0x4f0, 0x19,
    0x4f1, 0xa0,
    0x4f3, 0x14,
    0x4f4, 0xa9,
    0x4f5, 0xad,
    0x4f6, 0x2,
    0x4f7, 0xb0,
    0x4f8, 0x9d,
    0x4f9, 0x60,
    0x4fc, 0x84,
    0x4fd, 0x6d,
    0x500, 0x84,
    0x501, 0x63,
    0x503, 0xd0,
    0x504, 0xb8,
    0x505, 0x83,
    0x507, 0x41,
    0x50b, 0x4,
    0x50c, 0xb8,
    0x50d, 0xc3,
    0x50f, 0x45,
    0x510, 0xb8,
    0x511, 0xc6,
    0x513, 0x41,
    0x514, 0xa5,
    0x515, 0x63,
    0x516, 0xff,
    0x517, 0xff,
    0x518, 0xbc,
    0x519, 0xa6,
    0x51b, 0x1,
    0x51c, 0xf,
    0x51d, 0xff,
    0x51e, 0xff,
    0x51f, 0xfd,
    0x520, 0x9c,
    0x521, 0x6b,
    0x523, 0x1,
    0x524, 0xe0,
    0x525, 0x64,
    0x526, 0x58,
    0x527, 0x48,
    0x528, 0x9c,
    0x529, 0x63,
    0x52a, 0xff,
    0x52b, 0xf0,
    0x52c, 0xbc,
    0x52d, 0x3,
    0x530, 0x10,
    0x533, 0xa,
    0x534, 0xa8,
    0x535, 0xe3,
    0x538, 0x84,
    0x539, 0x6d,
    0x53c, 0x94,
    0x53d, 0x63,
    0x53f, 0xfc,
    0x540, 0xe0,
    0x541, 0x63,
    0x542, 0x38,
    0x544, 0xbc,
    0x545, 0xa3,
    0x547, 0xf,
    0x548, 0x10,
    0x54b, 0x4,
    0x54c, 0xa8,
    0x54d, 0xe3,
    0x550, 0x9c,
    0x551, 0x60,
    0x553, 0xf,
    0x554, 0xa8,
    0x555, 0xe3,
    0x558, 0x9c,
    0x559, 0x60,
    0x55b, 0x1,
    0x55c, 0x84,
    0x55d, 0x8d,
    0x560, 0xe0,
    0x561, 0x63,
    0x562, 0x58,
    0x563, 0x8,
    0x564, 0x8c,
    0x565, 0xa4,
    0x567, 0x8b,
    0x568, 0x9d,
    0x569, 0x3,
    0x56a, 0xff,
    0x56b, 0xff,
    0x56c, 0x94,
    0x56d, 0x64,
    0x56f, 0xb0,
    0x570, 0xb8,
    0x571, 0xc8,
    0x573, 0x4,
    0x574, 0xe0,
    0x575, 0x86,
    0x576, 0x38,
    0x578, 0x7,
    0x579, 0xfb,
    0x57a, 0x19,
    0x57b, 0xb7,
    0x57c, 0xa4,
    0x57d, 0x84,
    0x57f, 0xff,
    0x580, 0x85,
    0x581, 0x21,
    0x584, 0x44,
    0x586, 0x48,
    0x588, 0x9c,
    0x589, 0x21,
    0x58b, 0x4,
    0x58c, 0x9c,
    0x58d, 0x21,
    0x58e, 0xff,
    0x58f, 0xf4,
    0x590, 0xd4,
    0x591, 0x1,
    0x592, 0x48,
    0x594, 0xd4,
    0x595, 0x1,
    0x596, 0x50,
    0x597, 0x4,
    0x598, 0xd4,
    0x599, 0x1,
    0x59a, 0x60,
    0x59b, 0x8,
    0x59c, 0xbc,
    0x59d, 0x3,
    0x59e, 0x1,
    0x59f, 0xc,
    0x5a0, 0x10,
    0x5a3, 0x15,
    0x5a4, 0xbc,
    0x5a5, 0x43,
    0x5a6, 0x1,
    0x5a7, 0xc,
    0x5a8, 0xc,
    0x5ab, 0xb,
    0x5ac, 0xbc,
    0x5ad, 0x3,
    0x5ae, 0x1,
    0x5af, 0x5,
    0x5b0, 0xbc,
    0x5b1, 0x3,
    0x5b2, 0x1,
    0x5b3, 0x10,
    0x5b4, 0x10,
    0x5b7, 0x16,
    0x5b8, 0xbc,
    0x5b9, 0x3,
    0x5ba, 0x4,
    0x5bb, 0xc,
    0x5bc, 0x19,
    0x5bd, 0x80,
    0x5bf, 0x14,
    0x5c0, 0xa9,
    0x5c1, 0x8c,
    0x5c2, 0x3,
    0x5c3, 0x18,
    0x5c4, 0xc,
    0x5c7, 0x29,
    0x5c8, 0x15,
    0x5cf, 0x25,
    0x5d0, 0x15,
    0x5d4, 0x19,
    0x5d5, 0x80,
    0x5d7, 0x14,
    0x5d8, 0xa9,
    0x5d9, 0x8c,
    0x5da, 0x3,
    0x5db, 0x18,
    0x5dc, 0xc,
    0x5df, 0x23,
    0x5e0, 0x15,
    0x5e4, 0x7,
    0x5e5, 0xff,
    0x5e6, 0xff,
    0x5e7, 0x7f,
    0x5e8, 0x15,
    0x5ef, 0x20,
    0x5f0, 0x84,
    0x5f1, 0x6c,
    0x5f4, 0x19,
    0x5f5, 0x80,
    0x5f7, 0x14,
    0x5f8, 0xa9,
    0x5f9, 0x8c,
    0x5fa, 0x3,
    0x5fb, 0x18,
    0x5fc, 0x7,
    0x5fd, 0xff,
    0x5fe, 0xff,
    0x5ff, 0xad,
    0x600, 0x15,
    0x607, 0x1a,
    0x608, 0x84,
    0x609, 0x6c,
    0x60c, 0x18,
    0x60d, 0x60,
    0x60f, 0x14,
    0x610, 0xa8,
    0x611, 0x63,
    0x612, 0x2,
    0x613, 0xb0,
    0x614, 0x19,
    0x615, 0x40,
    0x617, 0x14,
    0x618, 0xa9,
    0x619, 0x4a,
    0x61a, 0x2,
    0x61b, 0xf8,
    0x61c, 0x84,
    0x61d, 0x83,
    0x620, 0x84,
    0x621, 0xaa,
    0x624, 0x94,
    0x625, 0x64,
    0x627, 0x7e,
    0x628, 0xe4,
    0x629, 0xa3,
    0x62a, 0x28,
    0x62c, 0x10,
    0x62f, 0x4,
    0x630, 0x15,
    0x634, 0x9c,
    0x635, 0x60,
    0x637, 0x4,
    0x638, 0xd8,
    0x639, 0x4,
    0x63a, 0x18,
    0x63b, 0xd7,
    0x63c, 0x19,
    0x63d, 0x80,
    0x63f, 0x14,
    0x640, 0xa9,
    0x641, 0x8c,
    0x642, 0x3,
    0x643, 0x18,
    0x644, 0x84,
    0x645, 0x6c,
    0x648, 0x7,
    0x649, 0xfb,
    0x64a, 0x5,
    0x64b, 0xf8,
    0x64c, 0x84,
    0x64d, 0x63,
    0x64e, 0x1,
    0x64f, 0xc0,
    0x650, 0x9c,
    0x651, 0x80,
    0x652, 0xff,
    0x653, 0xf0,
    0x654, 0xe1,
    0x655, 0x6b,
    0x656, 0x20,
    0x657, 0x3,
    0x65b, 0x4,
    0x65c, 0xd4,
    0x65d, 0xa,
    0x65e, 0x58,
    0x660, 0x7,
    0x661, 0xff,
    0x662, 0xff,
    0x663, 0xa2,
    0x664, 0x15,
    0x668, 0x84,
    0x669, 0x6c,
    0x66c, 0x9d,
    0x66d, 0x60,
    0x670, 0xd4,
    0x671, 0x3,
    0x672, 0x59,
    0x673, 0xcc,
    0x674, 0xd4,
    0x675, 0x3,
    0x676, 0x59,
    0x677, 0xc0,
    0x678, 0xd4,
    0x679, 0x3,
    0x67a, 0x59,
    0x67b, 0xc4,
    0x67c, 0xd4,
    0x67d, 0x3,
    0x67e, 0x59,
    0x67f, 0xc8,
    0x680, 0x85,
    0x681, 0x21,
    0x684, 0x85,
    0x685, 0x41,
    0x687, 0x4,
    0x688, 0x85,
    0x689, 0x81,
    0x68b, 0x8,
    0x68c, 0x44,
    0x68e, 0x48,
    0x690, 0x9c,
    0x691, 0x21,
    0x693, 0xc,

    //patch configure   /////////////////
    0x06e, 0x30,  //magnitude offset
    0x06f, 0x20,  //magnitude Min
    0x070, 0xc0,  //magnitude MAX
    //////ENABLE////////////////////////////
    0xfffe, 0x30,
    0x0051, 0x03,

    0xfffe, 0x50,
    0x0137, 0x99,
    0xfffe, 0x14,

    ///////////////////////////////////////
    /////   patch   end     ///////////////
    ///////////////////////////////////////
    /* ND */


    0xfffe, 0x30, //lenc_200_80%
    0x0200, 0x0,
    0x0201, 0x0,
    0x0202, 0x0,
    0x0203, 0x0,
    0x0204, 0x0,
    0x0205, 0x0,
    0x0206, 0x0,
    0x0207, 0x0,
    0x0208, 0x0,
    0x0209, 0x0,
    0x020a, 0x0,
    0x020b, 0x0,
    0x020c, 0x0,
    0x020d, 0x0,
    0x020e, 0x0,
    0x020f, 0x0,
    0x0210, 0x0,
    0x0211, 0x0,
    0x0212, 0x0,
    0x0213, 0x0,
    0x0214, 0x0,
    0x0215, 0x0,
    0x0216, 0x0,
    0x0217, 0x0,
    0x0218, 0x0,
    0x0219, 0x0,
    0x021a, 0x0,
    0x021b, 0x0,
    0x021c, 0x0,
    0x021d, 0x0,
    0x021e, 0x0,
    0x021f, 0x0,
    0x0220, 0x0,
    0x0221, 0x0,
    0x0222, 0x0,
    0x0223, 0x0,
    0x0224, 0x22,
    0x0225, 0x12,
    0x0226, 0x22,
    0x0227, 0x12,
    0x0228, 0x2,
    0x022a, 0x31,
    0x022b, 0x42,
    0x022c, 0x53,
    0x022d, 0x42,
    0x022e, 0x21,
    0x0230, 0x31,
    0x0231, 0x42,
    0x0232, 0x52,
    0x0233, 0x42,
    0x0234, 0x20,
    0x0236, 0x22,
    0x0237, 0x32,
    0x0238, 0x43,
    0x0239, 0x33,
    0x023a, 0x11,
    0x023c, 0x13,
    0x023d, 0x11,
    0x023e, 0x21,
    0x023f, 0x21,
    0x0240, 0x23,
    0x0248, 0xce,

    0xfffe, 0x30,
    0x024d, 0x1,
    0x024e, 0x33,
    0x024f, 0x2,
    0x250, 0x22,
    0x251, 0x1,
    0x252, 0x99,
    0x253, 0x1,
    0x254, 0x6C,


    0xfffe, 0x14,  //AWB_init
    0x0027, 0x00, //firmware中 awb_enable
    0x013c, 0x02, //0.AWB_ARITH_ORIGIN  1.AWB_SW_PRO  2.AWB_ARITH_MANUAL
    0x0176, 0x06,  //int B gain
    0x0177, 0x00,
    0x017a, 0x04, //int Gb gain
    0x017b, 0x00,
    0x017e, 0x04,  //int Gr gain
    0x017f, 0x00,
    0x0182, 0x04,  //int R gain
    0x0183, 0x04,
    0x01aa, 0x06,   //B_temp
    0x01ab, 0x00,
    0x01ae, 0x04,   //G_ temp
    0x01af, 0x00,
    0x01b2, 0x04,  //R_temp
    0x01b3, 0x04,

    0xfffe, 0x14,  //C_AWB
    0x0027, 0x01, //firmware中 awb_enable
    0x013c, 0x01, //0.AWB_ARITH_ORIGIN  1.AWB_SW_PRO  2.AWB_ARITH_MANUAL
    0x013d, 0x01, //AWBFlexiMap_en
    0x013e, 0x00, //AWBMove_en
    0x0170, 0x0d, //nMaxAwbGain
    0x0171, 0xff,

    0x0708, 0x03, //pixel max value[9:8]
    0x0709, 0xf0, //pixel max value[7:0]
    0x070a, 0x00, //pixel min value[9:8]
    0x070b, 0x0c, //pixel min value[7:0]
    0x0001, 0x67,
    0x0003, 0xe5,
    0x0051, 0x03,
    0x0096, 0x83,
    0x0019, 0x48,
    0x071c, 0x0a, //Simple_awb_en[4]: 0’b0: simple awb mode disenable 1’b1: simple awb mode enable

    0xfffe, 0x14,
    0x016e, 0x08,

    0xfffd, 0x80, //AWB_4
    0xfffe, 0x30,
    0x0000, 0x4f,
    0x0001, 0x85,
    0x0003, 0xe5,

    0x0730, 0x2a, // win1 startx
    0x0731, 0x88, // win1 endx
    0x0732, 0x6d, // win1 starty
    0x0733, 0xa0, // win1 endy
    0x0734, 0x60, // win2 startx
    0x0735, 0xa0, // win2 endx
    0x0736, 0x60, //60 // win2 starty
    0x0737, 0x80, // win2 endy
    0x0738, 0x40, // win3 startx
    0x0739, 0xb8, // win3 endx
    0x073a, 0x50, // win3 starty
    0x073b, 0x70, // win3 endy
    0x073c, 0x68, // win4 startx
    0x073d, 0xc8, // win4 endx
    0x073e, 0x34, // win4 starty
    0x073f, 0x55, // win4 endy
    0x0740, 0x80, // win5 startx
    0x0741, 0xe0, // win5 endx
    0x0742, 0x28, // win5 starty
    0x0743, 0x40, // win5 endy
    0x0744, 0x00, // win6 startx
    0x0745, 0x00, // win6 endx
    0x0746, 0x00, // win6 starty
    0x0747, 0x00, // win6 endy
    0x0748, 0x00, // win7 startx
    0x0749, 0x00, // win7 endx
    0x074a, 0x00, // win7 starty
    0x074b, 0x00, // win7 endy
    0x074c, 0x00, // win8 startx
    0x074d, 0x00, // win8 endx
    0x074e, 0x00, // win8 starty
    0x074f, 0x00, // win8 endy
    0x0750, 0x00, // win9 startx
    0x0751, 0x00, // win9 endx
    0x0752, 0x00, // win9 starty
    0x0753, 0x00, // win9 endy
    0x0754, 0x00, // win10 startx
    0x0755, 0x00, // win10 endx
    0x0756, 0x00, // win10 starty
    0x0757, 0x00, // win10 endy
    0x0758, 0x00, // win11 startx
    0x0759, 0x00, // win11 endx
    0x075a, 0x00, // win11 starty
    0x075b, 0x00, // win11 endy
    0x075c, 0x00, // win12 startx
    0x075d, 0x00, // win12 endx
    0x075e, 0x00, // win12 starty
    0x075f, 0x00, // win12 endy
    0x0760, 0x00, // win13 startx
    0x0761, 0x00, // win13 endx
    0x0762, 0x00, // win13 starty
    0x0763, 0x00, // win13 endy
    0x0764, 0x00, // win14 startx
    0x0765, 0x00, // win14 endx
    0x0766, 0x00, // win14 starty
    0x0767, 0x00, // win14 endy
    0x0768, 0x00, // win15 startx
    0x0769, 0x00, // win15 endx
    0x076a, 0x00, // win15 starty
    0x076b, 0x00, // win15 endy
    0x076c, 0x00, // win16 startx
    0x076d, 0x00, // win16 endx
    0x076e, 0x00, // win16 starty
    0x076f, 0x00, // win16 endy
    0x0770, 0x11, // wt1 wt2
    0x0771, 0x11, // wt3 wt4
    0x0772, 0x10, // wt5 wt6
    0x0773, 0x00, // wt7 wt8
    0x0774, 0x00, // wt9 wt10
    0x0775, 0x00, // wt11 wt12
    0x0776, 0x00, // wt13 wt14
    0x0777, 0x00, // wt15 wt16


    0xfffd, 0x80, //Nathan5
    0xfffe, 0x30,
    0x1400, 0xA,
    0x1401, 0x11,
    0x1402, 0x14,
    0x1403, 0x19,
    0x1404, 0x1F,
    0x1405, 0x24,
    0x1406, 0x29,
    0x1407, 0x2E,
    0x1408, 0x33,
    0x1409, 0x38,
    0x140a, 0x3C,
    0x140b, 0x41,
    0x140c, 0x46,
    0x140d, 0x4A,
    0x140e, 0x4F,
    0x140f, 0x53,
    0x1410, 0x58,
    0x1411, 0x5C,
    0x1412, 0x60,
    0x1413, 0x64,
    0x1414, 0x68,
    0x1415, 0x6C,
    0x1416, 0x6F,
    0x1417, 0x73,
    0x1418, 0x76,
    0x1419, 0x79,
    0x141a, 0x7D,
    0x141b, 0x80,
    0x141c, 0x83,
    0x141d, 0x86,
    0x141e, 0x88,
    0x141f, 0x8B,
    0x1420, 0x8E,
    0x1421, 0x93,
    0x1422, 0x97,
    0x1423, 0x9C,
    0x1424, 0xA0,
    0x1425, 0xA4,
    0x1426, 0xA7,
    0x1427, 0xAB,
    0x1428, 0xAE,
    0x1429, 0xB1,
    0x142a, 0xB4,
    0x142b, 0xB7,
    0x142c, 0xB9,
    0x142d, 0xBC,
    0x142e, 0xBE,
    0x142f, 0xC0,
    0x1430, 0xC2,
    0x1431, 0xC6,
    0x1432, 0xCA,
    0x1433, 0xCE,
    0x1434, 0xD2,
    0x1435, 0xD6,
    0x1436, 0xDA,
    0x1437, 0xDC,
    0x1438, 0xE1,
    0x1439, 0xE6,
    0x143a, 0xE9,
    0x143b, 0xEC,
    0x143c, 0xEE,
    0x143d, 0xF0,
    0x143e, 0xF5,
    0x143f, 0xFA,
    0x1440, 0xFF,




    0xfffe, 0x30, //Setting_A_Rb+0.2_Br+0.1_Bg-0.2_Gb-0.4_Rg-0.2.txt
    0x1200, 0x5,
    0x1201, 0xFF,
    0x1202, 0x0,
    0x1203, 0x6C,
    0x1204, 0x1,
    0x1205, 0xF8,
    0x1206, 0x1,
    0x1207, 0x39,
    0x1208, 0x0,
    0x1209, 0xA2,
    0x120A, 0x1,
    0x120B, 0x3,
    0x120C, 0x9,
    0x120D, 0xA8,
    0x120E, 0x1,
    0x120F, 0x4,
    0x1210, 0x1,
    0x1211, 0x70,
    0x1212, 0x1,
    0x1213, 0xF2,
    0x1214, 0x0,
    0x1215, 0xE0,
    0x1216, 0x0,
    0x1217, 0x15,
    0x1218, 0x7,
    0x1219, 0x40,
    0x121A, 0x0,
    0x121B, 0x87,
    0x121C, 0x0,
    0x121D, 0xBB,
    0x121E, 0x1,
    0x121F, 0x20,
    0x1220, 0x1,
    0x1221, 0x26,
    0x1222, 0x2,
    0x1223, 0x78,
    0x122e, 0x2,
    0x122F, 0x2,
    0x1230, 0xA,
    0x1228, 0x0,
    0x1229, 0x57,
    0x122A, 0x0,
    0x122B, 0x9E,
    0x122C, 0x1,
    0x122D, 0x1E,

    0xfffe, 0x30,
    0x1231, 0x02,

    0xfffe, 0x30,  //YEE
    0x2000, 0x30,  //bit[5]Yedge en; bit[3]UV_dns_en; bit[2]pre_dns_en; bit[1]ydns_man; bit[0]uvdns_man
    0x2001, 0x08, //edge_ratio
    0x2002, 0x07, //Y_dns_level_man
    0x2003, 0x02, //UV_dns_level_man
    0x2004, 0x01,   //gain list0
    0x2005, 0x01,
    0x2006, 0x02,
    0x2007, 0x03,
    0x2008, 0x04,
    0x2009, 0x05,
    0x200a, 0x06,
    0x200b, 0x1f,  //gain list7

    0x1907, 0xd2, //y_win+2
    0x1908, 0x01, //win_man_en

    0xfffe, 0x30,  //RAW_DNS
    0x0e00, 0x34, //34 //bit[5] Para_man_en  bit[4]G_en  34=manual 14=auto
    0x0e01, 0x00, //06 //noise list0
    0x0e02, 0x00, //0c
    0x0e03, 0x10, //10
    0x0e04, 0x30, //30
    0x0e05, 0x30,
    0x0e06, 0x68,
    0x0e07, 0x68,
    0x0e08, 0xa0,   //noise list7
    0x0e09, 0x3f,   //max_edge_thre
    0x0e0a, 0x04, //noise_man
    0x0e0b, 0x00, //edgethre_man

    0xfffe, 0x30,  //RGB_DNS
    0x130e, 0x02, //bit[3] Para_man_en  bit[2:0]Y_noise_man
    0x130f, 0x08, // bit[5:0]UV_noise_man

    0xfffe, 0x30,  //cip
    0x0f00, 0x00,  //bit[7:4] noise_y_slp  bit[3:0] Lsharp
    0x0f01, 0x00,
    0x0f02, 0x00, //noise list0
    0x0f03, 0x00,
    0x0f04, 0x00,
    0x0f05, 0x00,
    0x0f06, 0x00,
    0x0f07, 0x00,
    0x0f08, 0x00,
    0x0f09, 0x00, //noise list7
    0x0f0a, 0x00, //min_shp
    0x0f0b, 0x00, //max_shp
    0x0f0c, 0x00, //min_detail
    0x0f0d, 0x00, //max_detail
    0x0f0e, 0x00,  //min_shp_gain
    0x0f0f, 0x00,  //max_shp_gain

    0xfffe, 0x14,   //saturation adjust
    0x0160, 0x00, //bAutoSat_en
    0x0162, 0x40, //sat_u[0]
    0x0163, 0x38, //sat_u[1]
    0x0164, 0x30, //sat_u[2]
    0x0165, 0x2c, //sat_u[3]
    0x0166, 0x28, //sat_u[4]
    0x0167, 0x24, //sat_u[5]
    0x0168, 0x44, //sat_v[0]
    0x0169, 0x3c, //sat_v[1]
    0x016a, 0x34, //sat_v[2]
    0x016b, 0x30, //sat_v[3]
    0x016c, 0x2c, //sat_v[4]
    0x016d, 0x28, //sat_v[5]


    0xfffe, 0x14,
    0x0160, 0x00,
    0xfffe, 0x30,
    0x0000, 0x4f, //close LENC
    0x0001, 0x85, //a7
    0x0003, 0xe5,
    0x0013, 0x11,
    0x071c, 0x0a,
    0x1700, 0x09,
    0x1701, 0xa0,
    0x1702, 0xc0, //40
    0x1704, 0x22,
    0xfffe, 0x14,
    0x0027, 0x01,

    0xfffe, 0x30,
    0x0000, 0x4f,
    0x130e, 0x08,
    0x130f, 0x00,
    0xe00, 0x34,
    0xe0a, 0x00,
    0xe0b, 0xff,
    0x2000, 0x30,
    0x2001, 0x12,
    0x1701, 0xa0,
    0x1702, 0xa0,
    0x1704, 0x22,

    0xfffe, 0x30,
    0x0000, 0x4f,
    0x0001, 0x85,
    0x130e, 0x08,
    0x130f, 0x20,
    0xe00, 0x34,
    0xe0a, 0x0f,
    0xe0b, 0x38,
    0xf00, 0x48,
    0xf0a, 0x04,
    0x2000, 0x3f,
    0x2001, 0x14,
    0x1701, 0xa0,
    0x1702, 0xa0,
    0x1704, 0x23,


};


Sensor_reg_ini bypass_on[] = {
    0xfffd, 0x80,
    0xfffe, 0x50,
    0x004d, 0x01,
};

Sensor_reg_ini bypass_off[] = {
    0xfffd, 0x80,
    0xfffe, 0x50,
    0x004d, 0x00,
};




Sensor_reg_inix H42_regs[] = {

    // H42_1280x720_SET


    0x12, 0x40,
    0x0D, 0x40,
    0x1F, 0x04,
    0x0E, 0x1D,
    0x0F, 0x09,
    0x10, 0x1E,
    0x11, 0x80,
    0x19, 0x1c,
    0x20, 0x40,
    0x21, 0x06,
    0x22, 0xEE,
    0x23, 0x02,
    0x24, 0x00,
    0x25, 0xD0,
    0x26, 0x25,
    0x27, 0x48,
    0x28, 0x0e,
    0x29, 0x01,
    0x2A, 0x24,
    0x2B, 0x29,
    0x2C, 0x00,
    0x2D, 0x00,
    0x2E, 0xB9,
    0x2F, 0x00,
    0x30, 0x92,
    0x31, 0x0A,
    0x32, 0xAA,
    0x33, 0x14,
    0x34, 0x38,
    0x35, 0x54,
    0x42, 0x41,
    0x43, 0x50,
    0x1D, 0xFF,
    0x1E, 0x1F,
    0x6C, 0x90,
    0x73, 0xB3,
    0x70, 0x68,
    0x76, 0x40,
    0x77, 0x06,
    0x78, 0x18,
    0x60, 0xA4,
    0x61, 0xFF,
    0x62, 0x40,
    0x65, 0x00,
    0x67, 0x30,
    0x68, 0x04,
    0x69, 0x74,
    0x6F, 0x04,
    0x13, 0x67,
    0x14, 0x80,
    0x16, 0xC0,
    0x17, 0x40,
    0x18, 0xE1,
    0x38, 0x35,
    0x39, 0x98,
    0x4a, 0x03,
    0x48, 0x40,
    0x49, 0x0c,
    0x12, 0x00,
    0x12, 0x30, //PWDN Setting


};


void xc7011_reset(u8 isp_dev)
{
    u32 gpio;
    u32 pwdn_gpio;
    if (isp_dev == ISP_DEV_0) {
        gpio = reset_gpio[0];
        pwdn_gpio = pwdn_gpios[0];
    } else {
        gpio = reset_gpio[1];
        pwdn_gpio = pwdn_gpios[1];
    }


    gpio_direction_output(pwdn_gpio, 0);
    gpio_direction_output(gpio, 1);
    delay(50000);
    gpio_direction_output(gpio, 0);
    delay(50000);
    gpio_direction_output(gpio, 1);
}
s32 _xc7011_id_check(void)
{
    u8 id_hi;
    u8 id_lo;
    u32 i ;

    for (i = 0; i < 5; i++) { //

        rdXC7011Reg(0xfffb, &id_hi);
        rdXC7011Reg(0xfffc, &id_lo);

        if (id_hi == 0x70 && id_lo == 0x11) {
            log_d("Sensor PID: %x,%x\n", id_hi, id_lo);
            puts("----hello XC7011-----\n");
            return 0;
        }
    }
    puts("----not XC7011-----\n");
    return -1;
}


static u8 cur_sensor_type = 0xff;
s32 xc7011_id_check(u8 isp_dev, u32 _reset_gpio, u32 pwdn_gpio)
{
    if (!iic) {
        if (isp_dev == ISP_DEV_0) {
            iic = dev_open("iic0", 0);
        } else {
            iic = dev_open("iic2", 0);
        }
        if (!iic) {
            return -1;
        }
    } else {
        if (cur_sensor_type != isp_dev) {
            return -1;
        }
    }

    reset_gpio[isp_dev] = _reset_gpio;
    pwdn_gpios[isp_dev] = pwdn_gpio;
    xc7011_reset(isp_dev);

    if (0 != _xc7011_id_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }

    cur_sensor_type = isp_dev;

    return 0;

}
void XC7011_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    u32 i;
    u32 frame_exp_rows;
    u8 tmp;
    for (i = 0; i < sizeof(XC7011_default_regs) / sizeof(Sensor_reg_ini); i++) {
        wrXC7011Reg(XC7011_default_regs[i].addr, XC7011_default_regs[i].value);
    }

    for (i = 0; i < sizeof(bypass_on) / sizeof(Sensor_reg_ini); i++) {
        wrXC7011Reg(bypass_on[i].addr, bypass_on[i].value);
    }

    for (i = 0; i < sizeof(H42_regs) / sizeof(Sensor_reg_inix); i++) {
        wrH42Reg(H42_regs[i].addr, H42_regs[i].value);
    }

    for (i = 0; i < sizeof(bypass_off) / sizeof(Sensor_reg_ini); i++) {
        wrXC7011Reg(bypass_off[i].addr, bypass_off[i].value);
    }


    return;
}



s32 XC7011_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    xc7011_reset(isp_dev);
    puts("\n\n xc7011_init and h42init \n\n");

    XC7011_config_SENSOR(width, height, format, frame_freq);

    return 0;
}

s32 xc7011_set_output_size(u16 *width, u16 *height, u8 *freq)
{

    return 0;
}


s32 xc7011_power_ctl(u8 isp_dev, u8 is_work)
{
    return 0;
}
u8 xc7011_fps()
{
    return 0;//30fps
}
u8 xc7011_valid_signal(void *p)
{
    return 1;//valid
}

static u8 xc7011_mode_det(void *p)
{
    return 0;
}

REGISTER_CAMERA(XC7011) = {
    .logo 				= 	"XC7011",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_YUYV,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_PARALLEL,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_8B | SEN_MBUS_HSYNC_ACTIVE_HIGH | SEN_MBUS_PCLK_SAMPLE_FALLING,
    .fps         		= 	30,

    .sen_size 			= 	{1280, 720},
    .isp_size 			= 	{1280, 720},

    .ops                    = {
        .avin_fps           =   xc7011_fps,
        .avin_valid_signal  =   xc7011_valid_signal,
        .avin_mode_det      =   xc7011_mode_det,
        .sensor_check 		= 	xc7011_id_check,
        .init 		        = 	XC7011_init,
        .set_size_fps 		=	xc7011_set_output_size,
        .power_ctrl         =   xc7011_power_ctl,


        .sleep 		        =	NULL,
        .wakeup 		    =	NULL,
        .write_reg 		    =	NULL,
        .read_reg 		    =	NULL,
    }
};







