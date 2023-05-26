package com.zedo.nilouSDK;

import com.fazecast.jSerialComm.SerialPort;

public class Utils {
    public static String getSubStr(String str, String h, String b) {
        int hI = str.indexOf(h);
        if (hI == -1)
            return null;
        String a = str.substring(hI + h.length());
        int bI = a.indexOf(b);
        if (bI == -1)
            return null;
        return a.substring(0, bI);
    }

    public static String[] getSerialPortNameList() {
        SerialPort[] serialPorts = SerialPort.getCommPorts();
        String[] names = new String[serialPorts.length];
        for (int i = 0; i < names.length; i++) {
            names[i] = serialPorts[i].getSystemPortName();
        }
        return names;
    }
}
