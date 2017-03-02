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
    public static Philosopher[] Philosophers;
    public static Boolean[] Chopsticks;

    public static void main(String[] args) {

        // Initialize the correct number of philosophers
        Philosophers = new Philosopher[NUM];
        ExecutorService executor = Executors.newFixedThreadPool(NUM);

        // Assign each philosopher his seat
        for (int i = 0; i < NUM; i++) {
            Philosophers[i] = new Philosopher(i);
        }

        // Create the corresponding chopsticks
        Chopsticks = new Boolean[NUM];

        // Start the philosophers
        for (int i = 0; i < NUM; i++) {
            executor.execute(Philosophers[i]);
        }

        // Stop the threads once the program is complete
        executor.shutdown();
        while (!executor.isTerminated()) {
        }

        return;
    }
}
