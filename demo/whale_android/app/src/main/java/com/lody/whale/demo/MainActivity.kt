package com.lody.whale.demo

import android.annotation.SuppressLint
import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.lody.whale.xposed.XC_MethodHook
import com.lody.whale.xposed.XposedBridge
import com.lody.whale_android.R
import java.lang.RuntimeException

/**
 * @Author: xiongyiming
 * @Time: 2019-09-03 21:34
 * @Description:
 */
class MainActivity: AppCompatActivity(){
    companion object{
        private const val TAG = "MainActivity"
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_main)

        hookMethod()

        findViewById<View>(R.id.btn_test).setOnClickListener {
            Toast.makeText(this, "hello", Toast.LENGTH_SHORT).show()
        }

    }

    private fun hookMethod(){
        val method = Toast::class.java.getDeclaredMethod("show")

        XposedBridge.hookMethod(method, object : XC_MethodHook(){
            @SuppressLint("ShowToast")
            override fun beforeHookedMethod(param: MethodHookParam) {
                val e = RuntimeException()
                Log.e(TAG, "replaceHookedMethod", e)
                val toast = Toast.makeText(this@MainActivity, "hooked message", Toast.LENGTH_SHORT)
                val result = XposedBridge.invokeOriginalMethod(method, toast, emptyArray<Any>())
                param.result = result
            }
        })
    }
}