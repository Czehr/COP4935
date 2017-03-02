import static java.lang.Math.max;
import static java.lang.Math.min;

/**
 * Name:       Kenneth Lamar
 * Course:     COP4520 Spring 2017
 * Instructor: Dr. Damian Dechev
 */
public class Philosopher implements Runnable {
    private static final Boolean FIRST = false, SECOND = true;

    // This index is used to that the philosopher knows which "seat" is his
    private int index = -1;

    // The state of the philosopher
    private byte state = -1;
    private static final byte THINKING = 0, HUNGRY = 1, EATING = 2, STARVED = 3;

    // These variables hold the status of our chopsticks
    // -1 = no chopstick, all others are the index of the chopstick in hand
    int first = -1, second = -1;

    // Hunger controls when the philosopher eats
    // When it goes under a certain value, the philosopher will try to eat
    private int hunger = 100;

    private static final long THINK_TIME = 100, EAT_TIME = 200;

    @Override
    public void run() {
        while (hunger > 0) {
            // Keep thinking
            think();

            // Attempt to eat if he becomes too hungry
            while (hunger <= 10 && hunger > 0) eat();
        }
        // The philosopher will die if he doesn't eat in time
        setState(STARVED);
    }

    // Constructor used to ensure that a philosopher gets his assigned "seat"
    public Philosopher(int index) {
        this.index = index;
    }

    private void think() {
        setState(THINKING);

        timePass(THINK_TIME);
    }

    private void eat() {
        setState(HUNGRY);

        pickupChopsticks();

        // Once both chopsticks are in hand, begin eating for a time
        if (first != -1 && second != -1) {
            setState(EATING);

            timePass(EAT_TIME);

            // After eating, the philosopher becomes full
            hunger = 100;

            // Return the chopsticks when full
            Main.chopsticks[first] = Main.AVAILABLE;
            Main.chopsticks[second] = Main.AVAILABLE;
        } else
            timePass(THINK_TIME);

        return;
    }

    private void pickupChopsticks() {
        // Attempt to pick up the first chopstick if he doesn't have it already
        if (first == -1) first = pickupChopstick(FIRST);

        // Attempt to pick up the second chopstick if he doesn't have it already and also has the first chopstick
        // This is done to prevent deadlock
        if (first != -1 && second == -1)
            second = pickupChopstick(SECOND);

        // Put the first chopstick back if he was unable to pick up the second chopstick as well
        if (first != -1 && second == -1)
            Main.chopsticks[first] = Main.AVAILABLE;
    }

    // Attempt to pick up a chopstick and returns the index of that chopstick
    // Returns -1 if he was unable to pick it up
    private synchronized int pickupChopstick(Boolean side) {
        // Figure out where the chopstick is on the table
        int spot;
        // The first chopstick is always the one with the lowest index that the philosopher can reach
        if (side == FIRST)
            spot = min((index) % Main.num, (index + 1) % Main.num);
        else
            spot = max((index) % Main.num, (index + 1) % Main.num);

        // If the chopstick is available
        if (Main.chopsticks[spot] == Main.AVAILABLE) {
            // Take the chopstick if it is available
            Main.chopsticks[spot] = Main.TAKEN;
            return spot;
        }
        return -1;
    }

    private void setState(byte state) {
        byte oldState = this.state;
        this.state = state;

        // If our state has changed
        if (state != oldState) {
            if (state == THINKING) System.out.println(index + " is now thinking");
            if (state == EATING) System.out.println(index + " is now eating");
            if (state == HUNGRY) System.out.println(index + " is now hungry");
            if (state == STARVED) System.out.println(index + " has starved");
        }
    }

    // Use this wait period to slow the processing of events
    private void timePass(long time) {
        // Think for a moment at a time
        try {
            Thread.sleep(time);
        } catch (InterruptedException e) {
            System.err.println("Philosopher " + index + " failed to have his thread sleep.");
            e.printStackTrace();
            System.exit(-1);
        }

        // The philosopher becomes hungrier over time
        hunger--;
    }
}
