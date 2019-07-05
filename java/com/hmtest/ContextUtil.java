package com.pvdnc.hmtest;

import android.app.Application;
import android.content.Context;

import java.lang.reflect.Method;

public class ContextUtil {
    public static Context getAppContext(){
        try {
            Class<?> activityThread=Class.forName("android.app.ActivityThread");
            Method currentApplication=activityThread.getDeclaredMethod("currentApplication");

            Application app=(Application) currentApplication.invoke(null);
            return app.getApplicationContext();
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
}
