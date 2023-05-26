//EEPROM读写库

/*
更新日志
20210419 增加写的函数
20210501 增强存储过程。
*/

#ifndef Z_EEPROM_h
#define Z_EEPROM_h

#include <EEPROM.h> //EEPROM库

//读取EEPROM数据格式化输出(串口调试使用)
//起始行  结束行
void Z_EEPROM_RDHEX(unsigned int Start, unsigned int Stop)
{

    Serial.print("--------EEPROM_READ_HEX-------\n");
    for (unsigned int zz2 = Start; zz2 <= Stop; zz2++) //读取EEPROM数据
    {
        int rd = 0;
        for (unsigned int zz = 0; zz < 8; zz++)
        {
            rd = EEPROM.read(zz + zz2 * 16);

            Serial.print(rd, HEX);
            Serial.print(" ");
        }

        Serial.print("  ");
        for (unsigned int zz = 0; zz < 8; zz++)
        {
            rd = EEPROM.read(8 + zz + zz2 * 16);
            Serial.print(rd, HEX);
            Serial.print(" ");
        }
        Serial.print("| " + String(zz2) + "\n");
    }
}

//存储指定字节数字符串，超出存储范围返回假
//开始字节位 欲存储的字节数 欲存储的变量
bool Z_EEPROM_Write_string_A(uint16_t START_Byte, uint16_t Bytes_Length, const String& i_Data)
{

    int BT_Long = i_Data.length() + 1;
    if (BT_Long <= Bytes_Length)
    {
        char BT_Data[BT_Long];
        i_Data.toCharArray(BT_Data, BT_Long);
        for (int zz = 0; zz < BT_Long; zz++)
        {
            EEPROM.write(START_Byte + zz, BT_Data[zz]);

        }
        return true;
    }
    return false;
}

//读取指定字节数字符串
//开始字节位 欲读取的字节数
String Z_EEPROM_Read_string_A(uint16_t START_Byte, uint16_t Bytes_Length)
{

    String BT_Data = "";
    char BT_Data_2 = 255;
    int zz = 0;
    while (true)
    {
        BT_Data_2 = EEPROM.read(START_Byte + zz);

        if (BT_Data_2 == 0 || Bytes_Length < zz + 1) //判断字符串末尾0x00
        {
            break;
        }
        BT_Data = BT_Data + BT_Data_2;
        zz++;
    }
    /*     BT_Data_2=0;
    BT_Data = BT_Data + BT_Data_2; */
    return BT_Data;
}
//-------EEPROM存储函数-------

//填充字节
//开始字节位 欲填充的字节数 欲被填充的字节
void Z_EEPROM_Write_fill(uint16_t START_Byte, uint16_t Bytes_Length,uint8_t i_Data)
{
    for (int zz = 0; zz < Bytes_Length; zz++)
    {
        EEPROM.write(START_Byte + zz, i_Data);
    }

}

//存储Byte变量 1字节
//开始字节位 欲存储的变量 | 1字节 0-255
void Z_EEPROM_Write_byte(uint16_t START_Byte,uint8_t i_Data)
{

    EEPROM.write(START_Byte, i_Data);

}

//存储short变量 2字节
//开始字节位 欲存储的变量 | 有符号 | 2字节 -32768 ~ 32767
void Z_EEPROM_Write_short(uint16_t START_Byte, short i_Data)
{
   uint8_t BT_Data[2];
    memcpy(BT_Data, &i_Data, 2);
    for (int zz = 0; zz < 2; zz++)
    {
        EEPROM.write(START_Byte + zz, BT_Data[zz]);
    }

}

//存储short变量 2字节
//开始字节位 欲存储的变量 | 无符号 | 2字节 0 ~ 65535
void Z_EEPROM_Write_unsigned_short(uint16_t START_Byte, unsigned short i_Data)
{
   uint8_t BT_Data[2];
    memcpy(BT_Data, &i_Data, 2);
    for (int zz = 0; zz < 2; zz++)
    {
        EEPROM.write(START_Byte + zz, BT_Data[zz]);
    }

}

//存储int变量(2或4字节)
//开始字节位 欲存储的变量 | 有符号 | 2字节 -32768 ~ 32767 | 4字节 -2147483648 ~ 2147483647
void Z_EEPROM_Write_int(uint16_t START_Byte, int i_Data)
{
    int BT_Long = sizeof(int);
   uint8_t BT_Data[BT_Long];
    memcpy(BT_Data, &i_Data, BT_Long);
    for (int zz = 0; zz < BT_Long; zz++)
    {
        EEPROM.write(START_Byte + zz, BT_Data[zz]);
    }

}

//存储unsigned int变量(2或4字节)
//开始字节位 欲存储的变量 | 无符号 | 2字节 0 ~ 65535 |4字节 0 ~ 4294967295
void Z_EEPROM_Write_unsigned_int(uint16_t START_Byte, unsigned int i_Data)
{
    int BT_Long = sizeof(unsigned int);
   uint8_t BT_Data[BT_Long];
    memcpy(BT_Data, &i_Data, BT_Long);
    for (int zz = 0; zz < BT_Long; zz++)
    {
        EEPROM.write(START_Byte + zz, BT_Data[zz]);
    }

}

//存储long变量 4字节
//开始字节位 欲存储的变量 | 有符号 | 4字节 -2147483648 ~ 2147483647
void Z_EEPROM_Write_long(uint16_t START_Byte, long i_Data)
{
    int BT_Long = sizeof(long);
   uint8_t BT_Data[BT_Long];
    memcpy(BT_Data, &i_Data, BT_Long);
    for (int zz = 0; zz < BT_Long; zz++)
    {
        EEPROM.write(START_Byte + zz, BT_Data[zz]);
    }

}

//存储unsigned long变量 4字节
//开始字节位 欲存储的变量 | 有符号 | 4字节 0 ~ 4294967295
void Z_EEPROM_Write_unsigned_long(uint16_t START_Byte, unsigned long i_Data)
{
    int BT_Long = sizeof(unsigned long);
   uint8_t BT_Data[BT_Long];
    memcpy(BT_Data, &i_Data, BT_Long);
    for (int zz = 0; zz < BT_Long; zz++)
    {
        EEPROM.write(START_Byte + zz, BT_Data[zz]);
    }

}

//存储float变量 4字节
//开始字节位 欲存储的变量 | 有符号 | 4字节 -3.4028235E+38 to 3.4028235E+38
void Z_EEPROM_Write_float(uint16_t START_Byte, float i_Data)
{
    int BT_Long = sizeof(float);
   uint8_t BT_Data[BT_Long];
    memcpy(BT_Data, &i_Data, BT_Long);
    for (int zz = 0; zz < BT_Long; zz++)
    {
        EEPROM.write(START_Byte + zz, BT_Data[zz]);
    }

}

//存储double变量 4或8字节
//开始字节位 欲存储的变量 | 有符号 | (懒得查了)
void Z_EEPROM_Write_double(uint16_t START_Byte, double i_Data)
{
    int BT_Long = sizeof(double);
   uint8_t BT_Data[BT_Long];
    memcpy(BT_Data, &i_Data, BT_Long);
    for (int zz = 0; zz < BT_Long; zz++)
    {
        EEPROM.write(START_Byte + zz, BT_Data[zz]);
    }

}

//存储bool变量 1字节
//开始字节位 欲存储的变量 | 1字节 true false
void Z_EEPROM_Write_bool(uint16_t START_Byte, bool i_Data)
{
    int BT_Long = 1;
   uint8_t BT_Data[BT_Long];
    memcpy(BT_Data, &i_Data, BT_Long);
    for (int zz = 0; zz < BT_Long; zz++)
    {
        EEPROM.write(START_Byte + zz, BT_Data[zz]);
    }

}

//存储String变量 多占结束1字节 0x00
//开始字节位 欲存储的变量
void Z_EEPROM_Write_string(uint16_t START_Byte, const String& i_Data)
{

    int BT_Long = i_Data.length() + 1;
    char BT_Data[BT_Long];
    i_Data.toCharArray(BT_Data, BT_Long);
    for (int zz = 0; zz < BT_Long; zz++)
    {
        EEPROM.write(START_Byte + zz, BT_Data[zz]);
    }

}

//----------EEPROM读取函数----------

//读取Byte变量 1字节
//开始字节位  | 1字节 0-255
uint8_t Z_EEPROM_Read_byte(uint8_t START_Byte)
{

    return EEPROM.read(START_Byte);
}

//读取short变量 2字节
//开始字节位  | 有符号 | 2字节 -32768 ~ 32767
short Z_EEPROM_Read_short(uint16_t START_Byte)
{
   uint8_t BT_Data[2];
    for (int zz = 0; zz < 2; zz++)
    {
        BT_Data[zz] = EEPROM.read(START_Byte + zz);
    }
    short BK_Data;
    memcpy(&BK_Data, &BT_Data, 2);
    return BK_Data;
}

//读取unsigned short变量 2字节
//开始字节位  | 无符号 | 2字节 0 ~ 65535
unsigned short Z_EEPROM_Read_unsigned_short(uint16_t START_Byte)
{
   uint8_t BT_Data[2];
    for (int zz = 0; zz < 2; zz++)
    {
        BT_Data[zz] = EEPROM.read(START_Byte + zz);
    }
    unsigned short BK_Data;
    memcpy(&BK_Data, &BT_Data, 2);
    return BK_Data;
}

//读取int变量(2或4字节)
//开始字节位  | 有符号 | 2字节 -32768 ~ 32767 | 4字节 -2147483648 ~ 2147483647
int Z_EEPROM_Read_int(uint16_t START_Byte)
{
    int BT_Long = sizeof(int);
   uint8_t BT_Data[BT_Long];

    for (int zz = 0; zz < BT_Long; zz++)
    {
        BT_Data[zz] = EEPROM.read(START_Byte + zz);
    }
    int BK_Data;
    memcpy(&BK_Data, &BT_Data, BT_Long);
    return BK_Data;
}

//读取unsigned int变量(2或4字节)
//开始字节位  | 无符号 | 2字节 0 ~ 65535 |4字节 0 ~ 4294967295
unsigned int Z_EEPROM_Read_unsigned_int(uint16_t START_Byte)
{
    int BT_Long = sizeof(unsigned int);
   uint8_t BT_Data[BT_Long];

    for (int zz = 0; zz < BT_Long; zz++)
    {
        BT_Data[zz] = EEPROM.read(START_Byte + zz);
    }
    unsigned int BK_Data;
    memcpy(&BK_Data, &BT_Data, BT_Long);
    return BK_Data;
}

//读取long变量 4字节
//开始字节位  | 有符号 | 4字节 -2147483648 ~ 2147483647
long Z_EEPROM_Read_long(uint16_t START_Byte)
{
    int BT_Long = sizeof(long);
   uint8_t BT_Data[BT_Long];

    for (int zz = 0; zz < BT_Long; zz++)
    {
        BT_Data[zz] = EEPROM.read(START_Byte + zz);
    }
    long BK_Data;
    memcpy(&BK_Data, &BT_Data, BT_Long);
    return BK_Data;
}

//读取unsigned long变量 4字节
//开始字节位  | 有符号 | 4字节 0 ~ 4294967295
unsigned long Z_EEPROM_Read_unsigned_long(uint16_t START_Byte)
{
    int BT_Long = sizeof(unsigned long);
   uint8_t BT_Data[BT_Long];

    for (int zz = 0; zz < BT_Long; zz++)
    {
        BT_Data[zz] = EEPROM.read(START_Byte + zz);
    }
    unsigned long BK_Data;
    memcpy(&BK_Data, &BT_Data, BT_Long);
    return BK_Data;
}

//读取float变量 4字节
//开始字节位  | 有符号 | 4字节 -3.4028235E+38 to 3.4028235E+38
float Z_EEPROM_Read_float(uint16_t START_Byte)
{
    int BT_Long = sizeof(float);
   uint8_t BT_Data[BT_Long];

    for (int zz = 0; zz < BT_Long; zz++)
    {
        BT_Data[zz] = EEPROM.read(START_Byte + zz);
    }
    float BK_Data;
    memcpy(&BK_Data, &BT_Data, BT_Long);
    return BK_Data;
}

//读取double变量 4或8字节
//开始字节位  | 有符号 | (懒得查了)
double Z_EEPROM_Read_double(uint16_t START_Byte)
{
    int BT_Long = sizeof(double);
   uint8_t BT_Data[BT_Long];

    for (int zz = 0; zz < BT_Long; zz++)
    {
        BT_Data[zz] = EEPROM.read(START_Byte + zz);
    }
    double BK_Data;
    memcpy(&BK_Data, &BT_Data, BT_Long);
    return BK_Data;
}

//读取bool变量 1字节
//开始字节位  | 1字节 true false
bool Z_EEPROM_Read_bool(uint16_t START_Byte, bool i_Data)
{
    int BT_Long = sizeof(bool);
   uint8_t BT_Data[BT_Long];

    for (int zz = 0; zz < BT_Long; zz++)
    {
        BT_Data[zz] = EEPROM.read(START_Byte + zz);
    }
    bool BK_Data;
    memcpy(&BK_Data, &BT_Data, BT_Long);
    return BK_Data;
}

//读取String变量 查找结束位字节 0x00
//开始字节位
String Z_EEPROM_Read_string(uint16_t START_Byte)
{
    String BT_Data = "";
    uint8_t BT_Data_2 = 255;
    int zz = 0;
    while (true)
    {
        BT_Data_2 = EEPROM.read(START_Byte + zz);

        if (BT_Data_2 == 0) //判断字符串末尾0x00
        {
            break;
        }
        BT_Data = BT_Data + BT_Data_2;
        zz++;
    }
    return BT_Data;
}

//-------EEPROM存储读取函数末尾-------

#endif