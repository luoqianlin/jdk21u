/*
 * Experimental AI-assisted implementation for this personal fork.
 * Distributed under GPLv2 with the Classpath exception; see LICENSE and ASSEMBLY_EXCEPTION.
 * Not submitted to the OpenJDK Community.
 */

#ifndef _GIO_INTERFACE_H
#define _GIO_INTERFACE_H

#include <jni.h>

jboolean gio_load(void);
jint gio_supported_actions(void);
jboolean gio_show_uri(JNIEnv *env, const char *uri);

#endif /* !_GIO_INTERFACE_H */
