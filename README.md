# ISMErrDetect
## Effect of CRC-8 algorithm 'init' on detection of burst insertions into ISM messages 

In his paper with its detailed explanation of the CRC-8 algorithm, [Williams](http://ross.net/crc/download/crc_v3.txt), he mentions that the initial value of the remainder, frequently called 'init' should proably not be 0x00.  In his paper analyzing the "best" parameters for CRC-8 based on the size of the message, [Koopman](https://users.ece.cmu.edu/~koopman/roses/dsn04/koopman04_crc_poly_embedded.pdf), analyzed for the "best" polynomial but not the 'init' value.
