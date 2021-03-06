# gmt-cpp: An Implementation of Generic Message Trees in C++

An implementation of Generic Message Trees (GMTs) in C++ as presented in our
manuscript "Exploiting Dissent: Towards Fuzzing-based Differential Black-Box
Testing of TLS Implementations". The paper is to appear in IEEE Transactions on
Dependable and Secure Computing (TDSC). A [preprint](https://ivesk.hs-offenburg.de/fileadmin/Einrichtungen/ivesk/files/preprint_TLS-Diff-Fuzzing_IEEE-TDSC.pdf)
is available from our [institute's homepage](https://ivesk.hs-offenburg.de).

Further information about the concept can also be found in our paper "eTPL: An
Enhanced Version of the TLS Presentation Language Suitable for Automated Parser
Generation", 9th IEEE International Conference on Intelligent Data Acquisition
and Advanced Computing Systems: Technology and Applications (IDAACS’2017),
21-23 September 2017, Bucharest, Romania. A [preprint](https://ivesk.hs-offenburg.de/fileadmin/Einrichtungen/ivesk/files/preprint_eTPL_IDAACS-2017.pdf)
is also available.


## How to install

1. Clone this repository

```bash
git clone https://github.com/phantax/gmt-cpp.git
```

2. Clone the [bitbuffers-cpp repository](https://github.com/phantax/bitbuffers-cpp) and create link in the gmt-cpp directory

```bash
git clone https://github.com/phantax/bitbuffers-cpp.git
ln -s ../bitbuffers-cpp gmt-cpp/bitbuffers-cpp
```

3. Clone the [propertynode-cpp repository](https://github.com/phantax/propertynode-cpp) and create link in the gmt-cpp directory

```bash
git clone https://github.com/phantax/propertynode-cpp.git
ln -s ../propertynode-cpp gmt-cpp/propertynode-cpp
```

4. Build gmt-cpp library including libraries it depends on (bitbuffers-cpp and propertynode-cpp)

```bash
cd gmt-cpp
make deps
make
```


## How to use

gmt-cpp is a library that implements the GMT concept in C++. In order to make use of the code, you need an application that links against gmt-cpp.

There is a test application in the test/ subdirectory, which you might use as a starting point. In order to build and run the test application (after the gmt-cpp library has been built), type

```bash
make test
./test/test
```


## Contact

Andreas Walz (andreas.walz@hs-offenburg.de)  
[Institute of Reliable Embedded Systems and Communication Electronics (ivESK)](https://ivesk.hs-offenburg.de/)  
Offenburg University of Applied Sciences, Germany

