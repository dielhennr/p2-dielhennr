# Project 2: Distributed Password Cracker

See Project Spec here: https://www.cs.usfca.edu/~mmalensek/cs220/assignments/project-2.html

## Performance Evaluation

Edit this README.md file to answer the following questions:

### Password Recovery (1 pts)

Choose five entries from the password database and recover the passwords. List the username + password combinations here, along with the run time of your program and the number of hashes computed per second.

Using 52 cores
##1

User: aditya

Password: Hello 

Num Hashes: 26954161 (4789860.83/s)

##2
User: liu

Password: Meow

Num Hashes: 972064 (3755112.27/s)

##3
User:


### Performance Benchmarking (1 pts)

Choose one of the password hashes from the database (preferably one that runs for a while). Compare the run time with 4, 16, 32, and 64 processes. You'll need to run on the jet machines to do this. List the speedup and parallel efficiency for each.

(your answer)

### Algorithmic Trade-offs (1pts)

Brute-forcing passwords can take some time. What might be a better approach?  On the other hand, what is one guarantee our algorithm can provide that others may not be able to?

(your answer)
