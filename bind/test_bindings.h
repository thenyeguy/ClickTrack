#include <jni.h>
#include "../src/io_elements.h"

extern "C"
{
    JNIEXPORT void JNICALL 
        Java_edu_cmu_ece_ece551_clicktrack_TestClickTrack_initOpenSlesTest(
                JNIEnv* env, jobject obj);
}
