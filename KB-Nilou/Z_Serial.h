#ifndef Z_Serial_h
#define Z_Serial_h






//填充字节
//欲填充的字节数 欲被填充的字节
void Z_Serial_Write_fill(unsigned int Bytes_Length, byte i_Data)
{
    for (int zz = 0; zz < Bytes_Length; zz++)
    {
        Serial.write(i_Data);
    }

}

//写入Byte变量 1字节
//欲写入的变量 | 1字节 0-255
void Z_Serial_Write_byte(byte i_Data)
{
    Serial.write(i_Data);
}

//写入Bytes变量 1字节
//欲写入的变量 | 1字节 0-255
void Z_Serial_Write_bytes(char* buf[], uint16_t size)
{
    for (int zz = 0; zz < size; zz++)
    {
        Serial.write(buf[zz]);
    }
}

//写入short变量 2字节
//欲写入的变量 | 有符号 | 2字节 -32768 ~ 32767
void Z_Serial_Write_short(short i_Data)
{
    byte BT_Data[2];
    memcpy(BT_Data, &i_Data, 2);
    for (int zz = 0; zz < 2; zz++)
    {
        Serial.write(BT_Data[zz]);
    }

}

//写入short变量 2字节
//欲写入的变量 | 无符号 | 2字节 0 ~ 65535
void Z_Serial_Write_unsigned_short(unsigned short i_Data)
{
    byte BT_Data[2];
    memcpy(BT_Data, &i_Data, 2);
    for (int zz = 0; zz < 2; zz++)
    {
        Serial.write(BT_Data[zz]);
    }

}

//写入int变量(2或4字节)
//欲写入的变量 | 有符号 | 2字节 -32768 ~ 32767 | 4字节 -2147483648 ~ 2147483647
void Z_Serial_Write_int(int i_Data)
{
    int BT_Long = sizeof(int);
    byte BT_Data[BT_Long];
    memcpy(BT_Data, &i_Data, BT_Long);
    for (int zz = 0; zz < BT_Long; zz++)
    {
        Serial.write(BT_Data[zz]);
    }

}

//写入unsigned int变量(2或4字节)
//欲写入的变量 | 无符号 | 2字节 0 ~ 65535 |4字节 0 ~ 4294967295
void Z_Serial_Write_unsigned_int(unsigned int i_Data)
{
    int BT_Long = sizeof(unsigned int);
    byte BT_Data[BT_Long];
    memcpy(BT_Data, &i_Data, BT_Long);
    for (int zz = 0; zz < BT_Long; zz++)
    {
        Serial.write(BT_Data[zz]);
    }

}

//写入long变量 4字节
//欲写入的变量 | 有符号 | 4字节 -2147483648 ~ 2147483647
void Z_Serial_Write_long(long i_Data)
{
    int BT_Long = sizeof(long);
    byte BT_Data[BT_Long];
    memcpy(BT_Data, &i_Data, BT_Long);
    for (int zz = 0; zz < BT_Long; zz++)
    {
        Serial.write(BT_Data[zz]);
    }

}

//写入unsigned long变量 4字节
//欲写入的变量 | 有符号 | 4字节 0 ~ 4294967295
void Z_Serial_Write_unsigned_long(unsigned long i_Data)
{
    int BT_Long = sizeof(unsigned long);
    byte BT_Data[BT_Long];
    memcpy(BT_Data, &i_Data, BT_Long);
    for (int zz = 0; zz < BT_Long; zz++)
    {
        Serial.write(BT_Data[zz]);
    }

}

//写入float变量 4字节
//欲写入的变量 | 有符号 | 4字节 -3.4028235E+38 to 3.4028235E+38
void Z_Serial_Write_float(float i_Data)
{
    int BT_Long = sizeof(float);
    byte BT_Data[BT_Long];
    memcpy(BT_Data, &i_Data, BT_Long);
    for (int zz = 0; zz < BT_Long; zz++)
    {
        Serial.write(BT_Data[zz]);
    }

}

//写入double变量 4或8字节
//欲写入的变量 | 有符号 | (懒得查了)
void Z_Serial_Write_double(double i_Data)
{
    int BT_Long = sizeof(double);
    byte BT_Data[BT_Long];
    memcpy(BT_Data, &i_Data, BT_Long);
    for (int zz = 0; zz < BT_Long; zz++)
    {
        Serial.write(BT_Data[zz]);
    }

}

//存储String变量 多占结束1字节 0x00
//欲存储的变量
void Z_Serial_Write_string(String i_Data)
{

    int BT_Long = i_Data.length() + 1;
    char BT_Data[BT_Long];
    i_Data.toCharArray(BT_Data, BT_Long);
    for (int zz = 0; zz < BT_Long; zz++)
    {
        Serial.write(BT_Data[zz]);
    }

}


//----------Serial读取函数----------

//读取Byte变量 1字节
// | 1字节 0-255
byte Z_Serial_Read_byte()
{

    return Serial.read();
}
//读取Bytes变量 1字节
// | 1字节 0-255
Z_Serial_Read_bytes(char* buf[], uint16_t size)
{
    for (int zz = 0; zz < size; zz++)
    {
        buf[zz] = Serial.read();
    }
}
//读取short变量 2字节
// | 有符号 | 2字节 -32768 ~ 32767
short Z_Serial_Read_short()
{
    byte BT_Data[2];
    for (int zz = 0; zz < 2; zz++)
    {
        BT_Data[zz] = Serial.read();
    }
    short BK_Data;
    memcpy(&BK_Data, &BT_Data, 2);
    return BK_Data;
}

//读取unsigned short变量 2字节
// | 无符号 | 2字节 0 ~ 65535
unsigned short Z_Serial_Read_unsigned_short()
{
    byte BT_Data[2];
    for (int zz = 0; zz < 2; zz++)
    {
        BT_Data[zz] = Serial.read();
    }
    unsigned short BK_Data;
    memcpy(&BK_Data, &BT_Data, 2);
    return BK_Data;
}

//读取int变量(2或4字节)
// | 有符号 | 2字节 -32768 ~ 32767 | 4字节 -2147483648 ~ 2147483647
int Z_Serial_Read_int()
{
    int BT_Long = sizeof(int);
    byte BT_Data[BT_Long];

    for (int zz = 0; zz < BT_Long; zz++)
    {
        BT_Data[zz] = Serial.read();
    }
    int BK_Data;
    memcpy(&BK_Data, &BT_Data, BT_Long);
    return BK_Data;
}

//读取unsigned int变量(2或4字节)
// | 无符号 | 2字节 0 ~ 65535 |4字节 0 ~ 4294967295
unsigned int Z_Serial_Read_unsigned_int()
{
    int BT_Long = sizeof(unsigned int);
    byte BT_Data[BT_Long];

    for (int zz = 0; zz < BT_Long; zz++)
    {
        BT_Data[zz] = Serial.read();
    }
    unsigned int BK_Data;
    memcpy(&BK_Data, &BT_Data, BT_Long);
    return BK_Data;
}

//读取long变量 4字节
// | 有符号 | 4字节 -2147483648 ~ 2147483647
long Z_Serial_Read_long()
{
    int BT_Long = sizeof(long);
    byte BT_Data[BT_Long];

    for (int zz = 0; zz < BT_Long; zz++)
    {
        BT_Data[zz] = Serial.read();
    }
    long BK_Data;
    memcpy(&BK_Data, &BT_Data, BT_Long);
    return BK_Data;
}

//读取unsigned long变量 4字节
// | 有符号 | 4字节 0 ~ 4294967295
unsigned long Z_Serial_Read_unsigned_long()
{
    int BT_Long = sizeof(unsigned long);
    byte BT_Data[BT_Long];

    for (int zz = 0; zz < BT_Long; zz++)
    {
        BT_Data[zz] = Serial.read();
    }
    unsigned long BK_Data;
    memcpy(&BK_Data, &BT_Data, BT_Long);
    return BK_Data;
}

//读取float变量 4字节
// | 有符号 | 4字节 -3.4028235E+38 to 3.4028235E+38
float Z_Serial_Read_float()
{
    int BT_Long = sizeof(float);
    byte BT_Data[BT_Long];

    for (int zz = 0; zz < BT_Long; zz++)
    {
        BT_Data[zz] = Serial.read();
    }
    float BK_Data;
    memcpy(&BK_Data, &BT_Data, BT_Long);
    return BK_Data;
}

//读取double变量 4或8字节
// | 有符号 | (懒得查了)
double Z_Serial_Read_double()
{
    int BT_Long = sizeof(double);
    byte BT_Data[BT_Long];

    for (int zz = 0; zz < BT_Long; zz++)
    {
        BT_Data[zz] = Serial.read();
    }
    double BK_Data;
    memcpy(&BK_Data, &BT_Data, BT_Long);
    return BK_Data;
}


//读取String变量 查找结束位字节 0x00
//开始字节位
String Z_Serial_Read_string()
{
    String BT_Data = "";
    char BT_Data_2 = 255;
    int zz = 0;
    while (true)
    {
        BT_Data_2 = Serial.read();

        if (BT_Data_2 == 0) //判断字符串末尾0x00
        {
            break;
        }
        BT_Data = BT_Data + BT_Data_2;
        zz++;
    }
    return BT_Data;
}



#endif