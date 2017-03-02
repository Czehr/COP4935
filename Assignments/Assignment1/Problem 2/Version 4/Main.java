import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Name:       Kenneth Lamar
 * Course:     COP4520 Spring 2017
 * Instructor: Dr. Damian Dechev
 */
public class Main {
    public static final Boolean TAKEN = true, AVAILABLE = false;

    public static int num;
    public static Philosopher[] philosophers;
    public static Boolean[] chopsticks;

    public static void main(String[] args) {

        // Take in the number of philosophers we want to sit around the table
        if (args.length != 1) {
            System.out.println("Please type the number of philosophers as a command line argument.");
            return;
        }
        num = Integer.parseInt(args[0]);


        // Initialize the correct number of philosophers
        philosophers = new Philosopher[num];
        ExecutorService executor;
        if (num > 1)
            executor = Executors.newFixedThreadPool(num);
        else
            executor = null;


        // Assign each philosopher his seat
        for (int i = 0; i < num; i++) {
            philosophers[i] = new Philosopher(i);
        }

        // Create the corresponding chopsticks
        // If there is only one philosopher, we will give him two chopsticks so he won't starve
        chopsticks = new Boolean[num == 1 ? 2 : num];
        for (int i = 0; i < num; i++) {
            chopsticks[i] = AVAILABLE;
        }

        // Start the philosophers
        if (executor != null)
            for (int i = 0; i < num; i++) {
                executor.execute(philosophers[i]);
            }
        else
            for (int i = 0; i < num; i++)
                philosophers[i].run();

        // Stop the threads once the program is complete
        if (executor != null) {
            executor.shutdown();
            while (!executor.isTerminated()) {
            }
        }

        return;
    }
}
