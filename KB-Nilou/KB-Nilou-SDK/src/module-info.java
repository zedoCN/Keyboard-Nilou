module com.zedo.nilouSDK {
    requires com.fazecast.jSerialComm;
    opens com.zedo.nilouSDK to com.fazecast.jSerialComm;
    exports com.zedo.nilouSDK;
}

