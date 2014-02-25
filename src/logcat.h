#include <android/log.h>

/* Define the tag that our log will be labeled with
 */
#define CLICKTRACK_LOG_TAG "libclicktrack_native"

/* Define macros for the different logging levels
 */
#define logv(...) __android_log_print(ANDROID_LOG_VERBOSE, CLICKTRACK_LOG_TAG, __VA_ARGS__);
#define logd(...) __android_log_print(ANDROID_LOG_DEBUG, CLICKTRACK_LOG_TAG, __VA_ARGS__);
#define logi(...) __android_log_print(ANDROID_LOG_INFO, CLICKTRACK_LOG_TAG, __VA_ARGS__);
#define logw(...) __android_log_print(ANDROID_LOG_WARN, CLICKTRACK_LOG_TAG, __VA_ARGS__);
#define loge(...) __android_log_print(ANDROID_LOG_ERROR, CLICKTRACK_LOG_TAG, __VA_ARGS__);
