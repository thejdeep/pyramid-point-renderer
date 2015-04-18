# Introduction #

A few point-based rendering algorithm are available. Any _ply_ model with normals and radius per vertex can use any rendering mode, except for the **Pyramid Lines** mode which is specific for tree models.

# Details #


## Pyramid Points ##

This is the basic Pyramid Point Rendering as described in:
http://www.lcg.ufrj.br/Projetos/pbr

## Tree Rendering ##

**not included anymore**

This mode is specific for rendering tree models. The interpolation algorithm was adapted to handle lines as well as points. For more details refer to the article to be published on **Computer & Graphics**:
http://dx.doi.org/10.1016/j.cag.2008.01.011

The tree rendering mode still has a few minor bugs that will be treated in a near future.

## Models with Level-of-Detail Structure ##

**not included anymore**

The Lod check box for example should only be used when a _.lod_ model has been loaded.
The point-based LOD rendering is still a work in progress, thus the reason for its significant bad performance compared to the plain point rendering.

More details about this project can be found at:
http://www.lcg.ufrj.br/Projetos/object-textures

The color bars depicts the number/percentage of points being rendered at each of the four possible LOD levels, where the colors are respectively from coarsest to finest:
Ruby - Turquoise - Gold - Silver

When the **Write LOD** option under the **File** menu is selected, a file with the same name as the read model with an extra _.lod_ at the end is created with the Level-of-Detail structure. A _.lod_ is ready to be loaded as any other files without the need to recreate the multi-resolution structure.

At the present moment a _Kd-Tree_ is used to merge the splats and create the resolution levels, but a better and more robust version with a _Point Octree_ is under development and will soon be incorporated to the system.

**Note** that the code with the _Kd-Tree_ is a very weak implementation of the multi-resolution structure and doesn't work for all models. It was quickly implemented just to test the LOD code using the geometry shader. This issues will be corrected with the new version.

## Ellipse Rasterization ##

This mode is still at an early construction stage. More info about this will be available soon.