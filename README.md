# libprio - A Prio library in C using NSS 

**Warning:**
We do our best to write bug-free code, but I have no doubt
that there are scary bugs, side-channel attacks, and memory leaks 
lurking herein. 
If you are going to use this code for anything serious, please
give the source a careful read-through first and hammer it with
your favorite testing tools.

**Important:**
We have not yet implemented the items
described in the "Security-Critical TODOs" section below.
Without these features, do not use the code in a production environment.


## Overview

This is an implementation of the core cryptographic routines
for the [Prio system](https://crypto.stanford.edu/prio/) 
for the private computation of aggregate statistics:
> "Prio: Private, Robust, and Scalable Computation of Aggregate Statistics"<br>
> by Henry Corrigan-Gibbs and Dan Boneh<br>
> USENIX Symposium on Networked Systems Design and Implementation<br>
> March 2017

**Usage scenario.**
The library implements the cryptographic routines necessary
for the following application scenario:
Each client holds a vector of boolean values.
Each client uses the library to encode her private vector into two 
encoded packets&mdash;one for server A and one for server B.

After receiving shares from a client, the servers can use the routines
implemented here to check whether the client-provided packets are 
well formed. 
(Without this check, a single malicious client can corrupt the
output of the computation.)

After collecting data packets from many clients, the servers
can combine their state to learn how many clients had the
*i*th bit of their data vector set to `true` and how many
clients had the *i*th bit of their data vector set to `false`.
The servers learn *nothing else* about the clients' data.

For example, the *i*th bit of the data vector could indicate
whether the client ever visited the *i*th-ranked website
in the Alexa Top 500.
The servers would learn how many clients visited each of the 
Top 500 websites *without learning* which clients visited
which websites.

**Efficiency considerations.**
The code makes no use of public-key crypto, so it should 
be relatively fast.
When each a data packet is of length *N*, 
all arithmetic is modulo a prime *p* (we use an 87-bit prime by default), 
and "elements" are integers modulo *p*, 
the dominant costs of the system are:
* **Client compute:** O(*N* log *N*) multiplications 
* **Client-to-server communication:** 4*N* + O(1) elements<br>
    (NOTE: Using an optimization we haven't yet implemented, we can 
    drop this cost to 2*N* + O(1) elements.)
* **Server compute:** O(*N*) multiplications to check each packet 
* **Server-to-server communication:** O(1) elements
* **Server storage:** O(*N*) elements

## Running the code

After installing [NSS/NSPR](https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSS) 
and [scons](http://scons.org/), you should be able to run
and test the code using:
To compile the code, run:

    $ scons

To run the test suite, execute:

    $ build/ptest/ptest -v

To print debug messages while compiling:

    $ scons VERBOSE=1

To compile with debug symbols, run:

    $ scons BUILDTYPE=DEBUG

To clean up the object and binary files, run:

    $ scons -c

The files in this directory are:
````
/build      - Binaries, object files, etc.
/include    - Exported header files
              (Note: The public header is <mprio.h> since
              NSPR already has a file called <prio.h>.)
/libmpi     - NSS MPI bignum library 
/libmprio   - Prio library core code
/pclient    - Example code that uses the Prio library
/ptest      - Tests and test runner
````

## Security-Critical TODOs 
<a name="security"></a>
* Implement public-key encryption for client packets. 
  The client data packet for server A must be encrypted using
  the public key of server A. Same for server B.
* Check that our usage of the NSS random-number generator is correct.


## Optimizations and features not yet implemented
* **Client bandwidth.**
  Using a pseudorandom generator (e.g., AES in counter mode),
  we can reduce the client-to-server communication cost
  by a factor of 2x.
* **Server compute.**
  By using a fast polynomial interpolation-and-evaluation
  routine, we can reduce the cost of checking a single client
  request from O(*N* log *N*) multiplications down to O(*N*)
  multiplications, for a data packet of n items.
* **Differential privacy.**
  It would be very straightforward to add some small amount of 
  noise to the final statistics to provide differential privacy.
  If this would be useful, I can add it.
* **Seralization.**
  We haven't implemented the routines to serialize
  any of the data structures we use.
* **Misc.**
  There are TODO notes scattered throughout code indicating
  places for potential performance optimizations.
  

