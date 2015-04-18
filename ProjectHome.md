Pyramid-point-renderer is an image-space rendering algorithm for point-based models. It uses a pyramid scheme to reconstruct the model from the point samples with radius and normals.

This project includes codes for the main algorithm from three already published articles.
It uses the VCG library [VCG](http://vcg.sourceforge.net), and the _application_ files are a bridge between the interface and the actual rendering code. It currently uses GLUT, but it should be no trouble using other interfaces such as Qt (I've tried it before)

The Wiki is still under construction, but is being gradually updated. Remember to check specially the compiling page and the models to see what kind of files can be used.

For more information about this project follow the links on the sidebar, the first one included my PhD thesis on this topic (in Portuguese), while at the second link you can find some model files to use with this renderer. For more information about the system please refer to the following publications:

  * Efficient Point-Based Rendering Using Image Reconstruction. Ricardo Marroquim, Martin Kraus, Paulo Roma Cavalcanti, IEEE/Eurographics Symposium on Point-Based Graphics (PBG), September 2007

  * Efficient Image Reconstruction for Point-Based and Line-Based Rendering. Ricardo Marroquim, Martin Kraus, Paulo Roma Cavalcanti, Computer & Graphics, v. 32(2), p. 189-203, April 2008

  * High Quality Image Recontruction of Point Models. Ricardo Marroquim, Antonio Oliveira, Paulo Roma Cavalcanti, XXI Sibgrapi, October 2008

I hope you find this useful. Please don't hesitate to email me with any comments, doubts, suggestions or critics.