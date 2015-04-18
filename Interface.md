# Introduction #

I removed the Qt interface since this renderer will soon be integrated with Meshlab, thus users wanting to test with GUI will be able to do so from there.
However, I left a basic GLUT interface so the rendered can also be used independently from Meshlab, note that the VCG lib is still used in this case, but there is not need to have Qt installed.

# Details #

## Renderers ##

F1 key : original renderer

F2 key : original renderer with color per vertex (if available from ply file)

F3 key : template version renderer with color per vertex (if available from ply file)

## Colors ##

0 .. 4 key: switch between predefined material types.

5 key : no material

7 key : normal map

## Mask Size (Template Version) ##

The mask size can be modified with the - and + keys.

## Filter Sizes ##

The reconstruction filter is a global size modifier for all ellipses, thus increasing it will enlarge all ellipses proportionally.

[ key : decrease

] key : increase

## Depth Test ##

d key : depth test _on/off_.

## Back Face Culling ##

b key : back face culling _on/off_.

## Mouse ##

left+Drag : rotate

left+Ctrl+Drag : Pan

left+Shift+Drag : Zoom

wheel : Zoom

Shift+wheel : Change perspective

Ctrl+wheel : Move far clipping plane

Ctrl+shift+wheel : Move near clipping plane

right : Change light direction