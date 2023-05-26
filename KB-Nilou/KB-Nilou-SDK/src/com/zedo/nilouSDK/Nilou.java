package com.zedo.nilouSDK;

import com.fazecast.jSerialComm.SerialPort;
import com.fazecast.jSerialComm.SerialPortDataListener;
import com.fazecast.jSerialComm.SerialPortEvent;

import java.nio.ByteBuffer;
import java.util.ArrayList;

public class Nilou {
    private SerialPort serialPort;//串口对象
    private final byte checkVersion = 0;//校验版本
    private final ArrayList<Byte> serialBuf = new ArrayList<>();//串口数据缓冲区

    private byte[] readBytesBuf = null;//读取字节缓冲区
    private int readBytesLen = 0;//读取字节长度
    private boolean pingB = false;//ping成功

    private Thread pingThread = null;//ping线程
    private Color readColorBuf = null;//读取颜色缓冲区
    public boolean debug = false;//调试

    private boolean isEnable = false;//启用状态

    private NilouEventCallback nilouEventCallback = null;//事件回调

    /**
     * 是否开启
     *
     * @return 状态
     */
    public boolean isOpened() {
        return isEnable;
    }

    /**
     * 播放音频
     *
     * @param name 音频名(长度不能超过8)
     */
    public void playAudio(String name) {
        byte[] auName = name.getBytes();
        byte[] data = new byte[name.length() + 2];
        data[0] = (byte) CMD.CMD_PLAY.ordinal();
        data[1] = (byte) auName.length;
        System.arraycopy(auName, 0, data, 2, auName.length);
        sendData(data);
    }

    /**
     * 读取存储字节
     *
     * @param pos 地址
     * @param len 长度
     * @return 返回数据
     */
    public byte[] readBytes(int pos, int len) {
        byte[] data = new byte[3];
        data[0] = (byte) CMD.CMD_READ_E.ordinal();
        data[1] = BytesUtils.int2byte(pos);
        data[2] = BytesUtils.int2byte(len);
        sendData(data);
        readBytesLen = len;
        while (true) {
            if (readBytesBuf != null) {
                byte[] buf = readBytesBuf.clone();
                readBytesBuf = null;
                return buf;
            }
            try {
                Thread.sleep(1);
            } catch (InterruptedException e) {
                return null;
            }
        }
    }

    /**
     * 写存储字节
     *
     * @param pos 地址
     * @param buf 要存储的数据
     */
    public void writeBytes(int pos, byte[] buf) {
        byte[] data = new byte[3 + buf.length];
        data[0] = (byte) CMD.CMD_WRITE_E.ordinal();
        data[1] = BytesUtils.int2byte(pos);
        data[2] = BytesUtils.int2byte(buf.length);
        System.arraycopy(buf, 0, data, 3, buf.length);
        sendData(data);
    }

    public void writeBytes(int pos, byte buf) {
        byte[] data = new byte[4];
        data[0] = (byte) CMD.CMD_WRITE_E.ordinal();
        data[1] = BytesUtils.int2byte(pos);
        data[2] = BytesUtils.int2byte(1);
        data[3] = buf;
        sendData(data);
    }


    public void addEventListener(NilouEventCallback nilouEventCallback) {
        this.nilouEventCallback = nilouEventCallback;
    }

    /**
     * 重载数据
     */
    public void flush() {
        byte[] data = new byte[1];
        data[0] = (byte) CMD.CMD_FLUSH.ordinal();

        sendData(data);
    }

    /**
     * 读取键颜色
     *
     * @param index 索引 0-6
     * @return 返回键的当前颜色
     */
    public Color readColor(int index) {
        byte[] data = new byte[2];
        data[0] = (byte) CMD.CMD_READ_C.ordinal();
        data[1] = BytesUtils.int2byte(index);
        sendData(data);
        while (true) {
            if (readColorBuf != null) {
                Color readColorBuf_ = readColorBuf.clone();
                readColorBuf = null;
                return readColorBuf_;
            }
            try {
                Thread.sleep(1);
            } catch (InterruptedException e) {
                return null;
            }
        }
    }

    /**
     * 写键颜色(预期值)
     *
     * @param index 索引 0-6
     * @param color 欲设置的颜色
     */
    public void writeColor(int index, Color color) {
        byte[] data = new byte[5];
        data[0] = (byte) CMD.CMD_WRITE_C.ordinal();
        data[1] = (byte) index;
        data[2] = (byte) color.getRI();
        data[3] = (byte) color.getGI();
        data[4] = (byte) color.getBI();
        sendData(data);
    }

    /**
     * 测试连接
     *
     * @return 返回连接速度(ms)
     */
    public long ping() {
        byte[] data = new byte[1];
        data[0] = (byte) CMD.CMD_PING.ordinal();
        sendData(data);
        long pingT = System.currentTimeMillis();//ping开始时间
        while (true) {
            if (System.currentTimeMillis() - 200 > pingT)
                return -1;
            if (pingB)
                break;
        }
        return System.currentTimeMillis() - pingT;
    }

    /**
     * 设置串口端口
     *
     * @param portName 端口名
     */
    private void setSerialPort(String portName) {
        if (serialPort != null) serialPort.closePort();
        for (SerialPort serialPort1 : SerialPort.getCommPorts()) {
            if (serialPort1.getSystemPortName().equals(portName)) {
                serialPort = serialPort1;
                return;
            }
        }
        throw new RuntimeException("没有找到串口");
    }

    /**
     * 打开串口
     */
    private void openSerialPort() {
        serialPort.setBaudRate(115200);
        serialPort.setNumDataBits(8);
        serialPort.setNumStopBits(1);
        if (!serialPort.openPort()) throw new RuntimeException("串口打开失败");

        serialPort.addDataListener(new SerialPortDataListener() {
            @Override
            public int getListeningEvents() {
                return SerialPort.LISTENING_EVENT_DATA_AVAILABLE;
            }

            @Override
            public void serialEvent(SerialPortEvent serialPortEvent) {
                SerialPort comPort = serialPortEvent.getSerialPort();
                /*byte[] cmd = new byte[1];
                comPort.readBytes(cmd, 1);*/
                byte[] serialData = new byte[comPort.bytesAvailable()];
                comPort.readBytes(serialData, serialData.length);
                for (byte buf : serialData)
                    serialBuf.add(buf);//放入缓冲区
                if (debug) System.out.println("<收到:" + BytesUtils.bytes2fStr(serialData));
                byte tryCount = 0;

                while (true) {
                    tryCount++;
                    try {

                        byte len = serialBuf.get(0);
                        if (serialBuf.size() < len + 2) return;

                        byte cmd = serialBuf.get(1);
                        byte[] data = new byte[len - 1];
                        for (int i = 0; i < len - 1; i++) {
                            data[i] = serialBuf.get(i + 2);
                        }

                        if (debug) System.out.println("len: " + len + " cmd: " + cmd);


                        if (debug) {//debug
                            byte[] az = new byte[serialBuf.size()];
                            for (int i = 0; i < serialBuf.size(); i++) {
                                az[i] = serialBuf.get(i);
                            }
                            System.out.println("buffer: " + serialBuf.size() + "  > " + BytesUtils.bytes2fStr(az));
                        }

                        if (serialBuf.get(len + 1) != (byte) 0xff) {
                            System.out.println("数据对齐异常");
                            serialBuf.clear();

                            return;
                        }

                        ByteBuffer bufData = ByteBuffer.allocate(len);
                        bufData.put(data);
                        bufData.flip();

                        serialHandler(CMD.values()[cmd], bufData, data);


                        if (len > -2) {
                            serialBuf.subList(0, len + 2).clear();
                        }


                        if (debug) {//debug
                            byte[] az = new byte[serialBuf.size()];
                            for (int i = 0; i < serialBuf.size(); i++) {
                                az[i] = serialBuf.get(i);
                            }
                            System.out.println("buffer: " + serialBuf.size() + "  > " + BytesUtils.bytes2fStr(az));
                        }

                        if (serialBuf.size() == 0) return;

                    } catch (Exception e) {
                        System.out.println("执行命令时发生意外 " + e.getMessage());
                        serialBuf.clear();
                        e.printStackTrace();
                        return;
                    }
                    if (tryCount > 4) {
                        System.out.println("数据异常");
                        serialBuf.clear();
                        return;
                    }
                }

            }
        });


    }

    /**
     * 关闭串口
     */
    private boolean closeSerialPort() {
        if (serialPort.isOpen())
            if (serialPort.closePort()) {
                isEnable = false;
            }
        return false;
    }

    /**
     * 串口数据处理
     *
     * @param cmd   命令
     * @param data  数据
     * @param bytes 原始字节
     */
    private void serialHandler(CMD cmd, ByteBuffer data, byte[] bytes) {

        switch (cmd) {
            case CMD_ERROR -> {
                System.out.println("发生异常");
            }
            case CMD_HAND_SHAKE -> {
                if (data.get() == 'K' && data.get() == 'B' && data.get() == 'N' && data.get() == 'L') {
                    if (data.get() != checkVersion) throw new RuntimeException("握手版本错误");
                    System.out.println("握手成功");
                    isEnable = true;
                    pingThread = new Thread(() -> {
                        while (isEnable) {
                            ping();
                            try {
                                Thread.sleep(500);
                            } catch (InterruptedException ignored) {

                            }
                        }
                        serialPort.clearBreak();
                        serialPort.clearDTR();
                        serialPort.clearRTS();
                        serialPort.closePort();
                        serialPort = null;
                    });
                    pingThread.start();

                    break;
                }
                throw new RuntimeException("握手发生错误");
            }
            case CMD_EVENT -> {
                if (nilouEventCallback != null)
                    nilouEventCallback.Callback(NilouEvent.values()[data.get()], bytes[1]);

            }
            case CMD_OK -> {
                if (debug) System.out.println("成功");
            }
            case CMD_DEBUG -> {
                System.out.println("调试:" + new String(data.array()));
            }
            case CMD_READ_E -> {
                if (bytes.length == readBytesLen) {
                    readBytesBuf = bytes.clone();
                    return;
                }
                throw new RuntimeException("读取失败");
            }
            case CMD_READ_C -> {
                readColorBuf = new Color(bytes[0] & 0xff, bytes[1] & 0xff, bytes[2] & 0xff);
            }
            case CMD_PING -> {
                pingB = true;
            }
        }


    }

    /**
     * 发送数据
     *
     * @param data 欲发送的数据
     */
    private void sendData(byte[] data) {
        if (debug) System.out.println("<发送:" + BytesUtils.bytes2fStr(data));
        serialPort.writeBytes(data, data.length);//发送握手数据
    }

    /**
     * 打开Nilou
     *
     * @param portName 串口名
     * @throws RuntimeException
     */
    public void openNilou(String portName) throws RuntimeException {
        isEnable = false;
        if (serialPort != null) closeSerialPort();
        setSerialPort(portName);
        openSerialPort();

        sendData(new byte[]{0x1, 'K', 'B', 'N', 'L'});

        //等待握手成功
        for (int i = 0; i < 2000; i++) {
            try {
                Thread.sleep(1);
            } catch (InterruptedException ignored) {
            }
            if (isEnable) return;
        }
        throw new RuntimeException("握手失败 (超时)");
    }

    public void closeNilou() {
        isEnable = false;
    }

    /**
     * 设置音量
     *
     * @param vol 范围:0-31
     */
    public void setVolume(int vol) {
        writeBytes(0, BytesUtils.int2byte(vol));
    }

    /**
     * 设置灯光模式
     *
     * @param mode 范围:0-3 LIGHT_MODE_
     */
    public void setLightMode(byte mode) {
        writeBytes(1, mode);
    }

    /**
     * 设置灯光颜色
     *
     * @param color 范围:0-11 LIGHT_COLOR_
     */
    public void setLightColor(byte color) {
        writeBytes(2, color);
    }

    /**
     * 设置灯光亮度
     *
     * @param brightness 范围:0-15
     */
    public void setLightBrightness(int brightness) {
        writeBytes(3, BytesUtils.int2byte(brightness));
    }

    /**
     * 设置灯光亮度
     *
     * @param mode 范围:0-2 SHANK_MODE_
     */
    public void setShakeMode(byte mode) {
        writeBytes(4, mode);
    }

    /**
     * 设置key音
     *
     * @param audioID 范围:0-32 "key0-key32"
     */
    public void setKeyAudio(int audioID) {
        writeBytes(5, BytesUtils.int2byte(audioID));
    }

    /**
     * 设置灯光过渡
     *
     * @param transition 默认:0.2f
     */
    public void setLightTransition(float transition) {
        writeBytes(16, BytesUtils.float2bytesA(transition));
    }

    /**
     * 设置按下灯光过渡
     *
     * @param transition 默认:0.25f
     */
    public void setKeyTSPress(float transition) {
        writeBytes(20, BytesUtils.float2bytesA(transition));
    }

    /**
     * 设置松开灯光过渡
     *
     * @param transition 默认:0.08f
     */
    public void setKeyTSRelease(float transition) {
        writeBytes(24, BytesUtils.float2bytesA(transition));
    }

    /**
     * 设置总敲击统计
     *
     * @param count 计数
     */
    public void setKeyPressAllCount(long count) {
        byte[] cmd = new byte[4];
        System.arraycopy(BytesUtils.long2bytesA(count), 0, cmd, 0, 4);
        writeBytes(32, cmd);
    }

    /**
     * 设置运行次数
     *
     * @param count 计数
     */
    public void setRunCount(long count) {
        byte[] cmd = new byte[4];
        System.arraycopy(BytesUtils.long2bytesA(count), 0, cmd, 0, 4);
        writeBytes(36, cmd);
    }

    /**
     * 设置运行时间
     *
     * @param s 秒 精确到10s
     */
    public void setRunTimes(long s) {
        byte[] cmd = new byte[4];
        System.arraycopy(BytesUtils.long2bytesA(s / 10L), 0, cmd, 0, 4);
        writeBytes(40, cmd);
    }

    /**
     * 设置Key敲击统计
     *
     * @param index 索引 0-6
     * @param count 次数
     */
    public void setKeyPressCount(int index, long count) {
        byte[] cmd = new byte[4];
        System.arraycopy(BytesUtils.long2bytesA(count), 0, cmd, 0, 4);
        writeBytes((48 + index * 4), cmd);
    }


    /**
     * 设置Key映射键
     *
     * @param index   索引 0-6
     * @param Mapping 映射键
     */
    public void setKeyMapping(int index, char Mapping) {
        writeBytes((80 + index), (byte) Mapping);
    }


    /**
     * 设置Key按下颜色
     *
     * @param index 索引 0-6
     * @param color 按下颜色
     */
    public void setKeyPressColor(int index, Color color) {
        writeBytes((112 + index * 3), new byte[]{
                (byte) color.getRI(),
                (byte) color.getGI(),
                (byte) color.getBI()});
    }

    /**
     * 设置Key默认颜色
     *
     * @param color 按下颜色
     */
    public void setKeyReleaseColor(Color color) {
        writeBytes((0x90), new byte[]{
                (byte) color.getRI(),
                (byte) color.getGI(),
                (byte) color.getBI()});
    }

    /**
     * 获取音量
     *
     * @return 音量大小 0-31
     */
    public int getVolume() {
        return readBytes(0, 1)[0];
    }

    /**
     * 获取灯光模式
     *
     * @return 灯光模式 0-3 LIGHT_MODE_
     */
    public byte getLightMode() {
        return readBytes(1, 1)[0];
    }

    /**
     * 获取灯光颜色
     *
     * @return 灯光颜色 0-11 LIGHT_COLOR_
     */
    public byte getLightColor() {
        return readBytes(2, 1)[0];
    }

    /**
     * 获取灯光亮度
     *
     * @return 亮度 0-15
     */
    public int getLightBrightness() {
        return readBytes(3, (byte) 1)[0];
    }

    /**
     * 获取震动强度
     *
     * @return 强度 0-2 SHANK_MODE_
     */
    public byte getShakeMode() {
        return readBytes(4, (byte) 1)[0];
    }

    /**
     * 获取Key音
     *
     * @return Key音 0-32
     */
    public int getKeyAudio() {
        return readBytes(5, (byte) 1)[0];
    }


    /**
     * 获取灯光过渡
     *
     * @return 灯光过渡 默认0.2f
     */
    public float getLightTransition() {
        return BytesUtils.bytes2float(readBytes(16, 4));
    }

    /**
     * 获取按下灯光过渡
     *
     * @return 灯光过渡 默认0.25f
     */
    public float getKeyTSPress() {
        return BytesUtils.bytes2float(readBytes(20, 4));
    }

    /**
     * 获取松开灯光过渡
     *
     * @return 灯光过渡 默认0.08f
     */
    public float getKeyTSRelease() {
        return BytesUtils.bytes2float(readBytes(24, 4));
    }

    /**
     * 获取总敲击统计
     *
     * @return 总敲击统计
     */
    public long getKeyPressAllCount() {
        return BytesUtils.bytes2long(BytesUtils.bytesSplicing(readBytes(32, 4),
                new byte[]{0x0, 0x0, 0x0, 0x0}));
    }

    /**
     * 获取运行次数
     *
     * @return 运行次数
     */
    public long getRunCount() {
        return BytesUtils.bytes2long(BytesUtils.bytesSplicing(readBytes(36, 4),
                new byte[]{0x0, 0x0, 0x0, 0x0}));
    }

    /**
     * 获取运行时间
     *
     * @return 运行时间 s
     */
    public long getRunTimes() {
        return 10L * BytesUtils.bytes2long(BytesUtils.bytesSplicing(readBytes(40, 4),
                new byte[]{0x0, 0x0, 0x0, 0x0}));
    }


    /**
     * 获取Key敲击统计
     *
     * @param index 索引 0-6
     * @return 次数
     */
    public long getKeyPressCount(int index) {
        return BytesUtils.bytes2long(BytesUtils.bytesSplicing(readBytes((48 + index * 4), 4),
                new byte[]{0x0, 0x0, 0x0, 0x0}));
    }


    /**
     * 获取Key映射键
     *
     * @param index 索引 0-6
     * @return 映射键
     */
    public char getKeyMapping(int index) {
        return (char) readBytes((80 + index), 1)[0];
    }


    /**
     * 获取Key按下颜色
     *
     * @param index 索引 0-6
     * @return 按下颜色
     */
    public Color getKeyPressColor(int index, Color color) {
        byte[] bytes = readBytes((112 + index * 3), 3);
        return new Color(bytes[0] & 0xff, bytes[1] & 0xff, bytes[2] & 0xff);
    }

    /**
     * 获取Key默认颜色
     *
     * @return 按下颜色
     */
    public Color getKeyReleaseColor() {
        byte[] bytes = readBytes(144, 3);
        return new Color(bytes[0] & 0xff, bytes[1] & 0xff, bytes[2] & 0xff);
    }

    public void useDefaultInit() {
        setVolume(25);
        setLightMode(LIGHT_MODE_INDEPENDENT);
        setLightColor(LIGHT_COLOR_CUSTOM);
        setLightBrightness(15);
        setShakeMode(SHANK_MODE_LIGHT);
        setKeyAudio(1);
        setLightTransition(0.2f);
        setKeyTSPress(0.25f);
        setKeyTSRelease(0.08f);
        setKeyPressAllCount(0);
        setRunCount(0);
        setRunTimes(0);
        for (int i = 0; i < 7; i++) {
            setKeyPressCount(i, 0);
            setKeyPressColor(i, new Color(255, 0, 0));
        }
        setKeyReleaseColor(new Color(0, 50, 0));
        setKeyMapping(0, 'a');
        setKeyMapping(1, 's');
        setKeyMapping(2, 'd');
        setKeyMapping(3, 'j');
        setKeyMapping(4, 'k');
        setKeyMapping(5, 'l');
        setKeyMapping(6, ' ');
        flush();
    }

    /**
     * 无灯光
     */
    public static final byte LIGHT_MODE_NOT = 0x0;
    /**
     * 常亮
     */
    public static final byte LIGHT_MODE_ALWAYS = 0x1;
    /**
     * 独立 亮
     */
    public static final byte LIGHT_MODE_INDEPENDENT = 0x2;
    /**
     * 独立 闪烁 亮
     */
    public static final byte LIGHT_MODE_INDEPENDENT_TWINKLE = 0x3;
    /**
     * 自定义
     */
    public static final byte LIGHT_COLOR_CUSTOM = 0x0;
    /**
     * 七彩渐变
     */
    public static final byte LIGHT_COLOR_RAINBOW = 0x1;
    /**
     * 七彩渐变流水
     */
    public static final byte LIGHT_COLOR_RAINBOW_WATER = 0x2;
    /**
     * 随机
     */
    public static final byte LIGHT_COLOR_RANDOM = 0x3;
    /**
     * 纯红
     */
    public static final byte LIGHT_COLOR_RED = 0x4;
    /**
     * 纯橙
     */
    public static final byte LIGHT_COLOR_ORANGE = 0x5;
    /**
     * 纯黄
     */
    public static final byte LIGHT_COLOR_YELLOW = 0x6;
    /**
     * 纯绿
     */
    public static final byte LIGHT_COLOR_GREEN = 0x7;
    /**
     * 纯蓝
     */
    public static final byte LIGHT_COLOR_BLUE = 0x8;
    /**
     * 纯粉
     */
    public static final byte LIGHT_COLOR_PINK = 0x9;
    /**
     * 纯紫
     */
    public static final byte LIGHT_COLOR_PURPLE = 0xA;
    /**
     * 纯白
     */
    public static final byte LIGHT_COLOR_WHITE = 0xB;
    /**
     * 无震动
     */
    public static final byte SHANK_MODE_NOT = 0x0;
    /**
     * 轻震动
     */
    public static final byte SHANK_MODE_LIGHT = 0x1;
    /**
     * 重震动
     */
    public static final byte SHANK_MODE_HEAVY = 0x2;

    /**
     * Nilou 事件类
     */
    public enum NilouEvent {
        NE_KEY_PRESS,
        NE_KEY_RELEASE,
        NE_RE_ROTARY_CLOCKWISE,
        NE_RE_ROTARY_COUNTERCLOCKWISE,
        NE_RE_PRESS,
        NE_RE_LONG_PRESS
    }

    /**
     * Nilou事件回调
     */
    public interface NilouEventCallback {
        void Callback(NilouEvent nilouEvent, int index);
    }

    private enum CMD {
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
    }


}
