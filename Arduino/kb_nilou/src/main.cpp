//------------库-------------
#include <Arduino.h>

#include "FastLED.h"            //FastLED库
#include "HID-Project.h"    //USB通讯库


#include "Z_CFCS.h"        //时钟库
#include "Z_EEPROM.h"        //存储库
#include "Z_Serial.h"       //串口库

//----------宏定义----------

// key数量
#define NUM_KEYS 7

// LED灯带型号
#define LED_TYPE WS2812B

//---------引脚定义---------
#define LED_CONTROL 14          //led控制
#define MODE_SWITCH 3           //模式开关
#define MOTOR 10                //震动马达
//#define AUDIO                 //音频通过串口进行通讯
#define K_D 17                  //旋转编码器D
#define K_B 15                  //旋转编码器B
#define K_A 16                  //旋转编码器A

#define KEY_0 A5                //key0-6
#define KEY_1 A4
#define KEY_2 A3
#define KEY_3 A2
#define KEY_4 A1
#define KEY_5 A0
#define KEY_6 13

//事件定义
enum NilouEvent {
    NE_KEY_PRESS,
    NE_KEY_RELEASE,
    NE_RE_ROTARY_CLOCKWISE,
    NE_RE_ROTARY_COUNTERCLOCKWISE,
    NE_RE_PRESS,
    NE_RE_LONG_PRESS
};

//指令定义
enum CMD {
    CMD_ERROR,
    CMD_HAND_SHAKE,
    CMD_EVENT,
    CMD_READ_C,
    CMD_WRITE_C,
    CMD_PLAY,
    CMD_OK,
    CMD_DEBUG,
    CMD_READ_E,
    CMD_WRITE_E,
    CMD_FLUSH,
    CMD_PING
};


//key引脚
uint8_t keys[NUM_KEYS] = {KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6};

//--------------存储变量---------------

//key全局统计
unsigned long keyPressAll = 0;

//key统计
unsigned long keyPress[NUM_KEYS] = {0, 0, 0, 0, 0, 0, 0};

//运行次数
unsigned long runCount;

//运行时间
unsigned long runTimes;

//key映射
char keyMapping[NUM_KEYS] = {'a', 's', 'd', 'j', 'k', 'l', ' '};

//key按下颜色
CRGB keyPressColor[NUM_KEYS];

//Key默认颜色
CRGB keyReleaseColor;

//音量              0-31
char volume = 0;

//灯光模式          0无灯光 1常亮 2独亮 3独亮闪烁
char lightMode = 0;

//灯光颜色          0自定义颜色 1七彩渐变 2七彩渐变流水 3随机 4纯红 5纯橙 6纯黄 7纯绿 8纯蓝 9纯粉 10纯紫 11白色
char lightColor = 0;

//灯光亮度  * 17    0-15
char lightBrightness = 0;


//震动模式          无震动 轻震动 重震动
char shakeMode = 0;

//key音             keyAu 0无声音 1-32
char keyAudio = 0;

//灯光过渡
float lightTransition = 0.2;

//按下灯光过渡
float keyTSPress = 0.25;

//松开灯光过渡
float keyTSRelease = 0.08;

//--------------内部变量----------------

//key状态
boolean keys_State[NUM_KEYS] = {false, false, false, false, false, false, false};

//key时间状态   负责独立特效
float keys_TimeState[NUM_KEYS] = {0, 0, 0, 0, 0, 0, 0};

//旋转编码器最后一次按下时间
unsigned long K_D_Last = 0;

//旋转编码器按下时间
unsigned long K_D_Press = 0;

//0空 1模式选择 2设置选择 3设置中
short menuMode = 0;

//0普通模式 1设置模式  2音频编辑模式
short modeSelect = 0;

//0音量设置 1灯光模式设置 2灯光颜色设置 3灯光亮度设置 4震动设置 5key音设置 6退出设置
short settingSelect = 0;

//旋转编码器上一次输出
boolean RE_Previous_Output;

// 光带leds
CRGB leds[NUM_KEYS];


int CID_CLOCK_Shake = -1;
int CID_CLOCK_play_num = -1;

//上位机连接超时
unsigned long HO_Time = 0;
bool HO = false;
/*
//------------------常用方法---------------

//取字符串中间      参数 | 字符串 字符串内容 | 字符串 左边内容 | 字符串 右边内容   |  返回 字符串 中间内容
String str_z(const String& str, const String& left, const String& right) {
    int lw = str.indexOf(left) + left.length();    //找查左边内容的结束位置
    String rstr = str.substring(lw, str.length()); //取str左边内容之后的字符串
    int rw = rstr.indexOf(right);                  //找查右边内容的起始位置
    if (str.substring(lw - left.length(), lw) != left || rw == 0) {              //判断欲寻找的内容是否存在
        return ""; //没找到 返回空
    }
    String back = rstr.substring(0, rw);
    return back; //返回值
}
*/

//------------------内部函数---------------------



CRGB EEPROM_Read_CRGB(uint16_t START_Byte) {
    return {
            Z_EEPROM_Read_byte(START_Byte),
            Z_EEPROM_Read_byte(START_Byte + 1),
            Z_EEPROM_Read_byte(START_Byte + 2)
    };
}


//音频命令  命令    参数    参数大小
void send_audio_cmd(uint8_t order, const char *p, uint8_t size) {
    uint8_t verify = 0;
    uint8_t len = 3 + size;
    uint8_t send_order[len + 2];
    send_order[0] = 0x7E;
    send_order[len + 1] = 0xEF;
    send_order[1] = len;
    send_order[2] = order;
    verify += len;
    verify += order;
    for (uint8_t i = 0; i < size; i++) {
        send_order[3 + i] = p[i];
        verify += p[i];
    }
    send_order[len] = verify;

    Serial1.write(send_order, sizeof(send_order));

}

//播放音频  音频名
void play_audio(const String &name) {
    char Buf[sizeof(name)];
    name.toCharArray(Buf, sizeof(name));
    send_audio_cmd(0xA1, Buf, sizeof(Buf));
}

//-----------------

//震动控制时钟
int CLOCK_Shake(int CID) {
    if (shakeMode == 0)
        return -1;
    digitalWrite(MOTOR, HIGH);
    CFCS_Delay((CFCS[CID].C_P == 0 ? 20 : 50) * shakeMode, CID);

    digitalWrite(MOTOR, LOW);
    return -1;
}


//震动  true轻震动    false重震动
void shake(boolean m) {
    Z_CFCS_Del(CID_CLOCK_Shake);
    CID_CLOCK_Shake = Z_CFCS_Add("Shake", 0, CLOCK_Shake);
    CFCS[CID_CLOCK_Shake].C_P = (m ? 0 : -1);
}

//设置音频音量  范围0-31
void set_aduio_voi(uint8_t v) {
    char order[] = {(char) v};
    send_audio_cmd(0xAE, order, sizeof(order));
}

//播放数字 0-99
void play_num(int num, int delay) {
    CFCS[CID_CLOCK_play_num].C_P = num;
    Z_CFCS_Set_Time(CID_CLOCK_play_num, delay);

}


void send_NilouEvent(char type, char num) {
    Z_Serial_Write_byte(0x03);//长度
    Z_Serial_Write_byte(CMD_EVENT);
    Z_Serial_Write_byte(type);
    Z_Serial_Write_byte(num);
    Z_Serial_Write_byte(0xff);//结束符
}

void send_error() {
    Z_Serial_Write_byte(0x01);//长度
    Z_Serial_Write_byte(CMD_ERROR);
    Z_Serial_Write_byte(0xff);//结束符
}

void send_ok() {
    Z_Serial_Write_byte(0x01);//长度
    Z_Serial_Write_byte(CMD_OK);
    Z_Serial_Write_byte(0xff);//结束符
}

void send_debug(const String &msg) {
    char len = msg.length() + 1;
    char cmsg[len];
    msg.toCharArray(cmsg, len);
    Z_Serial_Write_byte(len);//长度
    Z_Serial_Write_byte(CMD_DEBUG);
    for (char i = 0; i < len - 1; i++) {
        Z_Serial_Write_byte(cmsg[i]);//长度
    }

    Z_Serial_Write_byte(0xff);//结束符
}


//重载变量
void flush() {
    //读取设置
    {
        volume = Z_EEPROM_Read_byte(0);
        lightMode = Z_EEPROM_Read_byte(1);
        lightColor = Z_EEPROM_Read_byte(2);
        lightBrightness = Z_EEPROM_Read_byte(3);
        shakeMode = Z_EEPROM_Read_byte(4);
        keyAudio = Z_EEPROM_Read_byte(5);

        lightTransition = Z_EEPROM_Read_float(16);
        keyTSPress = Z_EEPROM_Read_float(20);
        keyTSRelease = Z_EEPROM_Read_float(24);


        keyPressAll = Z_EEPROM_Read_unsigned_long(32);
        {
            runCount = Z_EEPROM_Read_unsigned_long(36);
            runCount++;
            Z_EEPROM_Write_unsigned_long(36, runCount);
        }
        runTimes = Z_EEPROM_Read_unsigned_long(40);


        for (uint8_t i = 0; i < NUM_KEYS; i++) {
            keyPress[i] = Z_EEPROM_Read_unsigned_long(48 + i * 4);
            keyMapping[i] = Z_EEPROM_Read_byte(80 + i);
            keyPressColor[i] = EEPROM_Read_CRGB(112 + i * 3);
        }
        keyReleaseColor = CRGB(
                EEPROM.read(0x90),
                EEPROM.read(145),
                EEPROM.read(146)
        );
        set_aduio_voi(volume);
    }
}


//--------------旋转编码器事件---------------


void Event_HO(bool isConnect) {
    if (isConnect) {
        play_audio("HOc");
    } else {
        play_audio("HOd");
        for (size_t i = 0; i < Serial.available(); i++) {
            Z_Serial_Read_byte();
        }
        Serial.end();
        delay(1000);
        Serial.begin(115200);
    }

    //send_debug("连接" + String(isConnect));
}


//旋转
void RE_Event_Rotary(boolean isClockwise, int CID) {
    send_NilouEvent((isClockwise ? NE_RE_ROTARY_CLOCKWISE : NE_RE_ROTARY_COUNTERCLOCKWISE), 0x0);

    //Serial.println("[Event:RE_ROTARY][V:" + String(isClockwise) + "]");
    play_audio("CH");
    shake(true);
    Z_CFCS_Delay(100, CID);


    switch (menuMode) {
        case 0://0空
            for (char i = 0; i < 5; i++) {
                Consumer.write((isClockwise ? HID_CONSUMER_VOLUME_INCREMENT : HID_CONSUMER_VOLUME_DECREMENT));
            }
            break;
        case 1://1模式选择
            (isClockwise ? modeSelect++ : modeSelect--);
            if (modeSelect < 0)
                modeSelect = 2;
            modeSelect %= 3;
            switch (modeSelect) {
                case 0:
                    play_audio("MNo");
                    break;
                case 1:
                    play_audio("MSe");
                    break;
                case 2:
                    play_audio("MAu");
                    break;
            }
            break;
        case 2://2设置选择
            (isClockwise ? settingSelect++ : settingSelect--);
            if (settingSelect < 0)
                settingSelect = 6;
            settingSelect %= 7;
            //0音量设置 1灯光模式设置 2灯光颜色设置 3灯光亮度设置 4震动设置 5key音设置 6退出设置
            switch (settingSelect) {
                case 0://0音量设置
                    play_audio("SVo");
                    break;
                case 1://1灯光模式设置
                    play_audio("SLm");
                    break;
                case 2://2灯光颜色设置
                    play_audio("SLc");
                    break;
                case 3://3灯光亮度设置
                    play_audio("SLb");
                    break;
                case 4://4震动设置
                    play_audio("SSh");
                    break;
                case 5://5key音设置
                    play_audio("SKe");
                    break;
                case 6://6退出设置
                    play_audio("SEx");
                    break;
            }
            break;
        case 3://3设置中


            //0音量设置 1灯光模式设置 2灯光颜色设置 3灯光亮度设置 4震动设置 5key音设置 6退出设置
            switch (settingSelect) {
                case 0://0音量设置
                    (isClockwise ? volume++ : volume--);
                    if (volume < 0)
                        volume = 31;
                    volume %= 32;
                    set_aduio_voi(volume);
                    Z_EEPROM_Write_byte(0, volume);
                    play_num(volume, 100);
                    break;
                case 1://1灯光模式设置
                    (isClockwise ? lightMode++ : lightMode--);
                    if (lightMode < 0)
                        lightMode = 3;
                    lightMode %= 4;
                    Z_EEPROM_Write_byte(1, lightMode);
                    play_audio("SLm" + String((int) lightMode));
                    break;
                case 2://2灯光颜色设置
                    (isClockwise ? lightColor++ : lightColor--);
                    if (lightColor < 0)
                        lightColor = 11;
                    lightColor %= 12;
                    Z_EEPROM_Write_byte(2, lightColor);

                    if (lightColor == 0) {
                        for (uint8_t i = 0; i < NUM_KEYS; i++) {
                            keyPressColor[i] = EEPROM_Read_CRGB(112 + i * 3);
                        }
                        keyReleaseColor = EEPROM_Read_CRGB(144);

                    }

                    play_audio("SLc" + String((int) lightColor));
                    break;
                case 3://3灯光亮度设置
                    (isClockwise ? lightBrightness++ : lightBrightness--);
                    if (lightBrightness < 0)
                        lightBrightness = 15;
                    lightBrightness %= 16;
                    Z_EEPROM_Write_byte(3, lightBrightness);

                    play_num(lightBrightness, 400);
                    break;
                case 4://4震动设置
                    (isClockwise ? shakeMode++ : shakeMode--);
                    if (shakeMode < 0)
                        shakeMode = 2;
                    shakeMode %= 3;
                    play_audio("SSh" + String((int) shakeMode));
                    Z_EEPROM_Write_byte(4, shakeMode);
                    shake(true);
                    CFCS_Delay_Void(100, CID);
                    shake(true);
                    CFCS_Delay_Void(400, CID);
                    shake(false);
                    break;
                case 5://5key音设置
                    (isClockwise ? keyAudio++ : keyAudio--);
                    if (keyAudio < 0)
                        keyAudio = 32;
                    keyAudio %= 33;
                    Z_EEPROM_Write_byte(5, keyAudio);
                    play_audio("key" + String((int) keyAudio));
                    play_num(keyAudio, 600);
                    //CFCS_Delay(1000, CID);


                    break;
            }


            break;
    }


}

//按下
void RE_Event_Press(int CID) {
    //Serial.println("[Event:RE_PRESS]");
    send_NilouEvent(NE_RE_PRESS, 0x0);
    switch (menuMode) {
        case 0://0空
            shake(false);
            Consumer.write(HID_CONSUMER_MUTE);
            break;


    }
}

//长按
void RE_Event_Long_press(int CID) {
    //Serial.println("[Event:RE_LONG_PRESS]");
    send_NilouEvent(NE_RE_LONG_PRESS, 0x0);
    shake(true);
    Z_CFCS_Delay(50, CID);
    shake(true);


    switch (menuMode)//0空 1模式选择 2设置选择 3设置
    {
        case 0://0空
            play_audio("MCh");
            menuMode = 1;
            break;
        case 1://1模式选择
            //0普通模式 1设置模式  2音频编辑模式
            switch (modeSelect) {
                case 0:
                    play_audio("UDSD");//了解
                    menuMode = 0;
                    break;
                case 1:
                    play_audio("SCh");//设置选择
                    menuMode = 2;
                    settingSelect = 6;//默认退出
                    break;
                case 2:
                    play_audio("UDSD");//了解
                    char temp = lightBrightness;
                    lightBrightness = 0;
                    Z_CFCS_Delay(1000, CID);
                    digitalWrite(MODE_SWITCH, HIGH);//音频编辑
                    fill_solid(leds, NUM_KEYS, CRGB::Blue);
                    while (true) {
                        for (uint8_t i = 0; i < 255; i++) {
                            FastLED.setBrightness(i * (temp / 15.));
                            FastLED.show();
                            if (digitalRead(K_D) == LOW)
                                break;
                            delay(4);
                        }
                        for (uint8_t i = 0; i < 255; i++) {
                            FastLED.setBrightness((255 - i) * (temp / 15.));
                            FastLED.show();
                            if (digitalRead(K_D) == LOW)
                                break;
                            delay(4);
                        }
                        if (digitalRead(K_D) == LOW) {
                            for (uint8_t i = FastLED.getBrightness(); i > 0; i--) {
                                FastLED.setBrightness(i * (temp / 15.));
                                FastLED.show();
                                delay(4);
                            }
                            break;
                        }

                    }

                    digitalWrite(MODE_SWITCH, LOW);//音频编辑
                    Z_CFCS_Delay(1000, CID);
                    shake(true);
                    Z_CFCS_Delay(100, CID);
                    shake(true);
                    Z_CFCS_Delay(1000, CID);
                    lightBrightness = temp;
                    set_aduio_voi(volume);//设置音量
                    Z_CFCS_Delay(600, CID);
                    play_audio("MAu");//设置选择
                    break;
            }
            break;
        case 2://2设置选择
            //0音量设置 1灯光模式设置 2灯光颜色设置 3灯光亮度设置 4震动设置 5key音设置 6退出设置
            menuMode = 3;
            switch (settingSelect) {
                case 0:
                    play_num(volume, 400);
                    break;
                case 1:
                    play_audio("SLm" + String((int) lightMode));
                    break;
                case 2:
                    play_audio("SLc" + String((int) lightColor));
                    break;
                case 3:
                    play_num(lightBrightness, 400);
                    break;
                case 4:
                    play_audio("SSh" + String((int) shakeMode));
                    break;
                case 5:
                    play_audio("key" + String((int) keyAudio));
                    play_num(keyAudio, 600);
                    break;
                case 6:
                    play_audio("MSe");//了解
                    menuMode = 1;
                    break;
            }
            break;
        case 3://3设置中
            menuMode = 2;
            //play_audio("SCh");
            //Z_CFCS_Delay(2000, CID);
            //0音量设置 1灯光模式设置 2灯光颜色设置 3灯光亮度设置 4震动设置 5key音设置 6退出设置
            switch (settingSelect) {
                case 0://0音量设置
                    play_audio("SVo");
                    break;
                case 1://1灯光模式设置
                    play_audio("SLm");
                    break;
                case 2://2灯光颜色设置
                    play_audio("SLc");
                    break;
                case 3://3灯光亮度设置
                    play_audio("SLb");
                    break;
                case 4://4震动设置
                    play_audio("SSh");
                    break;
                case 5://5key音设置
                    play_audio("SKe");
                    break;
                case 6://6退出设置
                    play_audio("MSe");
                    break;
            }
            break;
    }

}

//--------------KEY事件---------------

//按下
void KEY_Event_Press(uint8_t i) {
    keyPressAll++;
    Z_EEPROM_Write_unsigned_long(32, keyPressAll);
    keyPress[i]++;
    Z_EEPROM_Write_unsigned_long(48 + 4 * i, keyPress[i]);
    if (lightColor == 3)//按下随机颜色
        keyPressColor[i] = CHSV(random8(255), 255, 255);
    BootKeyboard.press(keyMapping[i]);
    send_NilouEvent(NE_KEY_PRESS, i);
    //Serial.println("[Event:KEY_PRESS][V:" + String((int)i) + "]");
    play_audio("key" + String((int) keyAudio));

}

//松开
void KEY_Event_Release(uint8_t i) {
    BootKeyboard.release(keyMapping[i]);
    send_NilouEvent(NE_KEY_RELEASE, i);
    //Serial.println("[Event:KEY_RELEASE][V:" + String((int)i) + "]");
}



//-----------------时钟-------------------

//灯效控制时钟
int CLOCK_Key_Light(int CID) {


    CFCS[CID].C_P = (CFCS[CID].C_P + 1) % 256;


    FastLED.setBrightness(
            FastLED.getBrightness() + ((lightBrightness * 17) - FastLED.getBrightness()) * lightTransition);


    //0自定义颜色 1七彩渐变 2常亮 3随机
    //debug(String(keys_TimeState[0]));
    for (uint8_t i = 0; i < NUM_KEYS; i++) {
        //处理key时间状态
        if (keys_State[i]) {
            if (keys_TimeState[i] > 0) {
                keys_TimeState[i] -= keyTSRelease;
            } else {
                keys_TimeState[i] = 0;
            }


        } else {
            if (keys_TimeState[i] < 1) {a
                keys_TimeState[i] += keyTSPress;
            } else {
                keys_TimeState[i] = 1;
            }
        }


        switch (lightColor) {
            case 0:
                break;
            case 1:
                keyPressColor[i] = CHSV(CFCS[CID].C_P, 255, 255);
                break;
            case 2:
                keyPressColor[i] = CHSV(CFCS[CID].C_P + i * 20, 255, 255);
                break;

            case 4:
                keyPressColor[i] = CRGB(255, 0, 0);
                break;
            case 5:
                keyPressColor[i] = CRGB(255, 165, 0);
                break;
            case 6:
                keyPressColor[i] = CRGB(255, 255, 0);
                break;
            case 7:
                keyPressColor[i] = CRGB(0, 255, 0);
                break;
            case 8:
                keyPressColor[i] = CRGB(0, 0, 255);
                break;
            case 9:
                keyPressColor[i] = CRGB(180, 0, 100);
                break;
            case 10:
                keyPressColor[i] = CRGB(140, 0, 200);
                break;
            case 11:
                keyPressColor[i] = CRGB(255, 255, 255);
                break;
        }

    }



    //send_debug("RGB: r:" + String(keyReleaseColor.r) + " g:" + String(keyReleaseColor.g) + " b:" + String(keyReleaseColor.b));

    for (uint8_t i = 0; i < NUM_KEYS; i++) {

        if (lightMode == 2 || lightMode == 3) {
            leds[i] = CRGB(
                    (leds[i].r + (keyPressColor[i].r - leds[i].r) * lightTransition) * keys_TimeState[i] +
                    (leds[i].r + (keyReleaseColor.r - leds[i].r) * lightTransition) * (1.0 - keys_TimeState[i]),
                    (leds[i].g + (keyPressColor[i].g - leds[i].g) * lightTransition) * keys_TimeState[i] +
                    (leds[i].g + (keyReleaseColor.g - leds[i].g) * lightTransition) * (1.0 - keys_TimeState[i]),
                    (leds[i].b + (keyPressColor[i].b - leds[i].b) * lightTransition) * keys_TimeState[i] +
                    (leds[i].b + (keyReleaseColor.b - leds[i].b) * lightTransition) * (1.0 - keys_TimeState[i]));
        }


        switch (lightMode) {
            case 0:
                leds[i] = CRGB(0, 0, 0);
                break;
            case 1:
                leds[i] = CRGB(leds[i].r + (keyPressColor[i].r - leds[i].r) * lightTransition,
                               leds[i].g + (keyPressColor[i].g - leds[i].g) * lightTransition,
                               leds[i].b + (keyPressColor[i].b - leds[i].b) * lightTransition);
                break;
            case 2:

                break;
            case 3:
                if (millis() % (16 * 5) < 18) {
                    leds[i] = CRGB(0, 0, 0);
                }

                break;
        }
    }



    /* switch (lightMode)//无灯光 自定义灯光 流水灯 常亮
    {
    case 0://无灯光
        fill_solid(leds, NUM_KEYS, CRGB(0, 0, 0));
        break;
    case 1://自定义灯光
        fill_solid(leds, NUM_KEYS, CRGB(0, 0, 0));
        break;
    case 2://流水灯

        CFCS[CID].C_P = (CFCS[CID].C_P + 1) % 255;
        fill_rainbow(leds, NUM_KEYS, CFCS[CID].C_P, 16);
        debug(String(CFCS[CID].C_P));
        break;
    default:
        break;
    } */



    FastLED.show();

    return 0;
}

//播放数字时钟
int CLOCK_play_num(int CID) {
    Z_CFCS_Set_Time(CID_CLOCK_play_num, -1);
    int num = CFCS[CID].C_P;

    //Serial.println("播放:" + String(num));
    if (num <= 10) {
        play_audio("Nu" + String(num));
    } else if (num >= 11 && num <= 19) {
        play_audio("Nu10");
        CFCS_Delay_L(500, CID);
        play_audio("Nu" + String(num % 10));
    } else if (num >= 20) {
        play_audio("Nu" + String(num / 10));
        CFCS_Delay_L(500, CID);
        play_audio("Nu10");
        if (num % 10 != 0) {
            CFCS_Delay_L(500, CID);
            play_audio("Nu" + String(num % 10));
        }
    }

    return 0;
}

//运行时间统计时钟
int CLOCK_RunTimes(int CID) {
    runTimes++;
    Z_EEPROM_Write_unsigned_long(40, runTimes);
    return 0;
}

//系统检测时钟
int CLOCK_SYS(int CID) {


    //上位机检测超时
    {
        if (HO)
            if (millis() - 2000 > HO_Time) {
                HO = false;
                Event_HO(false);
            }
    }







    //旋转编码器检测
    {
        //旋转编码器 按下检测
        if (digitalRead(K_D) == LOW) {
            Z_CFCS_Delay(10, CID);//防抖处理
            if (digitalRead(K_D) != LOW)
                return 0;
            K_D_Press = millis();
            shake(true);
            while (true) {
                if (digitalRead(K_D)) {
                    Z_CFCS_Delay(10, CID);//防抖处理
                    if (digitalRead(K_D))
                        break;
                }
                Z_CFCS_Delay(1, CID);
            }//等待松开
            if (millis() > K_D_Press + 500) //长按
                RE_Event_Long_press(CID);
            else//单按
                RE_Event_Press(CID);
        }

        if (digitalRead(K_A) != RE_Previous_Output) {
            //是否是顺时针
            boolean isClockwise = (digitalRead(K_B) != RE_Previous_Output);
            RE_Event_Rotary(isClockwise, CID);
        }
        RE_Previous_Output = digitalRead(K_A);
    }

    return 0;
}

//串口时钟
int CLOCK_Serial(int CID) {
    if (Serial.available() > 0) {
        uint8_t cmd = Z_Serial_Read_byte();//读取命令

        switch (cmd)//判断命令
        {
            case CMD_HAND_SHAKE:
                if (Z_Serial_Read_byte() == 'K')
                    if (Z_Serial_Read_byte() == 'B')
                        if (Z_Serial_Read_byte() == 'N')
                            if (Z_Serial_Read_byte() == 'L') {
                                Z_Serial_Write_byte(0x06);//长度
                                Z_Serial_Write_byte(CMD_HAND_SHAKE);
                                Z_Serial_Write_byte('K');
                                Z_Serial_Write_byte('B');
                                Z_Serial_Write_byte('N');
                                Z_Serial_Write_byte('L');
                                Z_Serial_Write_byte(0x00);//版本
                                Z_Serial_Write_byte(0xff);//结束符
                                HO = true;
                                HO_Time = millis();

                                Event_HO(true);
                                break;
                            }
                send_error();
                break;
            case CMD_READ_C: {
                uint8_t id = Z_Serial_Read_byte();//读编号

                Z_Serial_Write_byte(4);//长度
                Z_Serial_Write_byte(CMD_READ_C);
                Z_Serial_Write_byte(leds[id].r);
                Z_Serial_Write_byte(leds[id].g);
                Z_Serial_Write_byte(leds[id].b);
                Z_Serial_Write_byte(0xff);//结束符


                break;
            }
            case CMD_WRITE_C: {
                uint8_t id = Z_Serial_Read_byte();//读编号
                keyPressColor[id].r = Z_Serial_Read_byte();
                keyPressColor[id].g = Z_Serial_Read_byte();
                keyPressColor[id].b = Z_Serial_Read_byte();
                break;
            }
            case CMD_PLAY: {
                uint8_t len = Z_Serial_Read_byte();//读长度
                char au_name[len];
                for (int zz = 0; zz < len; zz++) {
                    au_name[zz] = Z_Serial_Read_byte();
                }
                play_audio(String(au_name));
                break;
            }
            case CMD_READ_E: {
                uint8_t pos = Z_Serial_Read_byte();//读地址
                uint8_t len = Z_Serial_Read_byte();//读长度


                Z_Serial_Write_byte(len + 1);//长度
                Z_Serial_Write_byte(CMD_READ_E);
                for (uint8_t zz = 0; zz < len; zz++) {
                    Z_Serial_Write_byte(Z_EEPROM_Read_byte(pos + zz));
                }

                Z_Serial_Write_byte(0xff);//结束符

                break;
            }
            case CMD_WRITE_E: {
                uint8_t pos = Z_Serial_Read_byte();//读地址
                uint8_t len = Z_Serial_Read_byte();//读长度


                for (uint8_t zz = 0; zz < len; zz++) {
                    Z_EEPROM_Write_byte(pos + zz, Z_Serial_Read_byte());
                }

                send_ok();
                break;
            }
            case CMD_FLUSH: {
                flush();
                break;
            }
            case CMD_PING: {
                Z_Serial_Write_byte(0x01);//长度
                Z_Serial_Write_byte(CMD_PING);
                Z_Serial_Write_byte(0xff);//结束符
                HO_Time = millis();
                break;
            }
            default: {
                for (size_t i = 0; i < Serial.available(); i++) {
                    Z_Serial_Read_byte();
                }
                send_error();
                break;
            }
        }

        return 0;
    }
}





//---------------初始化/循环---------------

//初始化
void setup() {



    //初始化引脚
    pinMode(LED_CONTROL, OUTPUT);
    pinMode(MODE_SWITCH, OUTPUT);
    pinMode(MOTOR, OUTPUT);
    pinMode(K_D, INPUT_PULLUP);
    pinMode(K_B, INPUT_PULLUP);
    pinMode(K_A, INPUT_PULLUP);

    for (uint8_t i = 0; i < NUM_KEYS; i++) {
        pinMode(keys[i], INPUT_PULLUP);
        keys_State[i] = true;
    }


    Z_CFCS_Init();

    EEPROM.begin();   //EEPROM申请空间 1k
    Serial.begin(115200); //串口通信
    Serial.setTimeout(200);//设置串口超时
    Serial1.begin(9600); //语音芯片串口通信

    LEDS.addLeds<LED_TYPE, LED_CONTROL, RGB>(leds, NUM_KEYS);   // 初始化光带
    FastLED.clear();


    flush();//读取配置

    delay(400);
    if (digitalRead(K_D) != LOW) {
        BootKeyboard.begin();
        Consumer.begin();
    }


    play_audio("Wel");//播放欢迎音频
    //shake(true);
    //lightBrightness = 0;
    //开机灯效
    for (int a = 0; a < 255; a++) {
        fill_rainbow(leds, NUM_KEYS, a, 20);
        FastLED.setBrightness(a);
        FastLED.show();
        delay(4);
    }
    for (int a = 0; a < 255; a++) {
        fill_rainbow(leds, NUM_KEYS, a, 20);
        FastLED.setBrightness(255 - a);
        FastLED.show();
        delay(4);
    }
    delay(500);
    FastLED.setBrightness(0);



    //旋转编码器 读取初始值
    RE_Previous_Output = digitalRead(K_B);

    //初始化时钟
    Z_CFCS_Add("Key_Light", 16, CLOCK_Key_Light);
    Z_CFCS_Add("SYS", 1, CLOCK_SYS);
    Z_CFCS_Add("Serial", 1, CLOCK_Serial);
    Z_CFCS_Add("RunTimes", 10000, CLOCK_RunTimes);
    CID_CLOCK_play_num = Z_CFCS_Add("playNum", -1, CLOCK_play_num);


    //Z_EEPROM_RDHEX(0,16);
}

//循环执行
void loop() {
    //时钟函数调用系统
    Z_CFCS_LOOP(-1);



    //key状态检测
    for (size_t i = 0; i < sizeof(keys); i++) {
        if (digitalRead(keys[i]) == LOW) {
            if (keys_State[i]) {
                keys_State[i] = !keys_State[i];
                KEY_Event_Press(i);
            }
        } else {
            if (!keys_State[i]) {
                keys_State[i] = !keys_State[i];
                KEY_Event_Release(i);
            }
        }

    }

}
