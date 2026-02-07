#include <jni.h>
#include "jsi/jsi.h"
#include "react-native-turbo-base64.h"


extern "C"
JNIEXPORT void JNICALL
Java_com_turbobase64_TurboBase64Module_nativeInstall (JNIEnv* /*env*/, jobject /*thiz*/, jlong runtimePtr) {
  auto* rt = reinterpret_cast<facebook::jsi::Runtime*>(runtimePtr);
  if (!rt) return;
  rntb_base64::install(rt);
}
