package com.turbobase64

import android.util.Log
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.bridge.ReactContextBaseJavaModule
import com.facebook.react.bridge.ReactMethod
import com.facebook.react.common.annotations.FrameworkAPI

class TurboBase64Module(private val reactContext: ReactApplicationContext) :
  ReactContextBaseJavaModule(reactContext) {

    override fun getName(): String {
        return NAME
    }

    companion object {
        const val NAME = "RNTurboBase64"
        init {
            System.loadLibrary("react-native-turbo-base64")
        }

        @OptIn(FrameworkAPI::class)
        @JvmStatic
        private external fun nativeInstall(jsiRuntimePtr: Long)
    }

    @OptIn(FrameworkAPI::class)
    @ReactMethod(isBlockingSynchronousMethod = true)
    fun install(): Boolean {
        Log.d("react-native-turbo-base64", "install() called")
        nativeInstall(
            reactContext.javaScriptContextHolder!!.get(),
        )
        return true
    }


}
