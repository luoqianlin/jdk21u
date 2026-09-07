# Personal fork: Linux Desktop GIO fix

This fork's `master` branch carries an experimental Linux fix for a
`java.awt.Desktop.open()` deadlock seen when it is called on the JavaFX
Application Thread. JavaFX can already own the non-recursive GDK global lock,
while the original AWT backend enters that lock again before calling
`gtk_show_uri()`.

The patched backend dynamically loads `libgio-2.0.so.0` and calls
`g_app_info_launch_default_for_uri(uri, NULL, &error)` without initializing GTK
or entering GDK. The Java API is unchanged, and `libawt_xawt` gains no direct
GIO or GTK link-time dependency.

The latest JDK 21u baseline and patched builds were compared on Linux with a
real X11 session. The baseline reproduced both the GDK-lock and JavaFX-thread
deadlocks; the patched build completed the same calls, including 20 opens of a
directory whose path contains spaces and non-ASCII characters.

See [the implementation notes](doc/desktop-gio-experiment/README.md) and
[tested results](doc/desktop-gio-experiment/RESULTS.md). Related OpenJDK reports
are [JDK-8240572](https://bugs.openjdk.org/browse/JDK-8240572) and
[JDK-8275494](https://bugs.openjdk.org/browse/JDK-8275494).

This is an AI-assisted personal experiment, not an OpenJDK contribution,
approved patch, or supported JDK distribution. Its validation is limited to
the environments documented above.

---

# Welcome to OpenJDK 21 Updates!

The JDK 21 Updates project uses two GitHub repositories.
Updates are continuously developed in the repository [jdk21u-dev](https://github.com/openjdk/jdk21u-dev). This is the repository usually targeted by contributors.
The [jdk21u](https://github.com/openjdk/jdk21u) repository is used for rampdown of the update releases of jdk21u and only accepts critical changes that must make the next release during rampdown. (You probably do not want to target jdk21u).

For more OpenJDK 21 updates specific information such as timelines and contribution guidelines see the [project wiki page](https://wiki.openjdk.org/display/JDKUpdates/JDK+21u/).


For build instructions please see the
[online documentation](https://openjdk.org/groups/build/doc/building.html),
or either of these files:

- [doc/building.html](doc/building.html) (html version)
- [doc/building.md](doc/building.md) (markdown version)

See <https://openjdk.org/> for more information about the OpenJDK
Community and the JDK and see <https://bugs.openjdk.org> for JDK issue
tracking.
