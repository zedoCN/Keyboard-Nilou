module com.zedo.kbnilou {
    requires javafx.controls;
    requires javafx.fxml;
    requires com.zedo.nilouSDK;
    opens com.zedo.kbnilou to javafx.fxml;
    exports com.zedo.kbnilou;
}