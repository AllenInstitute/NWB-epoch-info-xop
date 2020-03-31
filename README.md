# mies-nwb2-compound-XOP

Provides read/write support for compound attribute used in nwb2.

Important notes on HDF5 library from the FAQ:

NOTE:
  The runtime should be /MD or /MDd whether you are building dynamically or statically.
  Using /MT or /MTd, requires advanced changes to the supplied HDF5 libraries.

The library names are DIFFERENT for static or dynamic linking.

For static linking you might include for example:   libszip.lib; libzlib.lib; libhdf5.lib
For dynamic linking you might include for example:   szip.lib; zlib.lib; hdf5.lib

The order that the libraries are listed is important. The external libraries should be listed first, followed by the HDF5 library, and then optionally the HDF5 High Level, HDF5 Fortran or HDF5 C++ libraries.
