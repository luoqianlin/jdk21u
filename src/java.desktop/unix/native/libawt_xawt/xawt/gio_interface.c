/*
 * Experimental AI-assisted implementation for this personal fork.
 * Distributed under GPLv2 with the Classpath exception; see LICENSE and ASSEMBLY_EXCEPTION.
 * Not submitted to the OpenJDK Community.
 */

#include <dlfcn.h>
#include <string.h>
#include "jni_util.h"
#include "jvm_md.h"
#include "gio_interface.h"

/* GLib's public GError ABI, without introducing build-time GTK/GIO dependencies. */
typedef struct {
    unsigned int domain;
    int code;
    char *message;
} GioError;

typedef int (*LaunchUri)(const char *, void *, GioError **);
typedef void (*FreeError)(GioError *);
typedef void *(*DefaultVfs)(void);
typedef const char *const *(*VfsSchemes)(void *);

static void *gio_handle;
static LaunchUri launch_uri;
static FreeError free_error;
static DefaultVfs default_vfs;
static VfsSchemes vfs_schemes;

/* Callers serialize initialization and use through XToolkit.awtLock. */
jboolean gio_load(void) {
    void *handle;
    LaunchUri launch;
    FreeError release;
    DefaultVfs get_vfs;
    VfsSchemes get_schemes;

    if (gio_handle != NULL) {
        return JNI_TRUE;
    }

    handle = dlopen(VERSIONED_JNI_LIB_NAME("gio-2.0", "0"), RTLD_LAZY | RTLD_LOCAL);
    if (handle == NULL) {
        return JNI_FALSE;
    }

    launch = (LaunchUri)dlsym(handle, "g_app_info_launch_default_for_uri");
    release = (FreeError)dlsym(handle, "g_error_free");
    get_vfs = (DefaultVfs)dlsym(handle, "g_vfs_get_default");
    get_schemes = (VfsSchemes)dlsym(handle, "g_vfs_get_supported_uri_schemes");
    if (launch == NULL || release == NULL || get_vfs == NULL || get_schemes == NULL) {
        dlclose(handle);
        return JNI_FALSE;
    }

    launch_uri = launch;
    free_error = release;
    default_vfs = get_vfs;
    vfs_schemes = get_schemes;
    gio_handle = handle;
    return JNI_TRUE;
}

jint gio_supported_actions(void) {
    jint actions = 1; /* OPEN */
    void *vfs = default_vfs();
    const char *const *schemes = vfs == NULL ? NULL : vfs_schemes(vfs);

    /* Preserve the existing GTK3 backend's OPEN/MAIL/BROWSE capability policy. */
    if (schemes != NULL) {
        for (int i = 0; schemes[i] != NULL; i++) {
            if (strcmp(schemes[i], "http") == 0) {
                actions |= 2 | 4; /* MAIL | BROWSE */
                break;
            }
        }
    }
    return actions;
}

jboolean gio_show_uri(JNIEnv *env, const char *uri) {
    GioError *error = NULL;
    int success;

    if (gio_handle == NULL) {
        JNU_ThrowIOException(env, "Failed to load GIO native library.");
        return JNI_FALSE;
    }

    /* A GdkAppLaunchContext would reintroduce the GDK lock and event-loop dependency. */
    success = launch_uri(uri, NULL, &error);
    if (error != NULL) {
        if (!success) {
            JNU_ThrowIOException(env, error->message);
        }
        free_error(error);
    }
    return success ? JNI_TRUE : JNI_FALSE;
}
