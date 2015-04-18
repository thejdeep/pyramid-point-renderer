# Introduction #

The code is almost self-contained except for the VCG library and GLew,
The application classes are independent of the interface, thus
I have substituted the Qt interface for a lightweight GLut based version.

# Details #

Requirements:

  * I have successfully tested this code in a variety of computers and configurations, including an HP tx1215 laptop with a nVidia 6150 graphics board. However, I have only tested with nVidia boards, but I guess it should also work with ATI.

Before you compile:

  * Have **GLUT** and **GLEW** installed.

  * Download the [VCG](http://vcg.sourceforge.net) library, no need to install or compile, the necessary files are included in the Makefile. You can download it directly using svn:

> svn co https://vcg.svn.sourceforge.net/svnroot/vcg vcg

  * Set the VCGDIR path in the Makefile.in file to where you downloaded the VCG library

In the pyramid-point-renderer directory type the following:

  * make

  * ./ppr < plyfile >

I developed everything under Linux, but I have successfully compiled under Windows with Mingw and the Msys enviroment, never tested with Visual Studio.
Windows users may try to compile with (you might need to change the glut library name):

  * make OS=windows