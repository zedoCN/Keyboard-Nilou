package com.zedo.nilouSDK;

public class Main {
    public static void main(String[] args) throws InterruptedException {
        Nilou nilou = new Nilou();


        nilou.openNilou("COM3");
        nilou.useDefaultInit();
        nilou.addEventListener((nilouEvent, index) -> {
            System.out.println(nilouEvent + " " + index);
        });
        nilou.closeNilou();
        //nilou.SetKeyPressAllCount(0);
        /*System.out.println(BytesUtils.bytes2fStr(nilou.readBytes((byte) 16, (byte) 4)));
        nilou.writeBytes((byte) 16,BytesUtils.float2bytesA(0.2f));
        System.out.println(BytesUtils.bytes2fStr(nilou.readBytes((byte) 16, (byte) 4)));
        System.out.println(BytesUtils.bytes2float(nilou.readBytes((byte) 16, (byte) 4)));*/
        //nilou.flush();
        //nilou.useDefaultInit();
        //nilou.setKeyReleaseColor(new Color(10,20,30));
        //nilou.flush();
        /*for (int i = 0; i < 16; i++) {
            //nilou.writeBytes(i * 16,new byte[16]);
            System.out.println(BytesUtils.bytes2fStr(nilou.readBytes(i * 16, 16)));
        }*/

        /*while (true) {
            //nilou.writeColor((byte) 0,new Color(new Random().nextInt(255*255*255)));
            System.out.println(nilou.getRunCount());
            nilou.ping();
            Thread.sleep(400);
        }*/
        //Nilou.writeBytes((byte) 0,new byte[]{20});
        //Nilou.flush();
        //System.out.println(Nilou.readBytes((byte) 0,new byte[]{2}););
        /*System.out.println(Nilou.readColor((byte) 0));
        Nilou.writeColor((byte) 0,new Color(255,0,0));
        Nilou.writeColor((byte) 1,new Color(0,255,0));
        Nilou.playAudio("Wel");
        */
        //Nilou.writeBytes((byte) 10, new byte[]{(byte) 0xff, (byte) 0xff});
        //System.out.println("输出:"+BytesUtils.bytes2fStr(Nilou.readBytes((byte) 10, (byte) 6)));
        //Nilou.getStatistics();
    }
}
