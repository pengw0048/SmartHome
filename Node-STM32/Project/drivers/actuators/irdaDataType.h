/*****************************************************
*
*制作人：   
*
* 单位:    深圳市宏芯达科技有限公司
*
* 部门:   技术部
*
* 目的：  定义数据类型
*
* 日期:   2013年夏
******************************************************/

#ifndef __DATATYPE_H__
#define __DATATYPE_H__
typedef		unsigned	char		czx_u8;
typedef		unsigned	short	czx_u16;
typedef		unsigned	long		czx_u32;

typedef		unsigned	char	const	czx_uc8;
typedef		unsigned	short const	czx_uc16;
typedef		unsigned	long	const	czx_uc32;

typedef		signed		char	czx_s8;
typedef		signed		short	czx_s16;
typedef		signed		long	czx_s32;

typedef		signed		char	const	czx_sc8;
typedef		signed		short	const	czx_sc16;
typedef		signed		long	const	czx_sc32;

typedef		volatile	unsigned	char	czx_vu8;
typedef		volatile	unsigned	short	czx_vu16;
typedef		volatile	unsigned	long	czx_vu32;

typedef		volatile	signed	char	czx_vs8;
typedef		volatile	signed	short	czx_vs16;
typedef		volatile	signed	long	czx_vs32;

typedef		volatile	unsigned	char	const	czx_vuc8;
typedef		volatile	unsigned	short	const	czx_vuc16;
typedef		volatile	unsigned	long	const	czx_vuc32;

typedef		volatile	signed	char	const	czx_vsc8;
typedef		volatile	signed	short	const	czx_vsc16;
typedef		volatile	signed	long	const	czx_vsc32;

#define		czx_bool	czx_vu8  
// #define		TRUE	1
// #define		FALSE	0
//#define		ENABLE	1
//#define		DISABLE	0
// #define		NULL	0
typedef	union{
	struct{
		czx_vu8	port0	:1;
		czx_vu8	port1	:1;
		czx_vu8	port2	:1;
		czx_vu8	port3	:1;
		czx_vu8	port4	:1;
		czx_vu8	port5	:1;
		czx_vu8	port6	:1;
		czx_vu8	port7	:1;
	}port_bit;
	czx_vu8	port;
}czx_port8;

#endif // __DATATYPE_H__
