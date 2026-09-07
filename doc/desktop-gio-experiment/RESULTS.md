# Experimental results

## Current JDK 21u validation

Validated on 2026-09-07 using these source revisions:

- Official personal-fork `master` baseline:
  `b367e720927d1916abe446acd7b1c9c31c68745a`
- Desktop GIO implementation and regression tests:
  `1e2ba9fd9ecc7f2c32ae5e36c6facd524b24794c`

Both source trees produced `images` and `test-image` successfully in the same
Ubuntu 24.04 container with GCC 12.4, a JDK 21.0.2 bootstrap, `JOBS=110`, and
CPU set 0-109. The resulting runtime identifies itself as
`21.0.13-internal`. The patched `libawt_xawt.so` has no direct GIO, GTK, or GDK
link-time dependency.

The runtime checks used the local Linux x86-64 X11 session:

| Scenario | Current official baseline | Current GIO backend |
| --- | --- | --- |
| GDK lock held across `Desktop.open()` | Child watchdog reported the deadlock after 20 seconds | Returned normally |
| JavaFX Application Thread | First open blocked and the probe reported failure | 20 opens and 20 heartbeats passed |
| Path with spaces and non-ASCII characters | Blocked before completing the first open | All 20 opens completed |
| GIO API and mapping checks | Not applicable | Error propagation passed; no GTK/GDK mapping |
| Desktop GTK-load test | Not applicable | Passed with exit code 0 |

The GDK-lock test opens the stable test working directory. It does not delete a
temporary directory after the asynchronous launch request, so the file manager
cannot race with test cleanup and report a missing path.

The JavaFX run still emitted one GTK `XSetErrorHandler()` warning. It did not
block the event thread and is outside this AWT Desktop backend change.

## Original JDK 21.0.2 validation

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
