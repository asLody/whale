package com.yourpackagename.hmtest;

import android.app.Application;
import android.content.Context;

import java.lang.reflect.Method;

public class ContextUtil {
    public static Context getAppContext(){
        try {
            //It can be ensured that when module calls this,hidden API policy has been disabled.
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
