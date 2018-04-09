# Project 2: Distributed Password Cracker

See Project Spec here: https://www.cs.usfca.edu/~mmalensek/cs220/assignments/project-2.html

## Performance Evaluation

I attempted the extra credit but I'm not sure how well it works. Essentially what I did was if the rank is greater than the size of the character set, the start string is the rank modded with the size of the character set. So rank 52 on the alpha set will get a since 52%52 = 0 and `valid_chars[0]` is a. Then in the crack function, if the length of the string is equal to 1 (if we haven't added any characters yet) and the length of the character set is less than our rank. Then we add the character halfway across the character set from the current position of i in the for loop. This is because rank 0 should start with aaaa (if 4 letter password) and rank 52 should start with aAaa. They will each do half of the possible permutations, rank 0 may repeat a few of rank 52's.

### Password Recovery (1 pts)

Choose five entries from the password database and recover the passwords. List the username + password combinations here, along with the run time of your program and the number of hashes computed per second.

Using 52 cores


## 1
User: liu

4 characters and alpha

Hash: a3a73b6dfa8f4caedd0349f676ae46b39bdb7fbd

Password: Meow

Num Hashes: 972064 (3755112.27/s)

## 2

User: aditya

5 characters and alpha

Hash: f7ff9e8b7bb2e09b70935a5d785e0cc5d9d0abf0

Password: Hello 

Num Hashes: 26954161 (4789860.83/s)

## 3

User: carmen

6 characters and alpha

Hash: 64369a22cbc5686e2ccf609aae16fe42fa1178b4

Password: Bloink

Num Hashes: 4169047395 (1052427.06/s)

## 4 

User: niha

5 characters and alpha numeric

Hash: f4169f30903c1fca747cdcd7c2d0081a79e23514

Password: M0kaY

Num Hashes: 391992143 (3674576.52/s)

## 5 

User: erika

6 characters and alpha

Hash: 250e77f12a5ab6972a0895d290c4792f0a326ea8

Password: banana

Num Hashes: 178838067 (2352771.81/s)


### Performance Benchmarking (1 pts)

Choose one of the password hashes from the database (preferably one that runs for a while). Compare the run time with 4, 16, 32, and 64 processes. You'll need to run on the jet machines to do this. List the speedup and parallel efficiency for each.

Using erika's password

4 cores

3437.75s

Number of hashes: 1555336554 (452428.58/s)

8 cores

3238.62s 

Number of hashes: 3027910055 (934939.12/s)  

It makes sense that 4 cores and 8 cores took the same amount of time both rank 0's get start string a. No ranks get start string b and the password is banana. The same will be true with 16 cores and 32 cores so I won't test them for the sake of time. Lets try 64 cores. It should be faster since rank 1 will get start string b, and the password is bannana, so it will find it faster. The extra 12 cores starting with a 2 character start string, will not help in finding it faster (for this specific password).

64 cores

19.82s

Number of hashes: 121639100 (6136284.76/s)

### Algorithmic Trade-offs (1pts)

Brute-forcing passwords can take some time. What might be a better approach?  On the other hand, what is one guarantee our algorithm can provide that others may not be able to?

There are a lot of better approaches but the best is definitley just mass email scamming (eg nigerian prince) for information.
