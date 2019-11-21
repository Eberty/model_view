# Model view

## **Overview**

The purpose of this program is to display a textured 3d model produced from the combination of structure-from-motion (SFM), multi-view-stereo (MVS) and deep camera capture techniques.

The images with respective poses used by the SFM system will not be able to apply texturing to the bottom of the object as this view will be hidden when object is not on air. To omit this effect, this program mainly aims to post-apply a second texture with a photo of the bottom view of the artifact.

This program was written using the openscenegraph library. It reads a mesh file and add it into a scene. The program uses a MLP file to get the image position and orientation making a projective texture and apply it into the model.

**Keywords:** Viewer, projective texture mapping, color adjustment in image-based texture map.

<br />

### **License**

The source code is released under a [MIT](https://en.wikipedia.org/wiki/MIT_License) license.

**Author:** Eberty Alves da Silva<br />
**Affiliation:** Universidade Federal da Bahia - UFBA<br />
**Maintainer:** Eberty Alves da Silva, eberty.silva@hotmail.com

The model_view package has been tested under *Ubuntu 16.04 LTS* and *Ubuntu 19.10*.

## **Installation**

#### Dependencies

First update the package manager indexes like this:

`sudo apt update`

Then, install these packages:

`sudo apt install build-essential cmake git libopenscenegraph-dev openscenegraph -y`

#### Building

After instalation of the dependencies, run the following commands:

```sh
git clone https://github.com/Eberty/model_view.git
cd model_view/
mkdir build && cd build
cmake ..
cd ..
make -C ./build/
```

## **Usage**

You can run the code with:

```sh
./model_view <mesh_file.obj> <meshlab_script.mlp>
```

There are some examples of meshes and meshlab project files in the *models* folder, use them at will.

#### **MLP files**

To produce *mpl* files for this project is recommended install the meshlab:

`sudo apt install meshlab`

After, you can produce the file in the following way:

1. Open meshlab
2. Select **File** > **Import mesh** > "Choose a mesh file"
3. Select **File** > **Import Raster...** > "Choose a image file"
4. Select **Show Current Raster Mode** on menu
    * Press **Ctrl + H** to start from a initial point of view
    * You can use **Mouse right button** to rotate, **Ctrl + Mouse right button** to move and **Shift + Mouse right button** to scale
    * Find a good alignment of an image with respect to the 3d model
    * If in doubt, see: [Raster Layers: Set Raster Camera](https://www.youtube.com/watch?v=298OJABhkYs) and [Color Projection: Mutual Information, Basic](https://www.youtube.com/watch?v=Pv6_qFIr7gs)
5. Select **Filters** > **Camera** > **Set Raster Camera** > Click on **Get shot** and **Apply**
6. Select **File** > **Save Project** > "Choose a name and a folder and save the file"

Meshlab is not able to apply a second texture correctly over an existing one that not have camera (raster) calibration, so we are only use it to get the camera position in the world.  

Please, make sure that there is only one *MLRaster* on *mlp* file.

## **Bugs & Feature Requests**

Please report bugs and request features using the [Issue Tracker](https://github.com/Eberty/model_view/issues).