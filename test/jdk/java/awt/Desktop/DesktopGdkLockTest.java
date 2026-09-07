/*
 * Experimental AI-assisted regression test for this personal fork.
 * Distributed under GPLv2; see LICENSE. Not submitted to OpenJDK.
 */

/*
 * @test
 * @key headful
 * @summary Desktop.open must complete when the caller already owns the GDK lock
 * @requires (os.family == "linux")
 * @run main/othervm/native DesktopGdkLockTest
 */

import java.awt.Desktop;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.concurrent.TimeUnit;

public class DesktopGdkLockTest {
    private static native boolean acquireGdkLock();
    private static native void releaseGdkLock();

    public static void main(String[] args) throws Exception {
        if (args.length == 0) {
            String nativePath = System.getProperty("test.nativepath",
                    System.getProperty("java.library.path"));
            Process child = new ProcessBuilder(
                    Path.of(System.getProperty("java.home"), "bin", "java").toString(),
                    "-Djava.library.path=" + nativePath,
                    "-cp", System.getProperty("java.class.path"),
                    DesktopGdkLockTest.class.getName(), "child")
                    .inheritIO().start();
            if (!child.waitFor(20, TimeUnit.SECONDS)) {
                child.destroyForcibly();
                child.waitFor();
                throw new AssertionError("Desktop.open deadlocked while holding the GDK lock");
            }
            if (child.exitValue() != 0) {
                throw new AssertionError("Child failed: " + child.exitValue());
            }
            return;
        }

        System.loadLibrary("DesktopGdkLockTest");
        if (!acquireGdkLock()) {
            throw new AssertionError("GTK3 and a working display are required");
        }
        Path directory = Files.createTempDirectory("desktop-gdk-lock-");
        try {
            if (!Desktop.isDesktopSupported() || !Desktop.getDesktop().isSupported(Desktop.Action.OPEN)) {
                throw new AssertionError("Desktop.OPEN is unavailable");
            }
            Desktop.getDesktop().open(directory.toFile());
            System.out.println("PASS: Desktop.open returned with the GDK lock held");
        } finally {
            releaseGdkLock();
            Files.delete(directory);
        }
    }
}
