import java.io.FileNotFoundException;
import java.io.PrintWriter;

/**
 * Name:       Kenneth Lamar
 * Course:     COP4520 Spring 2017
 * Instructor: Dr. Damian Dechev
 */
public class Thread implements Runnable {
    private final int start;
    private final int end;

    private long numPrimes = 0;
    private long sumPrimes = 0;

    @Override
    public void run() {
        for (int i = start; i < end; i++) {
            if (isPrime(i)) {
                Main.nums[i] = Main.PRIME;
                incrementPrimeTotals(i);
            }
        }
        incrementFinalPrimeTotals(numPrimes, sumPrimes);
    }

    public Thread(int start, int end) {
        this.start = start;
        this.end = end;
    }

    // Adapted from Wikipedia primality pseudocode
    private boolean isPrime(int n) {
        // Numbers less than or equal to 1 are never prime
        if (n <= 1) return false;
        // 2 and 3 are prime
        if (n <= 3) return true;
        // Multiples of 2 and 3 are composite
        if (n % 2 == 0 || n % 3 == 0) return false;
        // Only go to the square root of our number. Anything above that cannot be a multiple
        for (int i = 5; i * i <= n; i += 6)
            if (n % i == 0 || n % (i + 2) == 0) return false;
        return true;
    }

    // Does not need to be synchronized as each thread maintains its own local counter until the end
    private void incrementPrimeTotals(int i) {
        // Add to the total number of primes found
        numPrimes++;
        // Add to the sum of all primes
        sumPrimes += i;
    }

    // This is synchronized to ensure that counters are not overwritten by other threads
    // Reduce thread wait periods by giving each thread their own counter and summing all of them at the end
    public static synchronized void incrementFinalPrimeTotals(long numPrimes, long sumPrimes) {
        Main.numPrimes += numPrimes;
        Main.sumPrimes += sumPrimes;
    }
}
