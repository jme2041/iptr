iptr:: Smart Pointer for COM Interfaces
=======================================

``iptr.h`` is a C++ header that implements ``IPtr::IPtr``, a smart pointer for
wrapping Component Object Model (COM) interfaces. ``IPtr`` is based on and
provides a non-Windows Runtime version of ``Modern::ComPtr``. ``ComPtr`` was
originally developed by Kenny Kerr and released under the MIT license.

To use ``IPtr``, include ``iptr.h`` in a C++ project. This repository includes
a test program, ``test_iptr``. To build and run ``test_iptr`` using CMake, use
the following from within the ``iptr`` directory::

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug -G "NMake Makefiles" ..
    cmake --build ..
    test_iptr.exe

License
=======

Copyright (c) 2021, Jeffrey M. Engelmann

``iptr`` is released under the MIT license.
For details, see LICENSE.txt.
