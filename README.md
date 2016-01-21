# glExamples

This repository contains some OpenGL examples.
These examples have been written in C++ and use OpenGL 3.3.

**These are not tutorials**. While I have tried to document the code
as much as possible, the why and how is not explained. If you want a
tutorial on OpenGL, I suggest you try out [open.gl](http://open.gl) or
[learn opengl](http://learnopengl.com).  
The point of these examples is not to teach you how 3D rendering works.
These examples merely serve as a point of reference as to how you can
implement solutions to typical problems in OpenGL 3.3.

Each example resides in its own folder. Code that is often reused, such as
code for compiling and linking shader programs, is put into a seperate folder and reused in the different examples.

This code has been written with OS X as a target. No special care has been
taken to make sure the code is cross-platform. While the examples might
not compile out of the box on your machine, the code can still be reused in
your projects. You should be able to just plug it into an OpenGL application
that compiles and runs on your machine.

## Dependencies

These examples have some dependencies:

* [GLFW](http://www.glfw.org)
* [SOIL](http://www.lonesock.net/soil.html)
* [GLM](http://glm.g-truc.net)
* [ASSIMP](http://assimp.sf.net/)

## Compiling

### OS X

Make sure that you have compiled the GLFW dependency for your system as a
static library. You should now have a file called `libglfw3.a`. Place this
in a subdirectory `lib` of the root folder (on the same level as this `README.md` file).

Download the latest release of SOIL. You can try putting the binary in the `lib` folder, but this
will most likely fail if your computer is from the last few years. You're best off using the supplied
makefile in the `projects/makefile` folder. Note that you need to alter a line if your OS is 64 bit:
change the line that says `CXX = gcc` to `CXX = gcc -arch x86_64`. Run `make` and you will find the new
`libSOIL.a` file in the `lib` subdirectory of the directory where you downloaded SOIL to.

Download the latest stable release of ASSIMP. You can find this release on [their GitHub repository page](https://github.com/assimp/assimp). Generate build files and compile the static library. Put the resulting `libassimp.a` file in the `lib` folder where you put the other library files.

Now you can use the command `make $EXAMPLE_NAME` (e.g., `make hdr`) to compile an example. The
resulting binaries will be placed in the `bin` folder.

## License

These examples are available under the MIT License. This is because public
domain is not recognized in every country. For more information, read
[this question and answer](http://programmers.stackexchange.com/questions/147111/what-is-wrong-with-the-unlicense).

Licenses of third party libraries are available in the [3rd\_party](3rd_party) folder.

## Examples

To compile all examples, you can run `make` or `make all`.

1. Beginner
    1. [Hello Triangle](#hello-triangle)
    2. [Hello Sprite](#hello-sprite)
    3. [Hello Cube](#hello-cube)
    4. [Hello Heightmap](#hello-heightmap)
    5. [Hello Mesh](#hello-mesh)
    6. [Transparency](#transparency)
2. Intermediate
    1. [Render To Texture](#render-to-texture)
    2. [Cubemaps](#cubemaps)
    3. [Instancing](#instancing)
    4. [Particles](#particles)
    5. [Sprite Batching](#sprite-batching)
3. Advanced
    1. [Morph Target Animation](#morph-target-animation)
    2. [Uniform Buffer Objects](#uniform-buffer-objects)
    3. [Forward Rendering](#forward-rendering)
    4. [Shadows](#shadows)
    5. [Billboards](#billboards)
4. Expert
    1. [Deferred Shading](#deferred-shading)
    2. [HDR Deferred Shading](#hdr)
    3. [Additive Light Passes](#additive-light-passes)

### Hello Triangle

*Level: beginner*

**Compile**: `make hello_triangle`  
**Run**: `cd bin; ./01-hello-triangle.out`

This example shows the minimal code needed to render a simple colored triangle.

[Code](src/examples/01-hello_triangle)

![Screenshot](img/01-hello_triangle.tiff)

### Hello Sprite

*Level: beginner*

**Compile**: `make hello_sprite`  
**Run**: `cd bin; ./02-hello_sprite.out`

This example shows the minimal code needed to render a textured quad. It also uses
`glDrawElements` to reduce duplication of vertices.

[Code](src/examples/02-hello_sprite)

![Screenshot](img/02-hello_sprite.tiff)

### Hello Cube

*Level: beginner*

**Compile**: `make hello_cube`  
**Run**: `cd bin; ./03-hello_cube.out`

This example introduces 3D transformations and perspective projection. It also
uses depth testing to make sure the cube is rendered correctly.

[Code](src/examples/03-hello_cube)

![Screenshot](img/03-hello_cube.tiff)

### Hello Heightmap

*Level: beginner*

**Compile**: `make hello_heightmap`  
**Run**: `cd bin; ./04-hello_heightmap.out`

This example introduces the loading and rendering of heightmaps. It also introduces
smooth camera movement and looking around with the mouse. The heightmap is rendered without
color or texture and in wireframe. Adding color and texture is a good exercise for the beginning
reader.
Finally, we also introduce backface culling.

[Code](src/examples/04-hello_heightmap)

![Screenshot](img/04-hello_heightmap.tiff)

### Hello Mesh

*Level: beginner*

**Compile**: `make hello_mesh`  
**Run**: `cd bin; ./05-hello_mesh.out`

This example uses Assimp to load a mesh. We load the material ourselves instead of using Assimp.
In this example we have moved the shader loading to a `Material` class and the data loading to
a `Mesh` class. We will be making changes to these classes in later examples if needed.

[Code](src/examples/05-hello_mesh)

![Screenshot](img/05-hello_mesh.tiff)

### Transparency

*Level: beginner*

**Compile**: `make transparency`  
**Run**: `cd bin; ./17-transparency.out`

This examples shows how to use blending to render transparent objects, by sorting them back-to-front.
This is just one way to render transparent objects.

[Code](src/examples/17-transparency)

![Screenshot](img/17-transparency.tiff)

### Render To Texture

*Level: intermediate*

**Compile**: `make render_to_texture`  
**Run**: `cd bin; ./06-render_to_texture.out`

We reuse the code from the [Hello Mesh](#hello-mesh) example, but this time we render to an
off-screen texture, and then render that texture in grayscale to the screen.

[Code](src/examples/06-render_to_texture)

![Screenshot](img/06-render_to_texture.tiff)

### Cubemaps

*Level: intermediate*

**Compile**: `make cubemaps`  
**Run**: `cd bin; ./07-cubemaps.out`

This example shows how to create a skybox with cubemaps. It demonstrates loading 6 images
into a cubemap, and it demonstrates how to draw the skybox behind everything else.

[Code](src/examples/07-cubemaps)

![Screenshot](img/07-cubemaps.tiff)

### Instancing

*Level: intermediate*

**Compile**: `make instancing`  
**Run**: `cd bin; ./08-instancing.out`

This example draws a large number of objects in a single draw call using instancing.
Model matrices are generated in the main file, but most of the actual implementation
is in the `Mesh` class. Make sure to look at the `setInstances` and `render` methods.

[Code](src/examples/08-instancing)

![Screenshot](img/08-instancing.tiff)

### Particles

*Level: intermediate*

**Compile**: `make particles`  
**Run**: `cd bin; ./09-particles.out`

This example uses instanced rendering to render a large amount of particles. The particle
system shown is rather basic, only supporting colored particles and no textures. There are
also no special particle emitter shapes or particle collision. This would complicate the
example and take away from the understanding of simple particle rendering.

[Code](src/examples/09-particles)

![Screenshot](img/09-particles.tiff)

### Sprite Batching

*Level: intermediate*

**Compile**: `make sprite_batching`  
**Run**: `cd bin; ./10-sprite_batching.out`

This example shows how to batch draw calls for sprites. This allows you to render thousands
of sprites at a high framerate. This implementation keeps collecting sprites into a buffer
until the buffer is full or a different texture is encountered. Other implementations
sort the buffer by texture and then render the whole buffer. This one is the easiest to implement,
but has more draw calls in a worst case scenario. It does not have the overhead of sorting.

[Code](src/examples/10-sprite_batching)

![Screenshot](img/10-sprite_batching.tiff)

### Morph Target Animation

*Level: advanced*

**Compile**: `make morph_target_animation`  
**Run**: `cd bin; ./11-morph_target_animation.out`

Morph Target Animation is one of the two popular methods used to animate 3D objects.
It uses more memory than skeletal animation and is less flexible, but is very easy to implement
and can be used for complex animations such as facial animation. This example focuses on the implementation
of morph target animation and strips down any unnecessary components. It simply animates a growing and shrinking cube.  
The methods used in this example can easily be updated to accomodate different meshes, textures, normals, etc.

[Code](src/examples/11-morph_target_animation)

![Screenshot](img/11-morph_target_animation.tiff)

### Uniform Buffer Objects

*Level: advanced*

**Compile**: `make uniform_buffer_objects`  
**Run**: `cd bin; ./12-uniform_buffer_objects.out`

Uniform Buffer Objects allow you to reuse uniforms easily between shader programs without having to add a lot of extra code.
They also allow you to have much more uniforms. We render two cubes with different shader programs, reusing the
projection and view uniform matrices.

[Code](src/examples/12-uniform_buffer_objects)

![Screenshot](img/12-uniform_buffer_objects.tiff)

### Forward Rendering

*Level: advanced*

**Compile**: `make forward_rendering`  
**Run**: `cd bin; ./13-forward_rendering.out`

Forward Rendering is one popular technique of rendering lit 3D scenes. This example uses the Blinn-Phong shading model
to render a lit cube with several lights.

[Code](src/examples/13-forward_rendering)

![Screenshot](img/13-forward_rendering.tiff)

### Shadows

*Level: advanced*

**Compile**: `make shadows`  
**Run**: `cd bin; ./14-shadows.out`

This is a simple example of how to do shadow mapping with percentage closer filtering for easy dynamic shadows. It only
handles directional lights.

[Code](src/examples/14-shadows)

![Screenshot](img/14-shadows.tiff)

### Billboards

*Level: advanced*

**Compile**: `make billboards`  
**Run**: `cd bin; ./15-billboards.out`

This example shows how to render billboards (2D sprites that are always oriented towards the camera) with a fixed size, or a size that changes depending on the distance between the object and the camera.

[Code](src/examples/15-billboards)

![Screenshot](img/15-billboards.tiff)

### Deferred Shading

*Level: expert*

**Compile**: `make deferred_shading`  
**Run**: `cd bin; ./16-deferred_shading.out`

This example shows how to render to multiple textures in a single framebuffer at a time and use this to optimize the rendering of many lights. Other optimizations not included are tile based
deferred rendering, which can speed up rendering even more.

[Code](src/examples/16-deferred_shading)

![Screenshot](img/16-deferred_shading.tiff)

### HDR Deferred Shading

*Level: expert*

**Compile**: `make hdr`  
**Run**: `cd bin; ./18-hdr.out`

This example shows how to combine deferred shading with HDR rendering. We render the same asteroid field, but add a single very bright light. We can see that details are still visible
in the bright asteroids, as well as in the asteroids that are less lit in the background.

[Code](src/examples/18-hdr)

![Screenshot](img/18-hdr.tiff)

### Additive Light Passes

*Level: expert*

**Compile**: `make additive_lights`  
**Run**: `cd bin; ./19-additive_lights.out`

This example shows how to do additive blending with a pass for each light. This is similar to how the Doom 3 renderer works. The advantages of such an approach compared
to a simple forward rendering approach are simpler shaders and the possibility to reduce the number of fragments affected by a light using a stencil test.  
A disadvantage is that we need to do one pass per light, instead of one pass for all lights. This implies a lot of additional draw calls. However the complexity of a single pass can be greatly reduced.  
In this example, we also do a Z pre-pass to fill the depth buffer before drawing lights, and we do some
math to calculate the screen space bounding box of a light. This is why this example got the *expert* label, as additive light blending
is in itself actually quite simple to implement without these extra tricks.

[Code](src/examples/19-additive_lights)

![Screenshot](img/19-additive_lights.tiff)
