/*
 * AI-assisted standalone diagnostic for this personal fork; GPLv2.
 * Requires JavaFX controls on the module path. Not an OpenJDK contribution.
 */
import java.awt.Desktop;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;
import javafx.animation.PauseTransition;
import javafx.application.Platform;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.stage.Stage;
import javafx.util.Duration;

public class FxDesktopProbe {
    public static void main(String[] args) throws Exception {
        Path directory = Path.of(args[0]).toAbsolutePath();
        int repetitions = args.length > 1 ? Integer.parseInt(args[1]) : 20;
        if (!Files.isDirectory(directory)) throw new IllegalArgumentException("Not a directory: " + directory);
        CountDownLatch done = new CountDownLatch(1);
        AtomicReference<Throwable> failure = new AtomicReference<>();
        Platform.startup(() -> {
            Stage stage = new Stage();
            Button open = new Button("Open directory");
            stage.setScene(new Scene(open, 360, 160));
            stage.show();
            int[] count = {0};
            PauseTransition next = new PauseTransition(Duration.millis(250));
            next.setOnFinished(event -> open.fire());
            open.setOnAction(event -> {
                try {
                    System.out.println("OPEN " + (count[0] + 1) + " on " + Thread.currentThread().getName());
                    Desktop.getDesktop().open(directory.toFile());
                    PauseTransition heartbeat = new PauseTransition(Duration.millis(250));
                    heartbeat.setOnFinished(tick -> {
                        System.out.println("HEARTBEAT " + ++count[0]);
                        if (count[0] == repetitions) {
                            stage.close();
                            done.countDown();
                        } else {
                            next.playFromStart();
                        }
                    });
                    heartbeat.play();
                } catch (Throwable error) {
                    failure.set(error);
                    done.countDown();
                }
            });
            next.play();
        });
        if (!done.await(60, TimeUnit.SECONDS)) {
            System.err.println("FAIL: Desktop.open blocked the JavaFX event thread");
            Runtime.getRuntime().halt(124);
        }
        Platform.exit();
        if (failure.get() != null) {
            failure.get().printStackTrace();
            System.exit(1);
        }
        System.out.println("PASS: " + repetitions + " opens and JavaFX heartbeats");
    }
}
