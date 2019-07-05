package com.pvdnc.hmtest;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.util.Log;
import android.widget.TextView;

import com.lody.whale.enity.HookModule;
import com.lody.whale.xposed.ClassUtils;
import com.lody.whale.xposed.XC_MethodHook;
import com.lody.whale.xposed.XposedBridge;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.concurrent.atomic.AtomicBoolean;

@HookModule(name = "QQTest",targetPkgName = "com.tencent.qqlite")
public class QQTest {
    private static final String TAG = "QQTest";

    /**
     * All hook modules 'must' have this method with the same name
     *  in order to let World know its target.
     * @return return to JNI with our target's package name.
     */
    public static String targetPkgName(){
        return "com.tencent.qqlite";//Only to avoid AssertException.
    }

    public static void hook() {
        new Thread(){
            @Override
            public void run() {
                try {
                    Thread.sleep(5000);//5s will be enough for QQ to finish its load.
                    Log.d(TAG,"hook thread has finish its wait status");
                    Class<?> codeWrapper=ContextUtil.getAppContext().getClassLoader().
                            loadClass("com.tencent.qphone.base.util.CodecWarpper");
                    Method encodeRequest= ClassUtils.findMethodByName(codeWrapper,"encodeRequest").get(0);
                    Log.d(TAG,"found codecWrapper class:"+codeWrapper.getName());
                    XposedBridge.hookMethod(encodeRequest,new CodeWrapper_Request());
                    Log.d(TAG,"hook encodeRequest finished");
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }.start();
        Log.d(TAG,"hook thread has been started");
    }

    @Deprecated
    private static void doHookAsync(Class<?> codecWrapper)throws Exception{

        Field soLoadedField= codecWrapper.getDeclaredField("isSoLoaded");
        soLoadedField.setAccessible(true);
        while (true){
            AtomicBoolean isSoLoaded=(AtomicBoolean)soLoadedField.get(null);
            if(isSoLoaded.get()){
                Log.d(TAG,"");
                break;
            }
        }
    }

    public static class CodeWrapper_Request extends XC_MethodHook {
        @Override
        protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
            StringBuilder sb=new StringBuilder();
            sb.append("invoke before encodeRequest").append("\n");
            sb.append("---  params  ---").append("\n");
            for (int i=0;i<param.args.length;i++){
                sb.append(i+1).append(".");
                Class<?> argItemClass=param.args[i].getClass();
                sb.append("(").append(argItemClass.getName()).append(")");
                if(argItemClass.getName().contains(byte[].class.getName())){//byte array.
                    byte[] data=(byte[])param.args[i];
                    String str=new String(data,"gb2312");
                    sb.append(str);//Output its string instead of address.
                    //arg[5] is action(MessageSvc.PbGetMsg is receive)
                    //arg[3](4) is QQ version.
                    //arg[9](10) is current QQ number
                    //arg[12](13) is MessageSvc.PbSendMsg's part content.
                }else {
                    sb.append(param.args[i]);
                }
                sb.append("\n");
            }
            sb.append("---      ---");
            Log.d(TAG,sb.toString());
        }
    }
}
