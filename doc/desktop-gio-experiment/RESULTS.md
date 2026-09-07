# Experimental results

Tested on 2026-09-07 on Linux x86-64 with a graphical X11 session. Both images
were built from the `jdk-21.0.2-ga` source baseline with GCC 12. The fixed image
contained only the changes in this experiment.

## GDK lock regression

`DesktopGdkLockTest` acquires GTK3's GDK lock and calls `Desktop.open` from the
same thread. An external 25 second timeout was also applied to each run.

| Image | Result |
| --- | --- |
| Unmodified JDK 21.0.2 | Child watchdog fired after 20 seconds: `Desktop.open deadlocked while holding the GDK lock` |
| GIO backend | Returned normally: `PASS: Desktop.open returned with the GDK lock held` |

## JavaFX event-thread regression

`FxDesktopProbe` used JavaFX 21.0.8 and invoked `Desktop.open` 20 times from the
JavaFX Application Thread. Every returned call scheduled a JavaFX heartbeat.

| Image | Result |
| --- | --- |
| Unmodified JDK 21.0.2 | Blocked on the first call; an external 15 second watchdog terminated the JVM |
| GIO backend | Completed all 20 calls and all 20 heartbeats |

The GTK warning `XSetErrorHandler() called with a GDK error trap pushed` was
still printed once by the JavaFX process. It did not block the event thread and
is outside the AWT Desktop backend addressed here.

## API checks

`DesktopGioTest` passed with the fixed image. It confirmed that a missing file
is rejected, a GIO URI-launch failure is surfaced as `IOException`, and the AWT
Desktop initialization does not map GTK or GDK into an otherwise plain AWT
process.

These results validate the tested Linux desktop and are not an upstream review,
approval, or compatibility claim for other desktop environments.
