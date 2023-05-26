package com.zedo.kbnilou;

import com.zedo.nilouSDK.Nilou;
import com.zedo.nilouSDK.Utils;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.geometry.Pos;
import javafx.scene.control.ChoiceBox;
import javafx.scene.control.ComboBox;
import javafx.scene.control.Label;
import javafx.scene.effect.BlurType;
import javafx.scene.effect.DropShadow;
import javafx.scene.effect.Glow;
import javafx.scene.layout.Background;
import javafx.scene.layout.BackgroundFill;
import javafx.scene.layout.HBox;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;


import java.net.URL;
import java.util.*;

public class Controller implements Initializable {

    Thread loop;

    Label[] keys = new Label[6];

    Nilou nilou = new Nilou();
    @FXML
    public HBox hBox;
    @FXML
    public ChoiceBox com;
    @FXML
    public Label state;

    @Override
    public void initialize(URL url, ResourceBundle resourceBundle) {
        for (int i = 0; i < keys.length; i++) {
            keys[i] = new Label("?");
            keys[i].setFont(new Font(24));
            keys[i].setPrefSize(60, 60);
            keys[i].setTextFill(Color.WHITE);
            keys[i].setEffect(new Glow(0.44));
            keys[i].setBackground(new Background(new BackgroundFill(Color.web("#21489c"), null, null)));
            keys[i].setAlignment(Pos.CENTER);
            keys[i].setEffect(new DropShadow(BlurType.TWO_PASS_BOX, Color.WHITE, 5, 1, 0, 0));
            hBox.getChildren().add(keys[i]);
        }
        for (String s : Utils.getSerialPortNameList())
            com.getItems().add(s);
    }

    @FXML
    public void connect() {
        try {
            state.setText("正在连接  " + com.getValue());
            nilou.openNilou((String) com.getValue());
        } catch (RuntimeException runtimeException) {
            state.setText("连接失败 " + runtimeException.getMessage());

        }
        state.setText("连接成功  " + com.getValue());
        //nilou.useDefaultInit();
        for (int i = 0; i < keys.length; i++) {
            keys[i].setText("" + nilou.getKeyMapping(i));
        }
        loop = new Thread(() -> {
            while (nilou.isOpened()) {
                for (int i = 0; i < keys.length; i++) {
                    com.zedo.nilouSDK.Color color = nilou.readColor(i);
                    keys[i].setBackground(new Background(new BackgroundFill(
                            Color.rgb(color.getRI(), color.getGI(), color.getBI())
                            , null, null)));

                }
                try {
                    Thread.sleep(30);
                } catch (InterruptedException ignored) {

                }
            }
        });
        loop.start();
        nilou.addEventListener((nilouEvent, i) -> {

            if (nilouEvent.equals(Nilou.NilouEvent.NE_KEY_PRESS))
                keys[i].setEffect(new DropShadow(BlurType.TWO_PASS_BOX, Color.web("#21489c"), 5, 1, 0, 0));

            if (nilouEvent.equals(Nilou.NilouEvent.NE_KEY_RELEASE))
                keys[i].setEffect(new DropShadow(BlurType.TWO_PASS_BOX, Color.WHITE, 5, 1, 0, 0));

        });
    }

    @FXML
    public void disconnect() {
        state.setText("关闭  " + com.getValue());
        nilou.closeNilou();
    }
}