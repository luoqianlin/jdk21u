/*
 * Experimental AI-assisted regression test for this personal fork.
 * Distributed under GPLv2; see LICENSE. Not submitted to OpenJDK.
 */

/*
 * @test
 * @key headful
 * @summary Desktop loads GIO without initializing GTK and preserves OPEN error handling
 * @requires (os.family == "linux")
 * @run main/othervm DesktopGioTest
 */

import java.awt.Desktop;
import java.io.IOException;
import java.net.URI;
import java.nio.file.Files;
import java.nio.file.Path;

public class DesktopGioTest {
    public static void main(String[] args) throws Exception {
        if (!Desktop.isDesktopSupported()) {
            throw new AssertionError("Desktop must be supported on the test desktop");
        }
        Desktop desktop = Desktop.getDesktop();
        if (!desktop.isSupported(Desktop.Action.OPEN)
                || desktop.isSupported(Desktop.Action.EDIT)
                || desktop.isSupported(Desktop.Action.PRINT)) {
            throw new AssertionError("Unexpected Desktop capabilities");
        }
        Path missing = Path.of("nonexistent-desktop-gio-test-file").toAbsolutePath();
        if (Files.exists(missing)) {
            throw new AssertionError("Test requires a nonexistent file: " + missing);
        }
        try {
            desktop.open(missing.toFile());
            throw new AssertionError("Missing file should fail");
        } catch (IllegalArgumentException expected) {
            System.out.println("PASS: missing file rejected");
        }
        if (desktop.isSupported(Desktop.Action.BROWSE)) {
            try {
                desktop.browse(URI.create("desktop-gio-unregistered-test-scheme://invalid"));
                throw new AssertionError("Unregistered URI scheme should fail");
            } catch (IOException expected) {
                System.out.println("PASS: GIO launch failure reported as IOException: " + expected.getMessage());
            }
        }
        assertNoGtk();
        System.out.println("PASS: GIO backend did not load GTK or GDK");
    }

    private static void assertNoGtk() throws IOException {
        String maps = Files.readString(Path.of("/proc/self/maps"));
        if (maps.contains("/libgtk-") || maps.contains("/libgdk-")) {
            throw new AssertionError("Desktop initialized GTK/GDK");
        }
    }
}
