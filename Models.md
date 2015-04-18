# Introduction #

With the inclusion of the VCG library the old ply library was removed and any file accepted by the VCG import method can be used.

# Details #

The accepted files must have two more attributes besides the vertex coordinates:

  * Normals
  * Radius

## Attributes ##

These three attributes compose a _surfel_:

### Surfel Coordinates ###

The 3D coordinates.

### Surfel Normals ###

Per vertex normal.

### Surfel Radius ###

The radius of a surfel corresponds to its sampling distance. This can be computed using the k-nearest neighbors of a sample.

### Precomputed Models ###

Some models with precomputed normals and radius can be found here under the **Downloads** tab, or follow the link to the project page under our lab's website.