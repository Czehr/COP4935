import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Name:       Kenneth Lamar
 * Course:     COP4520 Spring 2017
 * Instructor: Dr. Damian Dechev
 */
public class Main {
    public static final Boolean TAKEN = true, AVAILABLE = false;

    public static final int NUM = 5;
    public static Philosopher[] philosophers;
    public static Boolean[] chopsticks;

    public static void main(String[] args) {

        // Initialize the correct number of philosophers
        philosophers = new Philosopher[NUM];
        ExecutorService executor = Executors.newFixedThreadPool(NUM);

        // Assign each philosopher his seat
        for (int i = 0; i < NUM; i++) {
            philosophers[i] = new Philosopher(i);
        }

        // Create the corresponding chopsticks
        chopsticks = new Boolean[NUM];
        for (int i = 0; i < NUM; i++) {
            chopsticks[i] = AVAILABLE;
        }

        // Start the philosophers
        for (int i = 0; i < NUM; i++) {
            executor.execute(philosophers[i]);
        }

        // Stop the threads once the program is complete
        executor.shutdown();
        while (!executor.isTerminated()) {
        }

        return;
    }
}
