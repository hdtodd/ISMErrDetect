# ISMErrDetect
## Effect of CRC-8 algorithm 'init' on detection of burst insertions into ISM messages 

In his paper with its detailed explanation of the CRC-8 algorithm, [Williams](http://ross.net/crc/download/crc_v3.txt) mentions that the initial value of the remainder, frequently called 'init', should probably not be 0x00.  In their paper analyzing the "best" parameters for CRC-8 based on the size of the message, [Koopman and Chakravarty](https://users.ece.cmu.edu/~koopman/roses/dsn04/koopman04_crc_poly_embedded.pdf), analyzed for the "best" polynomial but not the 'init' value.

In my initial development of the interchangeable `C` and `Python` [CRC-8 libraries](http://github.com/hdtodd/CRC8-Library) and of the ["Omnisensor"](http://github.com/hdtodd/omnisensor_433) protocol for flexible ISM-band remote sensor transmissions, I used an 'init' value of 0x00 (having forgotten Williams' admonition not to do so).  When that oversight was called to my attention, I was unable to find analyses that reported "best" values for 'init'.  So I created a small program to see if there is such a value in what I thought might be reasonable assumptions about the types of errors ISM broadcasts might encounter.

### Summary conclusion
There doesn't seem to be a "best" value for 'init', as least for the types of errors considered in this model.  In general, for an 80-bit message (72 data and 8 CRC bits), the CRC-8 algorithm detects 88% to 91% of 1 to 8-bit block errors, whether the error is a burst insertion of 0's or of 1's.

### Burst Insertion Model

This model explores the impact of the insertion of a block of 0's or 1's into an ISM transmission message of 80 bits.  The "impact" evaluated is the ability of the algorithm to determine, at the receiving end, whether the message has been corrupted in transmission.  The validation test compares the computed CRC with the transmitted CRC: if they are equal, the message is assumed to be valid, and if not, the message is presumed to have been corrupted.

The program inserts blocks of 1 to 8 bits of first 0's and then 1's into the 80-bit message and recomputes the CRC to determine if the error would be detected.  For each block size, the program tests the effect of inserting the block aligned at bit 1 (the left-most bit) of the message, then moves the block to start at bit 2, etc., until just the left-most bit of the block is corrupting just the right-most bit of the message.

The message itself is a string of 9 bytes of randomized bits, augmented by the CRC byte to make a 10-byte, 80-bit message.  The polynomial used is Koopman and Chakravarty's "best" for this size message: 0x97.

The program ran the test of inserting the error block into 100 different randomized messages so that the results converged to stable values.

For each possible 'init' value from 0x00 to 0xFF, and for block sizes of 1 to 8 for each of those 'init' values, the results were reported as:
-  the count of messages in which the inserted block had "NoEffect" (did not change the message and so could not be detected),
-  the count of those 0-block insertions that were detected and missed,
-  the count of those 1-block insertions that were detected and missed,
-  the percentage of *observably corrupted* 0-block insertion messages, and
-  the percentage of *observably corrupted* 1-block insertion messages.

To check the impact upon messages that might have more systematic (less randomized) bit patterns, the program was modified to create messages that began with 4 bytes of 0's and then with 4 bytes of 1'.  The modeling was repeated with those more structured messages.

### Results

For completely randomized 72-bit payload data, the CRC-8 algorithm consistently detected 88% to 91% of the corrupted messages, whether they were anything from 1-bit to 8-bit blocks, regardless of the value of 'init'.

For 72-bit payload data with 4 bytes of 0's followed by 5 bytes of randomized data, the error detection rate *dropped by about 8%*. 

And similarly, for 72-bit payload data with 4 bytes of 1's followed by 5 bytes of randomized data, the error detection rate *dropped by about 7%*.

### Repository Contents

The repository has the modeling codes and the gziped output from the simulation models for the fully-randomized 72-bit message (ISMErrDetect.c) and for the simulations with messages with initial blocks of 4 bytes of 0's (MsgBlock0.c) and 1's (MsgBlock1.c).

To compile and run your own simulations:
```
gcc ISMErrDetect.c libcrc8.c
./a.out  (or ./a.out > mytest.txt)
```

Enabling DEBUG in the `C` programs generates a *great deal of output*: if you enable DEBUG, you might want to reduce the REPEATS from 100 to 1 and perhaps limit the range of 'init' values rather than use the full 0x00 ... 0xff range.

### Author

David Todd, hdtodd@gmail.com, 2025.05.09.





