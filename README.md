Licensing information
===============
The code of this library is build for research purposes and published under the MIT license. It is part of the [Eidomatica](https://github.com/kthr/Eidomatica) Mathematica package. Third party algorithms/components in the lib directory have their own license though. Especially the gco algorithm, is only allowed to be redistributed for research purposes.

Relevant publications:

[http://www.csd.uwo.ca/~yuri/Papers/pami04.pdf](http://www.csd.uwo.ca/~yuri/Papers/pami04.pdf)

[http://www.csd.uwo.ca/faculty/olga/Papers/pami01_final.pdf](http://www.csd.uwo.ca/faculty/olga/Papers/pami01_final.pdf)

[http://www.csd.uwo.ca/~yuri/Papers/pami04.pdf](http://www.csd.uwo.ca/~yuri/Papers/pami04.pdf)

[http://www.cs.ucl.ac.uk/staff/V.Kolmogorov/papers/KZ-PAMI-graph_cuts.pdf](http://www.cs.ucl.ac.uk/staff/V.Kolmogorov/papers/KZ-PAMI-graph_cuts.pdf)
[http://www.csd.uwo.ca/~yuri/Papers/ijcv10_labelcost.pdf](http://www.csd.uwo.ca/~yuri/Papers/ijcv10_labelcost.pdf)


Installation
===============
libEidomatica is currently only tested on MacOSX and Linux. 

Dependencies
---------------
First install the following dependencies:
    cgal
    hdf5
    boost

### On Mac (via macports) install
```bash
sudo port install boost cgal glm hdf5 +threadsafe +cxx
```

Installation
--------------
First clone the repository with
```bash
git clone https://gitlab.com/kth/libEidomatica.git libEidomatica
```
Then create a directory "build" inside the cloned repository
```bash
cd libEidomatica
mkdir build
cd build
```
Finally install the library to your local Mathematica applications directory 
with
```bash
cmake ..
make install
```
