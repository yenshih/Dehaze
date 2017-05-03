// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from dehaze.djinni

#include "NativeDehaze.hpp"  // my header
#include "Marshal.hpp"

namespace djinni_generated {

NativeDehaze::NativeDehaze() : ::djinni::JniInterface<::dehaze::Dehaze, NativeDehaze>("com/dehaze/Dehaze$CppProxy") {}

NativeDehaze::~NativeDehaze() = default;


CJNIEXPORT void JNICALL Java_com_dehaze_Dehaze_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<::djinni::CppProxyHandle<::dehaze::Dehaze>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_com_dehaze_Dehaze_create(JNIEnv* jniEnv, jobject /*this*/)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::dehaze::Dehaze::create();
        return ::djinni::release(::djinni_generated::NativeDehaze::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_com_dehaze_Dehaze_00024CppProxy_native_1dehaze(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jstring j_uri, jstring j_media)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::dehaze::Dehaze>(nativeRef);
        auto r = ref->dehaze(::djinni::String::toCpp(jniEnv, j_uri),
                             ::djinni::String::toCpp(jniEnv, j_media));
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

}  // namespace djinni_generated
