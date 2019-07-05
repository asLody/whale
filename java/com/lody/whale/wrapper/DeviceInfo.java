package com.lody.whale.wrapper;


import java.lang.reflect.Method;
import java.text.SimpleDateFormat;
import java.util.Date;

public class DeviceInfo {
    public String date;
    public String time;
    public static DeviceInfo getNow(){
        SimpleDateFormat ft = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss");
        String nowStr= ft.format(new Date());
        DeviceInfo now=new DeviceInfo();
        now.date=nowStr.split(" ")[0];
        now.time=nowStr.split(" ")[1];
        return now;
    }

    public static String getSystemProp(String name){
        try {
            Class sysPropClass=Class.forName("andro<id.os.SystemProperties");
            Method getMethod=sysPropClass.getDeclaredMethod("get",String.class,String.class);
            getMethod.setAccessible(true);
            return (String) getMethod.invoke(null,name,FAIL_STR);
        } catch (Exception e) {
            e.printStackTrace();
            return FAIL_STR;
        }
    }

    public static final String FAIL_STR ="unknown";
    public static String getDeviceID() {
        return getSystemProp("ro.build.id");
    }

    public static final int FAIL_INT=0;
    public static int getSDKInt(){
        String sdk=getSystemProp("ro.build.version.sdk");
        if(sdk.equals(FAIL_STR))return FAIL_INT;
        return Integer.parseInt(sdk);
    }
}
