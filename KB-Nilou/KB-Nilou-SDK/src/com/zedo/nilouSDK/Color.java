package com.zedo.nilouSDK;

public class Color {
    float R, G, B;


    public Color(float r, float g, float b) {
        R = r;
        G = g;
        B = b;
    }

    public Color(int r, int g, int b) {
        R = r / 255f;
        G = g / 255f;
        B = b / 255f;
    }

    public float getRF() {
        return R;
    }

    public float getGF() {
        return G;
    }

    public float getBF() {
        return B;
    }

    public int getRI() {
        return (int) (R * 255);
    }

    public int getGI() {
        return (int) (G * 255);
    }

    public int getBI() {
        return (int) (B * 255);
    }

    public Color clone() {
        return new Color(R, G, B);
    }

}
