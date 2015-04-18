**this class has been removed on newer versions!**

# Introduction #

The **camera** class basically controls most of the OpenGL states and methods. This implementation will soon be modified, but for the meanwhile every object reports to the camera class when a local rotation or translation occurs.

# Details #

## Key Frames ##

This class also contains method to record and interpolate key frames. After a sequence of key frames is recorded there is an option to write it to file to be loaded in the future. For the meanwhile it always writes to a file _camera.frames_, overwriting if it already exists.

The key frame interpolation is done with Spherical Linear Interpolation (_slerp_). The light position is also recorded together with the camera position and orientation. In addition specific application variables, such as the _Reconstruction Filter Size_ and the _Prefilter Size_ are stored per key frame for video recording purposes.

**Note**: since object positions and orientations are not stored per key frame, remember to move the **camera** and not the object during path creation.