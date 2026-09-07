/*
 * Experimental AI-assisted regression test for this personal fork.
 * Distributed under GPLv2; see LICENSE. Not submitted to OpenJDK.
 */

#include <jni.h>
#if defined(__linux__)
#include <dlfcn.h>

static void *handle;
static void (*leave)(void);

JNIEXPORT jboolean JNICALL Java_DesktopGdkLockTest_acquireGdkLock
  (JNIEnv *env, jclass cls) {
    void (*threads_init)(void);
    void (*enter)(void);
    int (*init_check)(int *, char ***);

    handle = dlopen("libgtk-3.so.0", RTLD_LAZY | RTLD_GLOBAL);
    if (handle == NULL) return JNI_FALSE;
    threads_init = dlsym(handle, "gdk_threads_init");
    enter = dlsym(handle, "gdk_threads_enter");
    leave = dlsym(handle, "gdk_threads_leave");
    init_check = dlsym(handle, "gtk_init_check");
    if (!threads_init || !enter || !leave || !init_check) return JNI_FALSE;
    threads_init();
    if (!init_check(NULL, NULL)) return JNI_FALSE;
    enter();
    return JNI_TRUE;
}

JNIEXPORT void JNICALL Java_DesktopGdkLockTest_releaseGdkLock
  (JNIEnv *env, jclass cls) {
    leave();
}
#endif
