# NGG
The Native Geometry Generator (NGG) is a private project that allows for chunk-based geometry generation using Unreal's UProceduralMeshComponent. 

I have implemented the Marching Cubes algorithm to allow surface editing. 
Individual chunks register themselves to the chunk manager and allow for edits that can affect multiple chunks at once.

A rough overview of the functionality can be seen in the video ![here.](https://github.com/V41U/NGG/blob/master/Reference/Version%200.0.1alpha.mp4)

## Current Features
- You can use individual chunks to create a map which create a surface using the UMath::Perlin noise function. 
- Editing of the surface using a spherical brush like:
![Blueprint Terrain Editing](https://github.com/V41U/NGG/blob/master/Reference/Example_blueprint_terrain_editing.PNG)
- An actor can call either _EditTerrain_ or _EditTerrainOVERRIDE_
  - _EditTerrain_ uses the chunk manager and allows for editing of terrain over multiple chunks
  - _EditTerrainOVERRIDE_ only adapts the individual chunk that is called. This may may be used, but I advise against it ;) 
- For each individual chunk one can define:
  - _Extent_: How large is the individual chunk
  - _Cube Resolution_: The size of the cube that is used in the marching cube algorithm
  - _Feature Size_: The size of the perlin noise features. A smaller value leads to more extreme differences which could represent a mountainous area. You do you :)
  - _Use Smooth Surface_: Boolean that defines whether a more exact surface representation should be done or whether the edges of the marching cube are always snapped to 0.5. 

## Open TODOs
- Expose the generation function so that users can overwrite this and have custom mesh generation (e.g. Underground tunnel systems)
- Expose VoxelData so that users can save the current version of chunks and the chunk state is not lost when destructing. 
- Bugfixes (e.g. Chunks are currently not unregistered from the chunk manager if they are destructed)
