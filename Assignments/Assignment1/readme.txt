Name:       Kenneth Lamar
Course:     COP4520 Spring 2017
Instructor: Dr. Damian Dechev

How to compile and run these programs:

Ensure that your JDK bin folder is in your PATH system variable. On Windows, the default location that needs to be added to the PATH system variable is "C:\Program Files\Java\jdk1.8.0_111\bin"
Each problem has a "runner.bat" file with the necessary commands to compile and run. These commands can also be run individually at your own discretion. 

For reference, to compile, please use the following commands: 
javac Main.java
jar cvfm Program.jar manifest.txt *.class

To run the program, use this command:
java -jar Program.jar

To run problem 2 version 4, use the following command, where n is the number of philosophers:
java -jar Program.jar n

Important note:
The dining philosophers problems do not terminate when n is pressed. Java does not have a platform-independent way to do this. If you want to end the program, use ordinary program terminating methods, such as ^C in most operating system terminals. 



Note that the source code is heavily commented to ensure that it is easy to understand. The writeups in this readme are a high-level explanation of what's going on in each program.

Problem 1

This program finds primes using an enhanced version of trial division. It is enhanced because it ends early in simple cases, such as divisibility by 2 or 3. The isPrime() method is based on the pseudocode provided in Wikipedia's primality test article. 
To take full advantage of the 8 threads made available, work is divided roughly equally based on runtime information that I logged for single thread performance. 
A byte array holds information about which numbers are prime. The index represents the number, which can be either prime or composite. This array is used at the end to compute the top 10 primes. 
Output results were confirmed via Wolfram|Alpha queries to ensure accuracy and correctness. Runtimes are reported in nanoseconds. 


Problem 2

Version 1 sets the foundation for the rest of the versions. By making the pickupChopstick() method synchronized, we can ensure that multiple philosophers never hold the same chopstick at the same time. Using the setState() method allows each thread to report only when their state changes. Philosophers start at 100% full, but will become hungry at 10%, where they will then attempt to eat. If they cannot eat, they will wait a moment, then try to pick up the missing chopsticks again. Because all threads start at the same time with the same hunger, they all get hungry at about the same time. Because of this, they all pick up the left chopstick and cannot pick up the (now taken) right chopstick. Thus, they deadlock and starve. 

Version 2 builds upon version 1. However, to prevent deadlock, the philosopher will put the left chopstick back and wait for a moment if he was unable to also pick up the right chopstick. This program has been run for over 5 minutes to confirm that deadlock does not occur practically. 

Version 3 is just like version 2. However, to prevent any potential for starvation, the resource hierarchy solution proposed by Dijkstra has been implemented. The pickupChopsticks() method has been altered. This new implementation numbers the chopstick indices from 0 through 4. Instead of attempting to pick up the left chopstick followed by the right chopstick, as in version 3, this version has a philosopher pick up the lowest indexed chopstick nearest him, followed by the second nearest. This ensures that Philosopher 0 or 4 will never have to wait in the case of what could otherwise be a deadlock. This program has been run for over 5 minutes to ensure that starvation never occurs. It could be run longer, but it appears that a sort of natural pattern emerges after they first resolve when they eat. This pattern continues in a repeating cycle for when philosophers become hungry and when they eat. Even if this pattern did not occur, Dijkstra's solution has a well-documented correctness. 

Version 4 has been modified so that the formerly constant value NUM is now a variable set by a command line argument. As previous versions were developed with this in mind, simply changing the value was simple. However, extra code was needed to handle the edge cases where there are 0 or 1 philosopher. In the case of 0, it initializes no threads, starts nothing, and simply ends. In the case of 1 philosopher, a second chopstick is provided so that he won't starve. Asside from the abillity to select an arbitrary number of philosophers, version 4 reamins unchanged from version 3. Thus, in terms of starvation and deadlock, it is just as correct. 