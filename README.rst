###################
 DES in C
###################
C implementation of Data Encryption Standard algorithm.

Overview
========
The Data Encryption Standard (DES) is a block cipher (a form of shared secret encryption) that was selected by the National 
Bureau of Standards as an official Federal Information Processing Standard (FIPS) for the United States in 1976 and which 
has subsequently enjoyed widespread use internationally. It is based on a symmetric-key algorithm that uses a 56-bit key.

This implementation of DES is not optimized in any way. The code has been written to provide readability and easy 
understanding of the algorithm. Padding scheme used in this implementation is `[PKCS5] <ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-5v2/pkcs5v2-0.pdf>`_

Compilation & Installation
==========================
This implementation has only been tested on Unix platform. But you may be able to compile/ run it on Windows.

1. Make sure des.c, des.h and run_des.c are in the same directory 
2. Compile using: gcc -O3 des.c run_des.c -o run_des.o   

Usage
=====
Say we want to encrypt/ decrypt a file named /home/user/sample.txt

1. Generate a keyfile using::

    run_des.o -g /tmp/keyfile.key
2. Encrypt sample.txt using::

    run_des.o -e /tmp/keyfile.key /home/user/sample.txt /home/user/sample.enc
3. Decrypt sample.txt using::

    run_des.o -d /tmp/keyfile.key /home/user/sample.enc /home/user/sample_decrypted.txt

Don't lose the key file! you won't be able to decrypt an encrypted if you lose the keyfile.

More
====
DES is provided for educational purposes only. Do not use for any other reason.
It has been implemented after `J. Orlin Grabbe's DES Algorithm Illustrated <http://orlingrabbe.com/des.htm>`_

It is possible to use this implementation to facilitate TripleDES encryption process:

1. Generate keys using::

    run_des.o -g /tmp/keyfile1.key
    run_des.o -g /tmp/keyfile2.key
    run_des.o -g /tmp/keyfile3.key

2. Encrypt using::

    run_des.o -e /tmp/keyfile1.key /home/user/sample.txt /home/user/sample.enc1
    run_des.o -e /tmp/keyfile2.key /home/user/sample.enc1 /home/user/sample.enc2
    run_des.o -e /tmp/keyfile3.key /home/user/sample.enc2 /home/user/sample.enc3

3. Decrypt using::

    run_des.o -d /tmp/keyfile3.key /home/user/sample.enc3 /home/user/sample.dec3
    run_des.o -d /tmp/keyfile2.key /home/user/sample.dec3 /home/user/sample.dec2
    run_des.o -d /tmp/keyfile1.key /home/user/sample.dec2 /home/user/sample_decrypted.txt

The primary repository for DES is located at: `http://github.com/tarequeh/DES/ <http://github.com/tarequeh/DES/>`_ The blog post
discussing the implementation can be found at: `CodeXN <http://www.codexn.com>`_
This implementation of DES was written by `Tareque Hossain <mailto:tareque@codexn.com>`_
