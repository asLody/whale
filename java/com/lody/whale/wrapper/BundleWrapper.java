package com.lody.whale.wrapper;

import java.io.Serializable;
import java.lang.reflect.Method;

public class BundleWrapper {
    private static Class BUNDLE_CLASS;
    static{
        try {
            BUNDLE_CLASS=Class.forName("android.os.Bundle");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    public static Object newBundle(){
        try {
            return BUNDLE_CLASS.newInstance();
        } catch (Exception e){
            e.printStackTrace();
        }
        return null;
    }

    public static Object getSerializable(Object bundle, String key){
        if(!BUNDLE_CLASS.isInstance(bundle))return null;
        try {
            Method getMethod=BUNDLE_CLASS.getDeclaredMethod("getSerializable",String.class);
            getMethod.setAccessible(true);
            return getMethod.invoke(bundle,key);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    public static void putSerializable(Object bundle,String key,Serializable value){
        if(!BUNDLE_CLASS.isInstance(bundle))return;
        try {
            Method putMethod=BUNDLE_CLASS.getDeclaredMethod("putSerializable",String.class,Serializable.class);
            putMethod.setAccessible(true);
            putMethod.invoke(bundle,key,value);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
