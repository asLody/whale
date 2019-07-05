package com.lody.whale;

import com.lody.whale.wrapper.LogWrapper;
import com.lody.whale.xposed.XC_MethodHook;
import com.lody.whale.xposed.XposedBridge;

import java.lang.reflect.Method;
import java.util.List;

public class CrossDex extends XC_MethodHook {
    private static final String TAG="CrossDex";
    public static void hook(){
        try {
            Class<?> inClass=Class.forName("android.app.Instrumentation");
            Class<?> appClass=Class.forName("android.app.Application");
            Method callAppOnCreate= inClass.getDeclaredMethod("callAppOnCreate", appClass);
            XposedBridge.hookMethod(callAppOnCreate,new CrossDex());
            LogWrapper.log(LogWrapper.Level.INFO,TAG,"hook callAppOnCreate finished");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    @Override
    protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
        LogWrapper.log(LogWrapper.Level.INFO,TAG,"invoke before callAppOnCreate");
        WhaleRuntime.handleCallAppOnCreate();//Give the control to JNI.
    }
}
