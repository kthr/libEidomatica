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
sudo port install hdf5 +threadsafe +cxx cgal boost
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
make install
```
