//时钟函数调用系统      可以实现某个函数被一定周期调用
boolean DEBUG_MOD = true;

#ifndef Z_CFCS_h
#define Z_CFCS_h

#define CFCS_N 40 //时钟函数调用系统的时钟上限


#define CFCS_Delay(ms,CID)  {CFCS[CID].C_CFCS_D = true;if(Z_CFCS_Delay_(ms, CID))return -1;}
#define CFCS_Delay_Void(ms,CID)  {CFCS[CID].C_CFCS_D = true;if(Z_CFCS_Delay_(ms, CID))return;}
#define CFCS_Delay_L(ms,CID)  {CFCS[CID].C_CFCS_D = true;if(Z_CFCS_Delay_(ms, CID))return 0;}
#define Z_CFCS_Delay(ms,CID)  {CFCS[CID].C_CFCS_D = true;Z_CFCS_Delay_(ms, CID);}
int Z_CFCS_CID = -1;       //当前执行的时钟
int Z_CFCS_Delay_CID = -1; //当前等待时钟

typedef int (*Function_pointer)(int CID); // 函数指针 被调用函数需要 CID int参数   返回int参数    -1删除自身   -2无视超时检测
typedef struct CLOCK_Function             //时钟函数结构体
{
    bool isStop = false;          //是否结束
    bool isDel = false;          //是否停止
    String C_Name;               //时钟名 为空时无视
    Function_pointer C_Function{}; //被调用的函数指针
    unsigned long C_Time{};        //时钟周期(ms)
    unsigned long C_L_Time{};      //上一次执行时间
    unsigned long C_R_L_Time{};    //执行之前的时间
    int C_P{};                     //自定义参数
    bool C_CFCS_D{};               //是否使用了CFCS_Delay 防止调用
};
CLOCK_Function CFCS[CFCS_N]; //时钟函数调用系统     Clock function call system


void Z_CFCS_Init() {
    for (auto & a_CID : CFCS)
    {
        a_CID.isStop = true;
    }

}

/* //CFCS打印调试
void Z_CFCS_Debug()
{
    if (DEBUG_MOD) //DEBUG
        for (int a_CID = 0; a_CID < CFCS_N; a_CID++)
        {
            if (CFCS[a_CID].C_Name != "") {
                Serial.println("---ID:" + String(a_CID) + "---");
                Serial.println("name:" + CFCS[a_CID].C_Name);
                Serial.println("isDel:" + String(CFCS[a_CID].isDel));
                Serial.println("isStop:" + String(CFCS[a_CID].isStop));
            }

        }
}
 */


 //CFCS查找一个时钟的ID 时钟名
int Z_CFCS_FD(const String& Name)
{
    for (int a_CID = 0; a_CID < CFCS_N; a_CID++)
    {
        if (CFCS[a_CID].C_Name == Name)
        {
            return a_CID;
        }
    }
    return -1;
}

//CFCS添加一个时钟 时钟名 时钟周期 函数地址(CID int)(返回 int 为-1时删除自身)   成功返回CID 失败返回-1
int Z_CFCS_Add(const String& Name, unsigned long Time, Function_pointer Function)
{
    for (int a_CID = 0; a_CID < CFCS_N; a_CID++)
    {
        if (CFCS[a_CID].C_Name == "" && CFCS[a_CID].isStop)
        {
            CFCS[a_CID].C_L_Time = millis();
            CFCS[a_CID].C_R_L_Time = CFCS[a_CID].C_L_Time;
            CFCS[a_CID].C_Time = Time;
            CFCS[a_CID].C_Function = Function;
            CFCS[a_CID].C_Name = Name;
            CFCS[a_CID].C_CFCS_D = false;
            CFCS[a_CID].isDel = false;
            CFCS[a_CID].isStop = false;
            //Z_CFCS_Debug();
            //Serial.println("创建" + String(a_CID));
            return a_CID;
        }
    }
    return -1;
}


//CFCS删除一个时钟 CID(强行终止)
bool Z_CFCS_Del(int CID)
{
    if (CID != -1)
    {
        CFCS[CID].isStop = true;
        return true;
    }
    return false;
}

//CFCS设置时钟周期 CID
bool Z_CFCS_Set_Time(int CID, unsigned long Time)
{
    if (CID != -1)
    {
        CFCS[CID].C_Time = Time;
        CFCS[CID].C_L_Time = millis();
        CFCS[CID].C_R_L_Time = CFCS[CID].C_L_Time;
        return true;
    }
    return false;
}

//CFCS删除一个时钟 时钟名(强行终止)
bool Z_CFCS_Del_S(const String& Name)
{
    int CID = Z_CFCS_FD(Name);
    return Z_CFCS_Del(CID);
}



//循环检测时钟
void Z_CFCS_LOOP(int D_CID)
{
    for (int a_CID = 0; a_CID < CFCS_N; a_CID++)
    {
        //ESP.wdtFeed(); //ESP喂看门狗
        if (CFCS[a_CID].C_Name != "") //检测CFCS是否可用
        {
            if (D_CID != a_CID) //检测是否为等待时钟
            {
                if (CFCS[a_CID].C_CFCS_D && D_CID != -1)
                {
                    continue;
                }
                unsigned long temp_N_T = millis();                         //取启动运行时间ms


                if (CFCS[a_CID].C_Time < 0)
                    continue;

                if (temp_N_T - CFCS[a_CID].C_L_Time >= CFCS[a_CID].C_Time) //检测时间是否够
                {
                    CFCS[a_CID].C_L_Time = temp_N_T;
                    CFCS[a_CID].C_R_L_Time = temp_N_T;
                    Z_CFCS_CID = a_CID;
                    int CLOCK_B = CFCS[a_CID].C_Function(a_CID);
                    Z_CFCS_CID = -1;
                    /* if (CLOCK_B != -2)
                    {
                        if (DEBUG_MOD) //DEBUG
                        {

                            unsigned long temp_T2 = millis();
                            if (temp_T2 - CFCS[a_CID].C_R_L_Time > 100)
                            {
                                Serial.println("CFCS:警告!有个时钟执行超时!时钟名:" + CFCS[a_CID].C_Name + " 时钟周期:" + String(CFCS[a_CID].C_Time) + "ms 所花时间:" + String(temp_T2 - CFCS[a_CID].C_R_L_Time) + "ms");
                            }
                        }
                    } */

                    if (CLOCK_B == -1)
                    { //返回-1关闭时钟
                        //Serial.println("关闭时钟:" + String(a_CID));
                        CFCS[a_CID].isStop = true;
                        CFCS[a_CID].C_Name = "";
                        CFCS[a_CID].isDel = true;
                        CFCS[a_CID].C_Function = nullptr;
                        CFCS[a_CID].C_Time = 0;
                        CFCS[a_CID].C_L_Time = 0;
                        CFCS[a_CID].C_R_L_Time = 0;
                        CFCS[a_CID].C_P = 0;
                        CFCS[a_CID].C_CFCS_D = false;
                    }


                }




            }
        }

    }
}



//CFCS延迟函数 等待途中继续执行CFCS 如果被停止 返回true
boolean Z_CFCS_Delay_(unsigned long ms, int CID)
{


    unsigned long temp_N_T = millis();
    while (millis() < temp_N_T + ms)
    {
        if (CFCS[CID].isDel || CFCS[CID].C_Name == "")
        {
            return true;
        }
        Z_CFCS_LOOP(CID);
    }
    CFCS[CID].C_R_L_Time = CFCS[CID].C_R_L_Time + (millis() - temp_N_T);
    return false;
}

#endif