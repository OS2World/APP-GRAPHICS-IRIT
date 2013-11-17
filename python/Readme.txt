This directory contains all files necessary to run IRIT as an
extension of python.  It was tested to work on the Windows version of
python 2.5, 2.6, and 2.7 (http://www.python.org/)

This code does not compile as part of the regular IRIT build as it
requires python installation and libraries.

The following directories are available:

1. irit2py - builds irit2py, a tool to convert irt scripts to py
   scripts.  Not perfect but does 90% of the job and was used to convert
   all our irt scripts to py...  Not really needed to use IRIT under python.

2. python_dll - builds the python dll interface of IRIT.  Also requires an
   installation of swig (http://www.swig.org/).

3. scripts - all py scripts as examples, converted from irt scripts using
   irit2py.

--------------------------------------------------------------------------

The end result of the above build 2 (python_dll) is two new files:

					   irit.py  &  _irit.pyd.

Both files should be installed in the python's "lib\site-packages" directory.

--------------------------------------------------------------------------

The python interface was made possible with the support of Hutchinson/Total.


