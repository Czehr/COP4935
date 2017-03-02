import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Name:       Kenneth Lamar
 * Course:     COP4520 Spring 2017
 * Instructor: Dr. Damian Dechev
 */
public class Main {
    public static final int NUM_CORES = 8;
    public static final int SEARCH_SIZE = 100000000;

    // Holds references to the threads that we will run
    public static Thread[] threads = new Thread[NUM_CORES];

    // This array stores the states of each number
    public static byte[] nums = new byte[SEARCH_SIZE + 1];
    public static final byte COMPOSITE = 0;
    public static final byte PRIME = 1;

    // These are stored for final output
    public static long numPrimes = 0;
    public static long sumPrimes = 0;
    public static int[] topPrimes = new int[10];

    public static void main(String[] args) {

        // Determine the beginning time to determine runtime
        long startTime = System.nanoTime();

        // Spawn our threads
        // Times were determined based on real runtime information of single core
        threads[0] = new Thread(0, 21823397);
        threads[1] = new Thread(21823397, 36517781);
        threads[2] = new Thread(36517781, 49113667);
        threads[3] = new Thread(49113667, 60575189);
        threads[4] = new Thread(60575189, 71234881);
        threads[5] = new Thread(71234881, 81264917);
        threads[6] = new Thread(81264917, 90826979);
        threads[7] = new Thread(90826979, SEARCH_SIZE);

        // Run our threads
        ExecutorService executor = Executors.newFixedThreadPool(NUM_CORES);
        for (int i = 0; i < NUM_CORES; i++) {
            executor.execute(threads[i]);
        }

        // Stop running when the threads finish
        executor.shutdown();
        while (!executor.isTerminated()) {
        }

        // Determine the final time to determine the total runtime
        long totalTime = System.nanoTime() - startTime;

        findTopPrimes();
        writeOutput(totalTime);
    }

    // Save our output to a file
    private static void writeOutput(long totalTime) {
        try (PrintWriter out = new PrintWriter("primes.txt")) {
            out.println(totalTime + " " + numPrimes + " " + sumPrimes);
            for (int i = 0; i < 10; i++) {
                out.print(topPrimes[i] + " ");
            }
            out.close();
        } catch (FileNotFoundException e) {
            System.err.println("Failed to create output file. ");
        }
    }

    // Find the top 10 primes
    private static void findTopPrimes() {
        // Look backwards through our array to find the primes
        for (int i = SEARCH_SIZE, j = 9; i > 2; i--) {
            // Once we have filled the array, we're done
            if (j < 0) break;
            // See if our current index is a prime
            if (nums[i] == PRIME) {
                // Add it to our list if so
                topPrimes[j--] = i;
            }
        }
    }
}
