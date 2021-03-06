/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <mach/gpiomux.h>

#include "msm_sensor.h"
#include "msm_cci.h"
#include "msm_camera_io_util.h"
#include "msm_camera_i2c_mux.h"
#include <misc/app_info.h>

//#define CONFIG_MSMB_CAMERA_DEBUG
#undef CDBG
#ifdef CONFIG_MSMB_CAMERA_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif

#define BF3905_SENSOR_NAME "hw_bf3905_byd"
DEFINE_MSM_MUTEX(hw_bf3905_byd_mut);

static struct msm_sensor_ctrl_t bf3905_s_ctrl;
static int8_t hw_bf3905_module_id = 0;

static struct msm_sensor_power_setting bf3905_power_setting[] = {
      /* 1, IOVDD */
      {
          .seq_type = SENSOR_VREG,
          .seq_val = CAM_VIO,
          .config_val = 0,
          .delay = 0,
      },
      /* 2, PDN */
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_HIGH,
		.delay = 0,
	},
      /* 3, MCLK */
	{
		.seq_type = SENSOR_CLK,
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 24000000,
		.delay = 0,
	},
      /* 4, I2C */
	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 1,
	},
      /* 5, AVDD */
      {
          .seq_type = SENSOR_VREG,
          .seq_val = CAM_VANA,
          .config_val = 0,
          .delay = 2,
      },
      /* 6, RESET */
      {
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_HIGH,
		.delay = 1,
	},
      /* 7, PDN */
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_LOW,
		.delay = 5,
	},


};


static struct msm_sensor_power_setting bf3905_power_down_setting[] = {
	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_CLK,
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 24000000,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_HIGH,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_LOW,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VANA,
		.config_val = 0,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VIO,
		.config_val = 0,
		.delay = 20,
	},
};

static struct msm_camera_i2c_reg_conf bf3905_start_settings[] = {
//	{0xf5, 0x80},
};

static struct msm_camera_i2c_reg_conf bf3905_stop_settings[] = {
//	{0xf5, 0x00},
};
static struct msm_camera_i2c_reg_conf bf3905_recommend_settings[] = {
	//{0x12,0x80},
	{0x15,0x12},
	{0x3e,0x83},
	{0x09,0x01},
	{0x12,0x00},
	{0x3a,0x20},
	{0x1b,0x0e},
	{0x2a,0x00},//30FPS
	{0x2b,0x74},//30FPS
	{0x92,0x00},//9 de c6///60 0929////30FPS
	{0x93,0x00},//0 3 7//06 0929//30FPS
	{0x8a,0x86},//2A/2B
	{0x8b,0x6f},//2A/2B
	{0x13,0x00},
	{0x01,0x15},
	{0x02,0x23},
	{0x9d,0x20},
	{0x8c,0x02},
	{0x8d,0xee},
	{0x13,0x07},
	{0x5d,0xb3},
	{0xbf,0x08},
	{0xc3,0x08},
	{0xca,0x10},
	{0x62,0x00},
	{0x63,0x00},
	{0xb9,0x00},
	{0x64,0x00},
	{0x0e,0x10},
	{0x22,0x12},
	{0xbb,0x10},
	{0x08,0x02},
	{0x20,0x09},
	{0x21,0x4f},
	{0x2f,0x84},
	{0x71,0x0f},   //0x3f  wl 5.3
	{0x7e,0x84},
	{0x7e,0x84},
	{0x7f,0x3c},
	{0x60,0xe5},
	{0x61,0xf2},
	{0x6d,0xc0},
	{0x1e,0x40},//change 0x70 to 0x40, default set 0x40 for C8813Q display normally.
	{0xd9,0x25},
	{0xdf,0x26},
	{0x16,0xa1},   //0xaf  wl 5.3//a1 0xe3
	{0x6c,0xc2},
	{0x17,0x00},
	{0x18,0xa0},
	{0x19,0x00},
	{0x1a,0x78},
	{0x03,0xa0},
	{0x4a,0x0c},
	{0xda,0x00},
	{0xdb,0xa2},
	{0xdc,0x00},
	{0xdd,0x7a},
	{0xde,0x00},
	{0x33,0x10},
	{0x34,0x08},
	{0x36,0xc5},
	{0x6e,0x20},
	{0xbc,0x0d},
	{0x35,0x98}, //0x82   0x8f 92  //8f 0927  
	{0x65,0x78}, //84   // 7f  0929
	{0x66,0x76}, //80  //7a  0929   
	{0xbd,0xf4}, //0xe8
	{0xbe,0x49}, 
	{0x9b,0xe4},
	{0x9c,0x4c},
	{0x37,0xe4},
	{0x38,0x50}, 
	{0x70,0x0b},
	{0x71,0x0f},// 0e  yzx 4.17   0x3f  wl 5.3
	{0x72,0x4c},
	{0x73,0x37},
	{0x74,0x6d},
	{0x75,0xaa},  //8a yzx  4.17
	{0x76,0x28}, //  98  yzx  4.17
	{0x77,0x2a},//2a 0929
	{0x78,0xff},
	{0x79,0x24},
	{0x7a,0x23},// 12   yzx  4.18
	{0x7b,0x58},
	{0x7c,0x55},
	{0x7d,0x00},
	{0x13,0x07},
	{0x24,0x4a},    //  48   yzx  4.18//4c 0927
	{0x25,0x88},
	{0x80,0x92},//0x92 wl 5.7//9e 0927//9c 0929 0x9a
	{0x81,0x00},
	{0x82,0x2a},
	{0x83,0x54},
	{0x84,0x39},
	{0x85,0x5d},
	{0x86,0xe6},   ///e6
	{0x89,0xa5},  //63     //73   yzx  4/18  0x70  wl 5.3
	{0x8e,0x2c},
	{0x8f,0x8a},//82 0927 0x86
	{0x94,0x42},// 22 0927
	{0x95,0x84},
	{0x96,0xb3},
	{0x97,0x3c},
	{0x98,0xbb},  //  0x8a  wl 5.3 //bb wl 5.7// bf 0927
	{0x99,0x10},
	{0x9a,0x20},  // 50  yzx 4.18//20 0927
	{0x9f,0x64},
	{0x39,0xa3}, //98  yzx 4.17//90 0927 0xaa
	{0x3f,0x23}, //98  yzx 4.17//90 0927 0x2a
	{0x90,0x20},//20 0927
	{0x91,0x70},//70 0927
	{0x40,0x36}, 
	{0x41,0x33}, 
	{0x42,0x2a}, 
	{0x43,0x22}, 
	{0x44,0x1b}, 
	{0x45,0x16}, 
	{0x46,0x13}, 
	{0x47,0x10}, 
	{0x48,0x0e}, 
	{0x49,0x0c}, 
	{0x4b,0x0b}, 
	{0x4c,0x0a}, 
	{0x4e,0x08}, //0x09
	{0x4f,0x07}, //08
	{0x50,0x06}, //07
	{0x5a,0x56},
	{0x51,0x12},
	{0x52,0x0d},
	{0x53,0x92},
	{0x54,0x7d},
	{0x57,0x97},
	{0x58,0x43},
	{0x5a,0xd6},
	/* avoiding too red skin */                                
	{0x51,0x0c},                                           
	{0x52,0x04},                                           
	{0x53,0x6a},                                           
	{0x54,0x58},                                           
	{0x57,0x7b},                                           
	{0x58,0x14},                                           
	{0x5b,0x02},                                           
	{0x5c,0x30},                                           
	{0x6a,0x81},                                           
	{0x23,0x66},//55 0929                                              
	{0xa0,0x00},                                           
	{0xa1,0x31},                                           
	{0xa2,0x09},                                           
	{0xa3,0x2c},//27 0927                                  
	{0xa4,0x09},//                                         
	{0xa5,0x28},//27 0927                                  
	{0xa6,0x04},                                           
	{0xa7,0x1c},                               
	{0xa8,0x1c},                                           
	{0xa9,0x16}, //16  15                                  
	{0xaa,0x16},                                           
	{0xab,0x1c},                                           
	{0xac,0x3c},                                           
	{0xad,0xe8},                                           
	{0xae,0x57},//7c   0x7b  0x7c 0x7b                                                                 
	{0xc5,0x55},//0xaa                                      
	{0xc6,0xcc},                                            
	{0xc7,0x30}, //  yzx 10 4.17                           
	{0xc8,0x0d},                                        
	{0xc9,0x17}, //0x19                                   
	{0xd0,0x93}, //0xb7//69 0930                            
	{0xd1,0x00},//00//00 0930                               
	{0xd2,0x58},                                            
	{0xd3,0x05},  ///0c                                     
	{0xd4,0x16},  /// 30 avoiding blue sky redish                               
	{0xee,0x30},                                            
	{0xb0,0xe0},//e0 0929                                   
	{0xb3,0x48},                                            
	{0xb4,0xa3},                                            
	{0xb1,0xef},                                            
	{0xb2,0xa0},// a0                                       
	{0xb4,0x63},                                            
	{0xb1,0xc0}, //0xba  c0 c8   ///ca    yzx 4.17//ce 0927 
	{0xb2,0xaa},  //0xa8  //aa   yzx 4.17//be 0927          
	{0x55,0x00},
	{0x56,0x40},                                           
	{0x56,0x40},  
};

static struct v4l2_subdev_info bf3905_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_YUYV8_2X8,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order  = 0,
	},
};

/*add bf3905 functional parameters*/
static struct msm_camera_i2c_reg_conf bf3905_reg_saturation[][6] = {
	{{0xb4,0xe3},{0xb1,0xef},{0xb2,0xaa},{0xb4,0x63},{0xb1,0x00},{0xb2,0x00},}, /* SATURATION LEVEL0*/
	{{0xb4,0xe3},{0xb1,0xef},{0xb2,0xaa},{0xb4,0x63},{0xb1,0x30},{0xb2,0x20},}, /* SATURATION LEVEL1*/
	{{0xb4,0xe3},{0xb1,0xef},{0xb2,0xaa},{0xb4,0x63},{0xb1,0x50},{0xb2,0x40},}, /* SATURATION LEVEL2*/
	{{0xb4,0xe3},{0xb1,0xef},{0xb2,0xaa},{0xb4,0x63},{0xb1,0x70},{0xb2,0x60},}, /* SATURATION LEVEL3*/
	{{0xb4,0xe3},{0xb1,0xef},{0xb2,0xaa},{0xb4,0x63},{0xb1,0x90},{0xb2,0x80},}, /* SATURATION LEVEL4*/
	{{0xb4,0xe3},{0xb1,0xef},{0xb2,0xaa},{0xb4,0x63},{0xb1,0xc0},{0xb2,0xaa},}, /* SATURATION LEVEL5*/
	{{0xb4,0xe3},{0xb1,0xef},{0xb2,0xaa},{0xb4,0x63},{0xb1,0xc8},{0xb2,0xba},}, /* SATURATION LEVEL6*/
	{{0xb4,0xe3},{0xb1,0xef},{0xb2,0xaa},{0xb4,0x63},{0xb1,0xd8},{0xb2,0xd0},}, /* SATURATION LEVEL7*/
	{{0xb4,0xe3},{0xb1,0xef},{0xb2,0xaa},{0xb4,0x63},{0xb1,0xe8},{0xb2,0xe8},}, /* SATURATION LEVEL8*/
	{{0xb4,0xe3},{0xb1,0xef},{0xb2,0xaa},{0xb4,0x63},{0xb1,0xf0},{0xb2,0xf0},}, /* SATURATION LEVEL8*/
	{{0xb4,0xe3},{0xb1,0xef},{0xb2,0xaa},{0xb4,0x63},{0xb1,0xff},{0xb2,0xff},}, /* SATURATION LEVEL10*/
};

static struct msm_camera_i2c_reg_conf bf3905_reg_contrast[][1] = {
	{{0x56,0x18},}, /* CONTRAST L0*/
	{{0x56,0x20},}, /* CONTRAST L1*/
	{{0x56,0x28},}, /* CONTRAST L2*/
	{{0x56,0x30},}, /* CONTRAST L3*/
	{{0x56,0x38},}, /* CONTRAST L4*/
	{{0x56,0x40},}, /* CONTRAST L5*/
	{{0x56,0x48},}, /* CONTRAST L6*/
	{{0x56,0x50},}, /* CONTRAST L7*/
	{{0x56,0x60},}, /* CONTRAST L8*/
	{{0x56,0x70},}, /* CONTRAST L9*/
	{{0x56,0x80},}, /* CONTRAST L10*/
};

static struct msm_camera_i2c_reg_conf bf3905_reg_sharpness[][2] = {
	{{0x7a,0x00},{0x73,0x63},}, /* SHARPNESS LEVEL 0*/
	{{0x7a,0x01},{0x73,0x45},}, /* SHARPNESS LEVEL 1*/
	{{0x7a,0x12},{0x73,0x27},}, /* SHARPNESS LEVEL 2*/
	{{0x7a,0x23},{0x73,0x1a},}, /* SHARPNESS LEVEL 3*/
	{{0x7a,0x55},{0x73,0x1a},}, /* SHARPNESS LEVEL 4*/
	{{0x7a,0x66},{0x73,0x1c},}, /* SHARPNESS LEVEL 5*/
	{{0x7a,0x77},{0x73,0x0f},}, /* SHARPNESS LEVEL 6*/
};

static struct msm_camera_i2c_reg_conf bf3905_reg_iso[][1] = {
	{{0x9f,0x61},},/*ISO_AUTO*/
	{{0x9f,0x61},},/*ISO_DEBLUR*/
	{{0x9f,0x62},},/*ISO_100*/
	{{0x9f,0x63},},/*ISO_200*/
	{{0x9f,0x64},},/*ISO_400*/
	{{0x9f,0x65},},/*ISO_800*/
	{{0x9f,0x66},},/*ISO_1600*/
};

static struct msm_camera_i2c_reg_conf bf3905_reg_exposure_compensation[][1] = {
	{{0x24,0x10},}, /*EXPOSURE COMPENSATION N2*/
	{{0x24,0x28},}, /*EXPOSURE COMPENSATION N1*/
	{{0x24,0x4a},}, /*EXPOSURE COMPENSATION D*/
	{{0x24,0x60},}, /*EXPOSURE COMPENSATION P1*/
	{{0x24,0x7f},}, /*EXPOSURE COMPENSATION P2*/
};

static struct msm_camera_i2c_reg_conf bf3905_reg_antibanding[][2] = {
	{{0x80,0x92},{0x8a,0x96},}, /*ANTIBANDING AUTO*/
	{{0x80,0x92},{0x8a,0x96},}, /*ANTIBANDING 50HZ*/
	{{0x80,0x90},{0x8b,0x7d},}, /*ANTIBANDING 60HZ*/
	{{0x80,0x92},{0x8a,0x96},}, /*ANTIBANDING AUTO*/
};

static struct msm_camera_i2c_reg_conf bf3905_reg_effect_normal[] = {
	/* normal: */
	{0xb4,0xe3},{0xb1,0xef},{0xb2,0xaa},{0xb4,0x63},{0xb1,0xc0},{0xb2,0xaa},{0x56,0x40},
	{0x7a,0x23},{0x73,0x27},{0x69,0x00},{0x67,0x80},{0x68,0x80},{0xb4,0x63},
};

static struct msm_camera_i2c_reg_conf bf3905_reg_effect_mono[] = {
	/* mono: */
	{0xb4,0xe3},{0xb1,0xef},{0xb2,0xef},{0xb4,0x63},{0xb1,0xba},{0xb2,0xaa},{0x56,0x40},
	{0x7a,0x12},{0x73,0x27},{0x69,0x20},{0x67,0x80},{0x68,0x80},{0xb4,0x03},
};

static struct msm_camera_i2c_reg_conf bf3905_reg_effect_negative[] = {
	/* Negative: */
	{0xb4,0xe3},{0xb1,0xef},{0xb2,0xef},{0xb4,0x63},{0xb1,0xba},{0xb2,0xaa},{0x56,0x40},
	{0x7a,0x12},{0x73,0x1a},{0x69,0x01},{0x67,0x80},{0x68,0x80},{0xb4,0x03},
};

static struct msm_camera_i2c_reg_conf bf3905_reg_effect_sepia[] = {
	/* Sepia(antique): */
	{0xb4,0xe3},{0xb1,0xef},{0xb2,0xef},{0xb4,0x63},{0xb1,0xba},{0xb2,0xaa},{0x56,0x40},
	{0x7a,0x12},{0x73,0x27},{0x69,0x20},{0x67,0x60},{0x68,0xa0},{0xb4,0x03},
};

static struct msm_camera_i2c_reg_conf bf3905_reg_effect_solarize[] = {
	/* solarize: */
	{0xb4,0xe3},{0xb1,0xef},{0xb2,0xef},{0xb4,0x63},{0xb1,0xba},{0xb2,0xaa},{0x56,0x40},
	{0x7a,0x12},{0x73,0x27},{0x69,0x40},{0x67,0x80},{0x68,0x80},{0xb4,0x03},
};

static struct msm_camera_i2c_reg_conf bf3905_reg_effect_whiteboard[] = {
	/* whiteboard: */
	{0xb4,0xe3},{0xb1,0xef},{0xb2,0xef},{0xb4,0x63},{0xb1,0xba},{0xb2,0xaa},{0x56,0x40},
	{0x7a,0x12},{0x73,0x27},{0x69,0x00},{0x67,0x80},{0x68,0x80},{0xb4,0x03},
};

static struct msm_camera_i2c_reg_conf bf3905_reg_effect_blackboard[] = {
	/* blackboard: */
	{0xb4,0xe3},{0xb1,0xef},{0xb2,0xef},{0xb4,0x63},{0xb1,0xba},{0xb2,0xaa},{0x56,0x40},
	{0x7a,0x12},{0x73,0x27},{0x69,0x00},{0x67,0x80},{0x68,0x80},{0xb4,0x03},
};

static struct msm_camera_i2c_reg_conf bf3905_reg_effect_aqua[] = {
	/* aqua: */
	{0xb4,0xe3},{0xb1,0xef},{0xb2,0xef},{0xb4,0x63},{0xb1,0xba},{0xb2,0xaa},{0x56,0x40},
	{0x7a,0x12},{0x73,0x27},{0x69,0x00},{0x67,0x80},{0x68,0x80},{0xb4,0x03},
};

static struct msm_camera_i2c_reg_conf bf3905_reg_effect_emboss[] = {
	/* emboss: */
	{0xb4,0xe3},{0xb1,0xef},{0xb2,0xef},{0xb4,0x63},{0xb1,0xba},{0xb2,0xaa},{0x56,0x40},
	{0x7a,0x12},{0x73,0x27},{0x69,0x00},{0x67,0x80},{0x68,0x80},{0xb4,0x03},
};

static struct msm_camera_i2c_reg_conf bf3905_reg_effect_sketch[] = {
	/* sketch: */
	{0xb4,0xe3},{0xb1,0xef},{0xb2,0xef},{0xb4,0x63},{0xb1,0xba},{0xb2,0xaa},{0x56,0x40},
	{0x7a,0x12},{0x73,0x27},{0x69,0x00},{0x67,0x80},{0x68,0x80},{0xb4,0x03},
};

static struct msm_camera_i2c_reg_conf bf3905_reg_effect_neon[] = {
	/* neon: */
	{0xb4,0xe3},{0xb1,0xef},{0xb2,0xef},{0xb4,0x63},{0xb1,0xba},{0xb2,0xaa},{0x56,0x40},
	{0x7a,0x12},{0x73,0x27},{0x69,0x20},{0x67,0x58},{0x68,0xc0},{0xb4,0x03},
};

static struct msm_camera_i2c_reg_conf bf3905_reg_wb_auto[] = {
	/* Auto: */
	{0x13,0x07},{0x01,0x15},{0x02,0x23},
};

static struct msm_camera_i2c_reg_conf bf3905_reg_wb_daylight[] = {
	/* day light: */
	{0x13,0x05},{0x6a,0x81},{0x23,0x55},{0x01,0x12}/*14*/,{0x02,0x1a},
};

static struct msm_camera_i2c_reg_conf bf3905_reg_wb_cloudy[] = {
	/* Cloudy: */
	{0x13,0x05},{0x6a,0x81},{0x23,0x55},{0x01,0x0e}/*10*/,{0x02,0x22},
};

static struct msm_camera_i2c_reg_conf bf3905_reg_wb_fluorescent[] = {
	/* fluorescent: */
	{0x13,0x05},{0x6a,0x81},{0x23,0x55},{0x01,0x1c},{0x02,0x16}/*1a*/, 
};

static struct msm_camera_i2c_reg_conf bf3905_reg_wb_incandescent[] = {
	/* incandescent: */
	{0x13,0x05},{0x6a,0x81},{0x23,0x55},{0x01,0x22}/*25*/,{0x02,0x0d},
};

static const struct i2c_device_id bf3905_i2c_id[] = {
	{BF3905_SENSOR_NAME, (kernel_ulong_t)&bf3905_s_ctrl},
	{ }
};

/*the interface to write register to enable the function*/
static void bf3905_i2c_write_table(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_camera_i2c_reg_conf *table,
		int num)
{
	int i = 0;
	int rc = 0;
	for (i = 0; i < num; ++i) {
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write(
			s_ctrl->sensor_i2c_client, table->reg_addr,
			table->reg_data,
			MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) {
			msleep(100);
			rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write(
				s_ctrl->sensor_i2c_client, table->reg_addr,
				table->reg_data,
				MSM_CAMERA_I2C_BYTE_DATA);
		}
		table++;
	}
}

static int32_t msm_bf3905_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	CDBG("%s, E.", __func__);

	return msm_sensor_i2c_probe(client, id, &bf3905_s_ctrl);
}

static struct i2c_driver bf3905_i2c_driver = {
	.id_table = bf3905_i2c_id,
	.probe  = msm_bf3905_i2c_probe,
	.driver = {
		.name = BF3905_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client bf3905_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
};

static const struct of_device_id hw_bf3905_byd_dt_match[] = {
	{.compatible = "qcom,hw_bf3905_byd", .data = &bf3905_s_ctrl},
	{}
};

MODULE_DEVICE_TABLE(of, hw_bf3905_byd_dt_match);

static struct platform_driver bf3905_platform_driver = {
	.driver = {
		.name = "qcom,hw_bf3905_byd",
		.owner = THIS_MODULE,
		.of_match_table = hw_bf3905_byd_dt_match,
	},
};

static int32_t bf3905_platform_probe(struct platform_device *pdev)
{
	int32_t rc;
	const struct of_device_id *match;
	CDBG("%s, E.", __func__);
	match = of_match_device(hw_bf3905_byd_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);
	return rc;
}

static int __init bf3905_init_module(void)
{
	int32_t rc;
	pr_info("%s:%d\n", __func__, __LINE__);
	rc = platform_driver_probe(&bf3905_platform_driver,
		bf3905_platform_probe);
	if (!rc)
		return rc;
	pr_err("%s:%d rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver(&bf3905_i2c_driver);
}

static void __exit bf3905_exit_module(void)
{
	pr_info("%s:%d\n", __func__, __LINE__);
	if (bf3905_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&bf3905_s_ctrl);
		platform_driver_unregister(&bf3905_platform_driver);
	} else
		i2c_del_driver(&bf3905_i2c_driver);
	return;
}

/*The interface to enable functions*/
static void bf3905_set_stauration(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	CDBG("%s %d", __func__, value);
	bf3905_i2c_write_table(s_ctrl, &bf3905_reg_saturation[value][0],
		ARRAY_SIZE(bf3905_reg_saturation[value]));
}

static void bf3905_set_contrast(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	CDBG("%s %d", __func__, value);
	bf3905_i2c_write_table(s_ctrl, &bf3905_reg_contrast[value][0],
		ARRAY_SIZE(bf3905_reg_contrast[value]));
}

static void bf3905_set_sharpness(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	int val = value / 6;
	CDBG("%s %d", __func__, value);
	bf3905_i2c_write_table(s_ctrl, &bf3905_reg_sharpness[val][0],
		ARRAY_SIZE(bf3905_reg_sharpness[val]));
}


static void bf3905_set_iso(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	CDBG("%s %d", __func__, value);
	bf3905_i2c_write_table(s_ctrl, &bf3905_reg_iso[value][0],
		ARRAY_SIZE(bf3905_reg_iso[value]));
}

static void bf3905_set_exposure_compensation(struct msm_sensor_ctrl_t *s_ctrl,
	int value)
{
	int val = (value + 12) / 6;
	CDBG("%s %d", __func__, val);
	bf3905_i2c_write_table(s_ctrl, &bf3905_reg_exposure_compensation[val][0],
		ARRAY_SIZE(bf3905_reg_exposure_compensation[val]));
}

static void bf3905_set_effect(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	CDBG("%s %d", __func__, value);
	switch (value) {
	case MSM_CAMERA_EFFECT_MODE_OFF: {
		bf3905_i2c_write_table(s_ctrl, &bf3905_reg_effect_normal[0],
			ARRAY_SIZE(bf3905_reg_effect_normal));
		break;
	}
	case MSM_CAMERA_EFFECT_MODE_MONO: {
		bf3905_i2c_write_table(s_ctrl, &bf3905_reg_effect_mono[0],
			ARRAY_SIZE(bf3905_reg_effect_mono));
		break;
	}
	case MSM_CAMERA_EFFECT_MODE_NEGATIVE: {
		bf3905_i2c_write_table(s_ctrl, &bf3905_reg_effect_negative[0],
			ARRAY_SIZE(bf3905_reg_effect_negative));
		break;
	}
	case MSM_CAMERA_EFFECT_MODE_SEPIA: {
		bf3905_i2c_write_table(s_ctrl, &bf3905_reg_effect_sepia[0],
			ARRAY_SIZE(bf3905_reg_effect_sepia));
		break;
	}
	case MSM_CAMERA_EFFECT_MODE_SOLARIZE: {
		bf3905_i2c_write_table(s_ctrl, &bf3905_reg_effect_solarize[0],
			ARRAY_SIZE(bf3905_reg_effect_solarize));
		break;
	}
	case MSM_CAMERA_EFFECT_MODE_WHITEBOARD: {
		bf3905_i2c_write_table(s_ctrl, &bf3905_reg_effect_whiteboard[0],
			ARRAY_SIZE(bf3905_reg_effect_whiteboard));
		break;
	}
	case MSM_CAMERA_EFFECT_MODE_BLACKBOARD: {
		bf3905_i2c_write_table(s_ctrl, &bf3905_reg_effect_blackboard[0],
			ARRAY_SIZE(bf3905_reg_effect_blackboard));
		break;
	}
	case MSM_CAMERA_EFFECT_MODE_AQUA: {
		bf3905_i2c_write_table(s_ctrl, &bf3905_reg_effect_aqua[0],
			ARRAY_SIZE(bf3905_reg_effect_aqua));
		break;
	}
	case MSM_CAMERA_EFFECT_MODE_EMBOSS: {
		bf3905_i2c_write_table(s_ctrl, &bf3905_reg_effect_emboss[0],
			ARRAY_SIZE(bf3905_reg_effect_emboss));
    	break;
	}
	case MSM_CAMERA_EFFECT_MODE_SKETCH: {
		bf3905_i2c_write_table(s_ctrl, &bf3905_reg_effect_sketch[0],
			ARRAY_SIZE(bf3905_reg_effect_sketch));
		break;
	}
	case MSM_CAMERA_EFFECT_MODE_NEON: {
		bf3905_i2c_write_table(s_ctrl, &bf3905_reg_effect_neon[0],
			ARRAY_SIZE(bf3905_reg_effect_neon));
		break;
	}
	default:
		bf3905_i2c_write_table(s_ctrl, &bf3905_reg_effect_normal[0],
			ARRAY_SIZE(bf3905_reg_effect_normal));
	}
}

static void bf3905_set_antibanding(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	CDBG("%s %d", __func__, value);
	bf3905_i2c_write_table(s_ctrl, &bf3905_reg_antibanding[value][0],
		ARRAY_SIZE(bf3905_reg_antibanding[value]));
}

static void bf3905_set_white_balance_mode(struct msm_sensor_ctrl_t *s_ctrl,
	int value)
{
	CDBG("%s %d", __func__, value);
	switch (value) {
	case MSM_CAMERA_WB_MODE_AUTO: {
		bf3905_i2c_write_table(s_ctrl, &bf3905_reg_wb_auto[0],
			ARRAY_SIZE(bf3905_reg_wb_auto));
		break;
	}
	case MSM_CAMERA_WB_MODE_INCANDESCENT: {
		bf3905_i2c_write_table(s_ctrl, &bf3905_reg_wb_incandescent[0],
			ARRAY_SIZE(bf3905_reg_wb_incandescent));
		break;
	}
	case MSM_CAMERA_WB_MODE_DAYLIGHT: {
		bf3905_i2c_write_table(s_ctrl, &bf3905_reg_wb_daylight[0],
			ARRAY_SIZE(bf3905_reg_wb_daylight));
				break;
	}
	case MSM_CAMERA_WB_MODE_FLUORESCENT: {
		bf3905_i2c_write_table(s_ctrl, &bf3905_reg_wb_fluorescent[0],
			ARRAY_SIZE(bf3905_reg_wb_fluorescent));
					break;
	}
	case MSM_CAMERA_WB_MODE_CLOUDY_DAYLIGHT: {
		bf3905_i2c_write_table(s_ctrl, &bf3905_reg_wb_cloudy[0],
			ARRAY_SIZE(bf3905_reg_wb_cloudy));
					break;
	}
	default:
		bf3905_i2c_write_table(s_ctrl, &bf3905_reg_wb_auto[0],
		ARRAY_SIZE(bf3905_reg_wb_auto));
	}
}

int32_t bf3905_sensor_config(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp)
{
	struct sensorb_cfg_data *cdata = (struct sensorb_cfg_data *)argp;
	long rc = 0;
	int32_t i = 0;
	mutex_lock(s_ctrl->msm_sensor_mutex);
	CDBG("%s:%d %s cfgtype = %d\n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
	switch (cdata->cfgtype) {
	case CFG_GET_SENSOR_INFO:
		memcpy(cdata->cfg.sensor_info.sensor_name,
			s_ctrl->sensordata->sensor_name,
			sizeof(cdata->cfg.sensor_info.sensor_name));
		cdata->cfg.sensor_info.session_id =
			s_ctrl->sensordata->sensor_info->session_id;
		for (i = 0; i < SUB_MODULE_MAX; i++)
			cdata->cfg.sensor_info.subdev_id[i] =
				s_ctrl->sensordata->sensor_info->subdev_id[i];
		CDBG("%s:%d sensor name %s\n", __func__, __LINE__,
			cdata->cfg.sensor_info.sensor_name);
		CDBG("%s:%d session id %d\n", __func__, __LINE__,
			cdata->cfg.sensor_info.session_id);
		for (i = 0; i < SUB_MODULE_MAX; i++)
			CDBG("%s:%d subdev_id[%d] %d\n", __func__, __LINE__, i,
				cdata->cfg.sensor_info.subdev_id[i]);

		break;
	case CFG_SET_INIT_SETTING:
		/* Write Recommend settings */
		pr_err("%s, sensor write init setting!!", __func__);

		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
					i2c_write_conf_tbl(s_ctrl->sensor_i2c_client,
					bf3905_recommend_settings,
					ARRAY_SIZE(bf3905_recommend_settings),
					MSM_CAMERA_I2C_BYTE_DATA);
		break;
	case CFG_SET_RESOLUTION:
		break;
	case CFG_SET_STOP_STREAM:
		pr_err("%s, sensor stop stream!!", __func__);
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(s_ctrl->sensor_i2c_client,
			bf3905_stop_settings,
			ARRAY_SIZE(bf3905_stop_settings),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	case CFG_SET_START_STREAM:
		pr_err("%s, sensor start stream!!", __func__);
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(s_ctrl->sensor_i2c_client,
			bf3905_start_settings,
			ARRAY_SIZE(bf3905_start_settings),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	case CFG_GET_SENSOR_INIT_PARAMS:
		cdata->cfg.sensor_init_params =
			*s_ctrl->sensordata->sensor_init_params;
		CDBG("%s:%d init params mode %d pos %d mount %d\n", __func__,
			__LINE__,
			cdata->cfg.sensor_init_params.modes_supported,
			cdata->cfg.sensor_init_params.position,
			cdata->cfg.sensor_init_params.sensor_mount_angle);
		break;
	case CFG_SET_SLAVE_INFO: {
		struct msm_camera_sensor_slave_info sensor_slave_info;
		struct msm_sensor_power_setting_array *power_setting_array;
		int slave_index = 0;
		if (copy_from_user(&sensor_slave_info,
		    (void *)cdata->cfg.setting,
		    sizeof(struct msm_camera_sensor_slave_info))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		/* Update sensor slave address */
		if (sensor_slave_info.slave_addr) {
			s_ctrl->sensor_i2c_client->cci_client->sid =
				sensor_slave_info.slave_addr >> 1;
		}

		/* Update sensor address type */
		s_ctrl->sensor_i2c_client->addr_type =
			sensor_slave_info.addr_type;

		/* Update power up / down sequence */
		s_ctrl->power_setting_array =
			sensor_slave_info.power_setting_array;
		power_setting_array = &s_ctrl->power_setting_array;
		power_setting_array->power_setting = kzalloc(
			power_setting_array->size *
			sizeof(struct msm_sensor_power_setting), GFP_KERNEL);
		if (!power_setting_array->power_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(power_setting_array->power_setting,
		    (void *)sensor_slave_info.power_setting_array.power_setting,
		    power_setting_array->size *
		    sizeof(struct msm_sensor_power_setting))) {
			kfree(power_setting_array->power_setting);
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		s_ctrl->free_power_setting = true;
		CDBG("%s sensor id %x\n", __func__,
			sensor_slave_info.slave_addr);
		CDBG("%s sensor addr type %d\n", __func__,
			sensor_slave_info.addr_type);
		CDBG("%s sensor reg %x\n", __func__,
			sensor_slave_info.sensor_id_info.sensor_id_reg_addr);
		CDBG("%s sensor id %x\n", __func__,
			sensor_slave_info.sensor_id_info.sensor_id);
		for (slave_index = 0; slave_index <
			power_setting_array->size; slave_index++) {
			CDBG("%s i %d power setting %d %d %ld %d\n", __func__,
				slave_index,
				power_setting_array->power_setting[slave_index].
				seq_type,
				power_setting_array->power_setting[slave_index].
				seq_val,
				power_setting_array->power_setting[slave_index].
				config_val,
				power_setting_array->power_setting[slave_index].
				delay);
		}
		kfree(power_setting_array->power_setting);
		break;
	}
	case CFG_WRITE_I2C_ARRAY: {
		struct msm_camera_i2c_reg_setting conf_array;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;

		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write_table(
			s_ctrl->sensor_i2c_client, &conf_array);
		kfree(reg_setting);
		break;
	}
	case CFG_WRITE_I2C_SEQ_ARRAY: {
		struct msm_camera_i2c_seq_reg_setting conf_array;
		struct msm_camera_i2c_seq_reg_array *reg_setting = NULL;

		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_seq_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_seq_reg_array)),
			GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_seq_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_seq_table(s_ctrl->sensor_i2c_client,
			&conf_array);
		kfree(reg_setting);
		break;
	}

	case CFG_POWER_UP:
		if (s_ctrl->func_tbl->sensor_power_up)
			rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl);
		else
			rc = -EFAULT;
		break;

	case CFG_POWER_DOWN:
		if (s_ctrl->func_tbl->sensor_power_down)
			rc = s_ctrl->func_tbl->sensor_power_down(
				s_ctrl);
		else
			rc = -EFAULT;
		break;

	case CFG_SET_STOP_STREAM_SETTING: {
		struct msm_camera_i2c_reg_setting *stop_setting =
			&s_ctrl->stop_setting;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;
		if (copy_from_user(stop_setting, (void *)cdata->cfg.setting,
		    sizeof(struct msm_camera_i2c_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = stop_setting->reg_setting;
		stop_setting->reg_setting = kzalloc(stop_setting->size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!stop_setting->reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(stop_setting->reg_setting,
		    (void *)reg_setting, stop_setting->size *
		    sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(stop_setting->reg_setting);
			stop_setting->reg_setting = NULL;
			stop_setting->size = 0;
			rc = -EFAULT;
			break;
		}
		break;
	}
#ifdef CONFIG_HUAWEI_KERNEL_CAMERA
    case CFG_GET_SENSOR_PROJECT_INFO:
        memcpy(cdata->cfg.sensor_info.sensor_project_name,
               s_ctrl->sensordata->sensor_info->sensor_project_name,
               sizeof(cdata->cfg.sensor_info.sensor_project_name));

        pr_info("%s, %d: sensor project name %s\n", __func__, __LINE__,
               cdata->cfg.sensor_info.sensor_project_name);
    break;
#endif

	case CFG_SET_SATURATION: {
		int32_t sat_lev;
		if (copy_from_user(&sat_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		CDBG("%s: Saturation Value is %d", __func__, sat_lev);
		bf3905_set_stauration(s_ctrl, sat_lev);
		break;
	}
	case CFG_SET_CONTRAST: {
		int32_t con_lev;
		if (copy_from_user(&con_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		CDBG("%s: Contrast Value is %d", __func__, con_lev);
		bf3905_set_contrast(s_ctrl, con_lev);
		break;
	}
	case CFG_SET_SHARPNESS: {
		int32_t shp_lev;
		if (copy_from_user(&shp_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		CDBG("%s: Sharpness Value is %d", __func__, shp_lev);
		bf3905_set_sharpness(s_ctrl, shp_lev);
		break;
	}
	case CFG_SET_ISO: {
		int32_t iso_lev;
		if (copy_from_user(&iso_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		CDBG("%s: ISO Value is %d", __func__, iso_lev);
		bf3905_set_iso(s_ctrl, iso_lev);
		break;
	}
	case CFG_SET_EXPOSURE_COMPENSATION: {
		int32_t ec_lev;
		if (copy_from_user(&ec_lev, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		CDBG("%s: Exposure compensation Value is %d",
			__func__, ec_lev);
		bf3905_set_exposure_compensation(s_ctrl, ec_lev);
		break;
	}
	case CFG_SET_EFFECT: {
		int32_t effect_mode;
		if (copy_from_user(&effect_mode, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		CDBG("%s: Effect mode is %d", __func__, effect_mode);
		bf3905_set_effect(s_ctrl, effect_mode);
		break;
	}
	case CFG_SET_ANTIBANDING: {
		int32_t antibanding_mode;
		if (copy_from_user(&antibanding_mode,
			(void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		CDBG("%s: anti-banding mode is %d", __func__,
			antibanding_mode);
		bf3905_set_antibanding(s_ctrl, antibanding_mode);
		break;
	}
	/*do not support bestshot mode for the bf3905*/
	/*
	case CFG_SET_BESTSHOT_MODE: {
		int32_t bs_mode;
		if (copy_from_user(&bs_mode, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		CDBG("%s: best shot mode is %d", __func__, bs_mode);
		bf3905_set_scene_mode(s_ctrl, bs_mode);
		break;
	}
	*/
	case CFG_SET_WHITE_BALANCE: {
		int32_t wb_mode;
		if (copy_from_user(&wb_mode, (void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		printk("%s: white balance is %d", __func__, wb_mode);
		bf3905_set_white_balance_mode(s_ctrl, wb_mode);
		break;
	}

	default:
		rc = -EFAULT;
		break;
	}

	mutex_unlock(s_ctrl->msm_sensor_mutex);

	return rc;
}

/*make bf3905 use its own if functions because of the msm-if changed to static*/
static int32_t bf3905_sensor_enable_i2c_mux(struct msm_camera_i2c_conf *i2c_conf)
{
	struct v4l2_subdev *i2c_mux_sd =
		dev_get_drvdata(&i2c_conf->mux_dev->dev);
	v4l2_subdev_call(i2c_mux_sd, core, ioctl,
		VIDIOC_MSM_I2C_MUX_INIT, NULL);
	v4l2_subdev_call(i2c_mux_sd, core, ioctl,
		VIDIOC_MSM_I2C_MUX_CFG, (void *)&i2c_conf->i2c_mux_mode);
	return 0;
}

static int32_t bf3905_sensor_disable_i2c_mux(struct msm_camera_i2c_conf *i2c_conf)
{
	struct v4l2_subdev *i2c_mux_sd =
		dev_get_drvdata(&i2c_conf->mux_dev->dev);
	v4l2_subdev_call(i2c_mux_sd, core, ioctl,
				VIDIOC_MSM_I2C_MUX_RELEASE, NULL);
	return 0;
}

/*make 3905 use its own power up&down func*/
int32_t bf3905_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	int32_t index = 0;
	struct msm_sensor_power_setting_array *power_setting_array = NULL;
	struct msm_sensor_power_setting *power_setting = NULL;

	struct msm_sensor_power_setting_array *power_down_setting_array = NULL;
	struct msm_camera_sensor_board_info *data = s_ctrl->sensordata;
	power_setting_array = &s_ctrl->power_setting_array;
	power_down_setting_array = &s_ctrl->power_down_setting_array;

	CDBG("%s:%d enter\n", __func__, __LINE__);

	if (data->gpio_conf->cam_gpiomux_conf_tbl != NULL) {
		pr_err("%s:%d mux install\n", __func__, __LINE__);
		msm_gpiomux_install(
			(struct msm_gpiomux_config *)
			data->gpio_conf->cam_gpiomux_conf_tbl,
			data->gpio_conf->cam_gpiomux_conf_tbl_size);
	}

	rc = msm_camera_request_gpio_table(
		data->gpio_conf->cam_gpio_req_tbl,
		data->gpio_conf->cam_gpio_req_tbl_size, 1);
	if (rc < 0) {
		pr_err("%s: request gpio failed\n", __func__);
		return rc;
	}
	for (index = 0; index < power_setting_array->size; index++) {
		CDBG("%s index %d\n", __func__, index);
		power_setting = &power_setting_array->power_setting[index];
		CDBG("%s type %d\n", __func__, power_setting->seq_type);
		CDBG("%s seq_val %d\n", __func__, power_setting->seq_val);
		switch (power_setting->seq_type) {
		case SENSOR_CLK:
			if (power_setting->seq_val >= s_ctrl->clk_info_size) {
				pr_err("%s clk index %d >= max %d\n", __func__,
					power_setting->seq_val,
					s_ctrl->clk_info_size);
				goto power_up_failed;
			}
			if (power_setting->config_val)
				s_ctrl->clk_info[power_setting->seq_val].
					clk_rate = power_setting->config_val;

			rc = msm_cam_clk_enable(s_ctrl->dev,
				&s_ctrl->clk_info[0],
				(struct clk **)&power_setting->data[0],
				s_ctrl->clk_info_size,
				1);
			CDBG("%s powerup clock num %d", __func__, s_ctrl->clk_info_size);
			if (rc < 0) {
				pr_err("%s: clk enable failed\n",
					__func__);
				goto power_up_failed;
			}

#ifdef CONFIG_HUAWEI_KERNEL_CAMERA
			/*store data[0] for the use of power down*/
			if(s_ctrl->power_down_setting_array.power_setting){
				int32_t i = 0;
				int32_t j = 0;
				struct msm_sensor_power_setting *power_down_setting = NULL;
				for(i=0; i<power_down_setting_array->size; i++){
					power_down_setting = &power_down_setting_array->power_setting[i];
					if(power_setting->seq_val == power_down_setting->seq_val){
						for(j=0; j<s_ctrl->clk_info_size; j++)
                        {
							power_down_setting->data[j] = power_setting->data[j];
							CDBG("%s clkptr %p \n", __func__,power_setting->data[j]);
						}
					}
				}
			}
#endif

			break;
		case SENSOR_GPIO:
			if (power_setting->seq_val >= SENSOR_GPIO_MAX ||
				!data->gpio_conf->gpio_num_info) {
				pr_err("%s gpio index %d >= max %d\n", __func__,
					power_setting->seq_val,
					SENSOR_GPIO_MAX);
				goto power_up_failed;
			}
			pr_debug("%s:%d gpio set val %d\n", __func__, __LINE__,
				data->gpio_conf->gpio_num_info->gpio_num
				[power_setting->seq_val]);
			gpio_set_value_cansleep(
				data->gpio_conf->gpio_num_info->gpio_num
				[power_setting->seq_val],
				power_setting->config_val);		
			break;
		case SENSOR_VREG:
			if (power_setting->seq_val >= CAM_VREG_MAX) {
				pr_err("%s vreg index %d >= max %d\n", __func__,
					power_setting->seq_val,
					SENSOR_GPIO_MAX);
				goto power_up_failed;
			}

			msm_camera_config_single_vreg(s_ctrl->dev,
				&data->cam_vreg[power_setting->seq_val],
				(struct regulator **)&power_setting->data[0],
				1);

#ifdef CONFIG_HUAWEI_KERNEL_CAMERA
			/*store data[0] for the use of power down*/
			if(s_ctrl->power_down_setting_array.power_setting){
				int32_t i = 0;
				struct msm_sensor_power_setting *power_down_setting = NULL;
				for(i=0;i<power_down_setting_array->size; i++){
					power_down_setting = &power_down_setting_array->power_setting[i];
					if(power_setting->seq_val == power_down_setting->seq_val){
						power_down_setting->data[0] = power_setting->data[0];
					}
				}
			}
#endif

			break;
		case SENSOR_I2C_MUX:
			if (data->i2c_conf && data->i2c_conf->use_i2c_mux)
				bf3905_sensor_enable_i2c_mux(data->i2c_conf);
			break;
		default:
			pr_err("%s error power seq type %d\n", __func__,
				power_setting->seq_type);
			break;
		}
		if (power_setting->delay > 20) {
			msleep(power_setting->delay);
		} else if (power_setting->delay) {
			usleep_range(power_setting->delay * 1000,
				(power_setting->delay * 1000) + 1000);
		}
	}

	if (s_ctrl->sensor_device_type == MSM_CAMERA_PLATFORM_DEVICE) {
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_util(
			s_ctrl->sensor_i2c_client, MSM_CCI_INIT);
		if (rc < 0) {
			pr_err("%s cci_init failed\n", __func__);
			goto power_up_failed;
		}
	}

	if (s_ctrl->func_tbl->sensor_match_id)
		rc = s_ctrl->func_tbl->sensor_match_id(s_ctrl);
	else
		rc = msm_sensor_match_id(s_ctrl);
	if (rc < 0) {
		pr_err("%s:%d match id failed rc %d\n", __func__, __LINE__, rc);
		goto power_up_failed;
	}
#ifdef CONFIG_HUAWEI_KERNEL_CAMERA
	if (s_ctrl->func_tbl->sensor_match_module)
		rc = s_ctrl->func_tbl->sensor_match_module(s_ctrl);
	if (rc < 0) {
		pr_err("%s:%d match module failed rc %d\n", __func__, __LINE__, rc);
	}
#endif
	CDBG("%s exit\n", __func__);
	return 0;
power_up_failed:
	CDBG("%s:%d failed\n", __func__, __LINE__);
	if (s_ctrl->sensor_device_type == MSM_CAMERA_PLATFORM_DEVICE) {
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_util(
			s_ctrl->sensor_i2c_client, MSM_CCI_RELEASE);
	}

	if(s_ctrl->power_down_setting_array.power_setting){
		power_setting_array = &s_ctrl->power_down_setting_array;
		index = power_setting_array->size;
		CDBG("power down use the power down settings array\n");
	}
	for (index = 0; index < power_setting_array->size; index++){
		CDBG("%s index %d\n", __func__, index);
		power_setting = &power_setting_array->power_setting[index];
		CDBG("%s type %d\n", __func__, power_setting->seq_type);
		switch (power_setting->seq_type) {
		case SENSOR_CLK:
			msm_cam_clk_enable(s_ctrl->dev,
				&s_ctrl->clk_info[0],
				(struct clk **)&power_setting->data[0],
				s_ctrl->clk_info_size,
				0);
			CDBG("%s powerdown clock num %d\n", __func__, s_ctrl->clk_info_size);
			break;
		case SENSOR_GPIO:
			gpio_set_value_cansleep(
				data->gpio_conf->gpio_num_info->gpio_num
				[power_setting->seq_val], GPIOF_OUT_INIT_LOW);
			break;
		case SENSOR_VREG:
			msm_camera_config_single_vreg(s_ctrl->dev,
				&data->cam_vreg[power_setting->seq_val],
				(struct regulator **)&power_setting->data[0],
				0);
			break;
		case SENSOR_I2C_MUX:
			if (data->i2c_conf && data->i2c_conf->use_i2c_mux)
				bf3905_sensor_disable_i2c_mux(data->i2c_conf);
			break;
		default:
			pr_err("%s error power seq type %d\n", __func__,
				power_setting->seq_type);
			break;
		}
		if (power_setting->delay > 20) {
			msleep(power_setting->delay);
		} else if (power_setting->delay) {
			usleep_range(power_setting->delay * 1000,
				(power_setting->delay * 1000) + 1000);
		}
	}
	msm_camera_request_gpio_table(
		data->gpio_conf->cam_gpio_req_tbl,
		data->gpio_conf->cam_gpio_req_tbl_size, 0);
	return rc;
}

int32_t bf3905_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t index = 0;
	struct msm_sensor_power_setting_array *power_setting_array = NULL;
	struct msm_sensor_power_setting *power_setting = NULL;
	struct msm_camera_sensor_board_info *data = s_ctrl->sensordata;
	s_ctrl->stop_setting_valid = 0;
	power_setting_array = &s_ctrl->power_down_setting_array;

	CDBG("%s:%d enter\n", __func__, __LINE__);
	if (s_ctrl->sensor_device_type == MSM_CAMERA_PLATFORM_DEVICE) {
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_util(
			s_ctrl->sensor_i2c_client, MSM_CCI_RELEASE);
	}

	for (index = 0; index < power_setting_array->size; index++) {
		CDBG("%s index %d\n", __func__, index);
		power_setting = &power_setting_array->power_setting[index];
		CDBG("%s type %d\n", __func__, power_setting->seq_type);
		switch (power_setting->seq_type) {
		case SENSOR_CLK:
			msm_cam_clk_enable(s_ctrl->dev,
				&s_ctrl->clk_info[0],
				(struct clk **)&power_setting->data[0],
				s_ctrl->clk_info_size,
				0);
			CDBG("%s powerdown clock num %d", __func__, s_ctrl->clk_info_size);
			break;
		case SENSOR_GPIO:
			if (power_setting->seq_val >= SENSOR_GPIO_MAX ||
				!data->gpio_conf->gpio_num_info) {
				pr_err("%s gpio index %d >= max %d\n", __func__,
					power_setting->seq_val,
					SENSOR_GPIO_MAX);
				continue;
			}

			gpio_set_value_cansleep(
				data->gpio_conf->gpio_num_info->gpio_num
				[power_setting->seq_val], power_setting->seq_val);
			break;
		case SENSOR_VREG:
			if (power_setting->seq_val >= CAM_VREG_MAX) {
				pr_err("%s vreg index %d >= max %d\n", __func__,
					power_setting->seq_val,
					SENSOR_GPIO_MAX);
				continue;
			}
			msm_camera_config_single_vreg(s_ctrl->dev,
				&data->cam_vreg[power_setting->seq_val],
				(struct regulator **)&power_setting->data[0],
				0);
			break;
		case SENSOR_I2C_MUX:
			if (data->i2c_conf && data->i2c_conf->use_i2c_mux)
				bf3905_sensor_disable_i2c_mux(data->i2c_conf);
			break;
		default:
			pr_err("%s error power seq type %d\n", __func__,
				power_setting->seq_type);
			break;
		}
		if (power_setting->delay > 20) {
			msleep(power_setting->delay);
		} else if (power_setting->delay) {
			usleep_range(power_setting->delay * 1000,
				(power_setting->delay * 1000) + 1000);
		}
	}
	msm_camera_request_gpio_table(
		data->gpio_conf->cam_gpio_req_tbl,
		data->gpio_conf->cam_gpio_req_tbl_size, 0);
	CDBG("%s exit\n", __func__);

	return 0;
}

static int hw_bf3905_byd_match_module(struct msm_sensor_ctrl_t *s_ctrl)
{
	hw_bf3905_module_id = 1;
	
	/*add project name for the project menu*/
	s_ctrl->sensordata->sensor_name = "hw_bf3905_byd";
	strncpy(s_ctrl->sensordata->sensor_info->sensor_project_name, "23060153FF-BYD-B", strlen("23060153FF-BYD-B")+1);
	pr_info("%s %d : hw_mt9v113_match_module sensor_name=%s, sensor_project_name=%s \n",  __func__, __LINE__,
            s_ctrl->sensordata->sensor_name, s_ctrl->sensordata->sensor_info->sensor_project_name);
	pr_info("check module id from camera id PIN:OK \n");
	
	return 0;
}
static struct msm_sensor_fn_t bf3905_sensor_func_tbl = {
    .sensor_config = bf3905_sensor_config,
    .sensor_power_up = bf3905_sensor_power_up,
    .sensor_power_down = bf3905_sensor_power_down,
#ifdef CONFIG_HUAWEI_KERNEL_CAMERA
	.sensor_match_module = hw_bf3905_byd_match_module,
#endif
};

static struct msm_sensor_ctrl_t bf3905_s_ctrl = {
	.sensor_i2c_client = &bf3905_sensor_i2c_client,
	.power_setting_array.power_setting = bf3905_power_setting,
	.power_setting_array.size = ARRAY_SIZE(bf3905_power_setting),
	.msm_sensor_mutex = &hw_bf3905_byd_mut,
	.sensor_v4l2_subdev_info = bf3905_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(bf3905_subdev_info),
	.func_tbl = &bf3905_sensor_func_tbl,
#ifdef CONFIG_HUAWEI_KERNEL_CAMERA
	.power_down_setting_array.power_setting = bf3905_power_down_setting,
	.power_down_setting_array.size = ARRAY_SIZE(bf3905_power_down_setting),
#endif
};

module_init(bf3905_init_module);
module_exit(bf3905_exit_module);
MODULE_DESCRIPTION("BYD 0.3MP YUV sensor driver");
MODULE_LICENSE("GPL v2");
