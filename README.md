# ISMErrDetect
## Effect of CRC-8 algorithm 'init' on detection of burst insertions into ISM messages 

In his paper with its detailed explanation of the CRC-8 algorithm, [Williams](http://ross.net/crc/download/crc_v3.txt) mentions that the initial value of the remainder, frequently called 'init', should probably not be 0x00.  In their paper analyzing the "best" parameters for CRC-8 based on the size of the message, [Koopman and Chakravarty](https://users.ece.cmu.edu/~koopman/roses/dsn04/koopman04_crc_poly_embedded.pdf), analyzed for the "best" polynomial but not the 'init' value.

In my initial development of the interchangeable `C` and `Python` [CRC-8 libraries](http://github.com/hdtodd/CRC8-Library) and of the ["Omnisensor"](http://github.com/hdtodd/omnisensor_433) protocol for flexible ISM-band remote sensor transmissions, I used an 'init' value of 0x00 (having forgotten Williams' admonition not to do so).  When that oversight was called to my attention, I was unable to find analyses that reported "best" values for 'init'.  So I created a small program to see if there is such a value in what I thought might be reasonable assumptions about the types of errors ISM broadcasts might encounter.

### Summary conclusion
There doesn't seem to be a "best" value for 'init', as least for the types of errors considered in this model.  In general, for an 80-bit message (72 data and 8 CRC bits), the CRC-8 algorithm detects 88% to 91% of 1 to 8-bit block errors, whether the error is a burst insertion of 0's or of 1's.

### Burst Insertion Model




