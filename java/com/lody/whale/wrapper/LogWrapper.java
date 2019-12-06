package com.lody.whale.wrapper;

import java.lang.reflect.Method;

public class LogWrapper {
    private static Class LOG_CLASS;
    private static Method I_METHOD;
    private static Method E_METHOD;

    public enum Level {
        ERROR, INFO
    }

    static {
        try {
            LOG_CLASS = Class.forName("android.util.Log");
            E_METHOD = LOG_CLASS.getDeclaredMethod("e", String.class, String.class);
            E_METHOD.setAccessible(true);
            I_METHOD = LOG_CLASS.getDeclaredMethod("i", String.class, String.class);
            I_METHOD.setAccessible(true);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void log(Level level, String TAG, String text) {
        try {
            switch (level) {
                case ERROR: {//error
                    E_METHOD.invoke(null, TAG, text);
                    break;
                }
                case INFO: {//info
                    I_METHOD.invoke(null, TAG, text);
                    break;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void e(String TAG, Throwable t) {
        try {
            Method getStackTraceString = LOG_CLASS.getDeclaredMethod("getStackTraceString", Throwable.class);
            getStackTraceString.setAccessible(true);
            String stackTraceStr = (String) getStackTraceString.invoke(null, t);
            log(Level.ERROR, TAG, stackTraceStr);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
