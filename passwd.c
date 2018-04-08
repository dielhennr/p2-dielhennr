/**
 * passwd.c
 *
 * Given a length and SHA-1 password hash, this program will use a brute-force
 * algorithm to recover the password using hash inversions.
 *
 * Compile: mpicc -g -Wall passwd.c -o passwd
 * Run: mpirun --oversubscribe -n 4 ./passwd num-chars hash [valid-chars]
 *
 * Where:
 *   - num-chars is the number of characters in the password
 *   - hash is the SHA-1 password hash (may not be in uppercase...)
 *   - valid-chars tells the program what character set to use (numeric, alpha,
 *     alphanumeric)
 */

#include <ctype.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Provides SHA-1 functions: */
#include "sha1.c"

#define HASH_LENGTH 50

/* Defines the alphanumeric character set: */
char *alpha_numeric = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
char *numeric = "0123456789";
char *alpha = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
/* Pointer to the current valid character set */
char *valid_chars;

/* When the password is found, we'll store it here: */
char found_pw[128] = { 0 };

unsigned long inversions = 0;

int rank;

/**
 * Generates passwords in order (brute-force) and checks them against a
 * specified target hash.
 * Inputs:
 *  - target - target hash to compare against
 *  - str - initial string. For the first function call, this can be "".
 *  - max_length - maximum length of the password string to generate
 */
bool crack(char *target, char *str, int max_length) {
    int curr_len = strlen(str);
    char *strcp = calloc(max_length + 1, sizeof(char));
    strcpy(strcp, str);


    /* We'll copy the current string and then add the next character to it. So
     * if we start with 'a', then we'll append 'aa', and so on. */
    int i;
    int flag = 0;
    for (i = 0; i < strlen(valid_chars); ++i) {

        MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
        if (flag == 1){
            fflush(stdout);
            MPI_Recv(found_pw, 128, MPI_CHAR, MPI_ANY_SOURCE, 0,
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            return true;
        }
        
        strcp[curr_len] = valid_chars[i];
        if (curr_len + 1 < max_length) {
            bool found = crack(target, strcp, max_length);
            if (found) {
                return found;
            }
        } else {
            /* Only check the hash if our string is long enough */
            char hash[41];
            sha1sum(hash, strcp);
            inversions++;

            if (inversions % 1000000 == 0 && inversions != 0){

                printf("[%d|%lu] %s -> %s\n", rank, inversions, strcp, hash);
                fflush(stdout);
            }
            if (strcmp(hash, target) == 0) {
                /* We found a match! */
                strcpy(found_pw, strcp);
                fflush(stdout);
                return true;
            }
        }
    }
    free(strcp);
    return false;
}


/**
 * Modifies a string to only contain uppercase characters.
 */
void uppercase(char *string) {
    int i;
    for (i = 0; string[i] != '\0'; i++) {
        string[i] = toupper(string[i]);
    }
}

int main(int argc, char *argv[]) {


    int comm_sz,  name_sz, length;
    double start_time, end_time;
    char target[HASH_LENGTH];
    char hostname[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz); 
    MPI_Get_processor_name(hostname, &name_sz);

    if ((argc < 3 || argc > 4)) {
        if (rank == 0){
            printf("Usage: mpirun %s num-chars hash [valid-chars]\n", argv[0]);
            printf("  Options for valid-chars: numeric, alpha, alphanum\n");
            printf("  (defaults to 'alphanum')\n");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    length = atoi(argv[1]);
    strcpy(target, argv[2]);
    uppercase(target);

    if (length <= 0 || strlen(target) != 40) {
        if (rank == 0){
            printf("  Password length must be positive.");
            printf("  Length of hash must be 40.");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    if (argc == 4){
        if (strcmp(argv[3], "alpha") == 0) {
            valid_chars = alpha;
        }
        else if (strcmp(argv[3], "numeric") == 0){
            valid_chars = numeric;
        }
        else{
            valid_chars = alpha_numeric;
        }
    }else{
        valid_chars = alpha_numeric;
    }

    if (rank == 0){
        printf("Starting parallel password cracker\n");
        printf("Number of processes: %d\n", comm_sz);
        printf("Coordinator node: %s\n", hostname);
        printf("Valid characters: %s (%d)\n", valid_chars, strlen(valid_chars));
        printf("Target password length: %d\n", length);
        printf("Target hash: %s\n", target);
        fflush(stdout);
    }

    start_time = MPI_Wtime();
    //how many letters we have to give to each process.
    int num_letters_per;
    int num_letters_last_rank;

    //if character set is smaller than the number of cores runnning
    if (comm_sz > strlen(valid_chars)) {

        num_letters_per = 1;

        num_letters_last_rank = 1;
        
        
    }else{
        /**
         * character set / number of cores gives the number of characters each process
         * should get.
         */
        num_letters_per = strlen(valid_chars) / comm_sz;
        //how many letters we have to give to last process if division is not natural
        num_letters_last_rank = num_letters_per + strlen(valid_chars) % comm_sz;
    }

    int i;
    //Since num_letters_last_rank will be greater than or equal to num_letters
    //Need to make sure to iterate to get every character to the last process.
    for (i = 0; i < num_letters_last_rank; ++i) {
        //rank*num_letters gives start index of the partition in the character set.
        //add i so that the process will go through every character in its partition.
        char start_str[3];

        if (comm_sz <= strlen(valid_chars)){
            if (rank != comm_sz - 1) {
                //if we aren't last rank, need to skip creating a start string
                if (i >= num_letters_per) {
                    break; 
                }
                start_str[0] = valid_chars[rank*num_letters_per + i];
                start_str[1] = '\0';
            }
            else {
                //if we are the last rank we potentially have a larger partition
                start_str[0] = valid_chars[rank*num_letters_per + i];
                start_str[1] = '\0';
            }   
        }else/*else the character set is less than the number of cores we have*/{

            /**
             * if the rank is less than number of characters in the set, we just start 
             * that core with the character at the index of this cores rank
             */
            if (rank < strlen(valid_chars)){
                start_str[0] = valid_chars[rank*num_letters_per + i];
                start_str[1] = '\0';
            }
            /**
             * Otherwise that core will start with 2 characters instead of 1.
             * The first character will be repeated again but the second character will be
             * different.Consider running 12 cores on a numeric character set. 
             * 10 cores will start with a single number (0-9). The 1th core should start
             * with 01 and the 12th core should start with 11. This is because the first
             * core will go into crack and the first number added to it will be 0 so it
             * will look like 00. The second core will look like 10. We want to be able to
             * run additional permutations in parallel.
             *
             * rank % strlen(valid_chars) will essentially resart us to the begining of
             * the character set every time we add new permutations
             *
             * rank/strlen(valid_chars) will give us which character to add to create the
             * permutation. consider rank 53 on the alpha set. 53%52 = 1 so we will start 
             * rank 53 with the string ab.
             */
            else {
                start_str[0] = valid_chars[rank%strlen(valid_chars)];
                start_str[1] = valid_chars[(rank / strlen(valid_chars))];
                start_str[2] = '\0';
            }

        }

        printf("Rank: %d Starting with: %s\n", rank, start_str);

        bool found = crack(target, start_str, length);
        
        if (found) {
            int j;
            for (j = 0; j < comm_sz; ++j) {
                if (j != rank){
                    MPI_Send(found_pw, 128, MPI_CHAR, j, 0, MPI_COMM_WORLD);
                }
            }
        }
    }

    unsigned long global_sum;
    MPI_Reduce(&inversions, &global_sum, 1, MPI_UNSIGNED_LONG, 
            MPI_SUM, 0, MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    if (rank == 0){
        double time = end_time - start_time;
        printf("Operation complete!\n");
        printf("Time elapsed: %.2fs\n", time);
        if (strlen(found_pw) > 0) { 
            printf("Recovered password: %s\n", found_pw);
            fflush(stdout);
        }else {
            printf("Failed to recover password\n");
        }
        printf("Total Passwords Hashed: %lu (%.2f/s)\n", global_sum, global_sum / time);
    }

    MPI_Finalize();
    return 0;
}
