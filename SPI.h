

#ifndef __SPI_H
#define __SPI_H




////////////////////////////////////////////////////////////////////////////
//==**"外部" API 函数声明**Exported  API funcitions**===============//
////////////////////////////////////////////////////////////////////////////

extern void Initial_SPI_B(void);  //SPI_B 模块初始化:端口初始化、配置SPI功能

extern void Send_OneByte_SPI_B(unsigned char byte) ;  //SPI_B 发送一个字节数据-- "SPI 同步串行外设接口" 




#endif  


