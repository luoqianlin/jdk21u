/*
 * Copyright (c) 2005, 2020, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

#ifdef HEADLESS
    #error This file should not be included in headless library
#endif

#include "jni_util.h"
#include "gio_interface.h"

/*
 * Class:     sun_awt_X11_XDesktopPeer
 * Method:    init
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_sun_awt_X11_XDesktopPeer_init
  (JNIEnv *env, jclass cls)
{
    return gio_load() ? gio_supported_actions() : 0;
}

/*
 * Class:     sun_awt_X11_XDesktopPeer
 * Method:    openURI
 * Signature: ([B)Z
 */
JNIEXPORT jboolean JNICALL Java_sun_awt_X11_XDesktopPeer_openURI
  (JNIEnv *env, jobject obj, jbyteArray url_j)
{
    jboolean success;
    const char* url_c;

    url_c = (char*)(*env)->GetByteArrayElements(env, url_j, NULL);
    if (url_c == NULL) {
        if (!(*env)->ExceptionCheck(env)) {
            JNU_ThrowOutOfMemoryError(env, 0);
        }
        return JNI_FALSE;
    }

    success = gio_show_uri(env, url_c);

    (*env)->ReleaseByteArrayElements(env, url_j, (signed char*)url_c, JNI_ABORT);

    return success;
}
