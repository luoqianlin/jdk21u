# Experimental Desktop GIO backend

This is an AI-assisted experiment in a personal fork, not an OpenJDK contribution,
approved patch, or supported JDK distribution. Do not submit these generated files
or their text to OpenJDK. The OpenJDK Interim AI Policy permits private research
but prohibits AI-generated contributions to its repositories and discussions.

## Problem and implementation

A JavaFX GTK callback can already own the non-recursive GDK global lock when it
calls `Desktop.open`. The existing AWT native backend calls `gdk_threads_enter`
again before `gtk_show_uri`, deadlocking the calling thread. This experiment
changes the native backend, not the application or its calling thread.

The backend loads `libgio-2.0.so.0` with `dlopen` and calls
`g_app_info_launch_default_for_uri(uri, NULL, &error)`. It never initializes GTK
or acquires a GDK lock. Missing required symbols disable the backend; there is no
fallback into the old GTK/GNOME path. Successful loading retains the library for
the process lifetime; incomplete loading closes the handle. GIO errors are freed
and failed launches throw `IOException`. URI bytes use UTF-8.

The Java public API is unchanged. The existing GTK3 capability policy is retained:
OPEN is available when the backend loads; MAIL and BROWSE are enabled when the
default VFS advertises HTTP support. EDIT and PRINT remain unsupported. The
existing AWT lock continues to serialize initialization and launch, but the launch
no longer enters GDK. No GDK mutex is replaced and no thread-name heuristic is used.

Using a NULL launch context drops GDK-specific startup notification and screen
selection metadata. Application activation/focus and environment inheritance need
desktop-specific validation. This patch does not promise to fix unrelated GTK
error-handler interactions in other AWT facilities.

Related existing reports: JDK-8240572 (JavaFX/Desktop interaction) and
JDK-8275494 (Desktop/GTK main-loop dependency). The latter includes an upstream
suggestion to investigate calling GIO directly. Neither report implies approval
of this implementation.

## Build

Use a Linux x86-64 build environment with a complete JDK 21, GCC 12, autoconf,
make, zip/unzip, ALSA, CUPS, fontconfig/freetype and X11 development packages.
The GIO/GTK packages are runtime requirements for the regression tests, not new
link-time requirements for `libawt_xawt`.

```sh
BOOT_JDK=/path/to/jdk-21 JOBS=8 CONF=desktop-gio \
    sh doc/desktop-gio-experiment/build.sh
```

The complete image is `build/desktop-gio/images/jdk`. Keep it separate from your
installed JDK. Run the same build from the unmodified baseline for comparison.

## Regression checks

Run the `test/jdk/java/awt/Desktop` jtreg directory with native tests enabled and
a working graphical desktop. `DesktopGdkLockTest` deliberately holds the real
GTK3 lock across `Desktop.open`; its watchdog kills a blocked child JVM.
The original backend must fail this test and the GIO backend must pass it.
`DesktopGioTest` verifies no GTK/GDK mapping, capability flags and error handling.
The existing GTK-load test now checks that Desktop does not initialize GTK.

For an independent JavaFX test, use the existing JavaFX 21.0.8 platform jars:

```sh
JDK=/path/to/test-jdk
FX=/path/to/javafx-platform-jars
mkdir -p build/desktop-probes
"$JDK/bin/javac" --module-path "$FX" --add-modules javafx.controls \
    -d build/desktop-probes doc/desktop-gio-experiment/FxDesktopProbe.java
"$JDK/bin/java" --module-path "$FX" --add-modules javafx.controls \
    -cp build/desktop-probes FxDesktopProbe /path/to/existing/directory 20
```

Each call runs on the JavaFX Application Thread and must be followed by a
heartbeat. Verify the file manager's location independently while the JVM is
still running. Use an external watchdog when testing the unmodified JDK.
Test non-ASCII/spaced paths, ordinary files, AWT EDT/background callers, missing
GIO symbols and failed URI launches as well. Preserve separate logs and source
SHAs for each baseline. See RESULTS.md for the actual tested coverage.
