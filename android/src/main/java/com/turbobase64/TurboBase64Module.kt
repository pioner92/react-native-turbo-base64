package com.turbobase64

import com.facebook.react.bridge.ReactApplicationContext

class TurboBase64Module(reactContext: ReactApplicationContext) :
  NativeTurboBase64Spec(reactContext) {

  override fun multiply(a: Double, b: Double): Double {
    return a * b
  }

  companion object {
    const val NAME = NativeTurboBase64Spec.NAME
  }
}
