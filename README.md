# Animation
## Introduction
 This application involves rendering hierarchical 3D skeletal animation from BVH files. Animations blending is implemented, with a blending period of 0.5s.

 The character can perform 5 different animation cycles:
 - Standing (Default)
 - Walking
 - Sprinting
 - Veering Left
 - Veering Right

## Compilation

This application was made using Linux and QT 5.15.3, which can be downloaded at  
https://www.qt.io/download-dev  

If using a Windows machine, the Linux terminal can be accessed by using WSL
Learn more here: https://learn.microsoft.com/en-us/windows/wsl/

To compile the program, enter the following commands in a terminal:  
    
    qmake -project QT+=opengl LIBS+=-lGLU
    qmake
    make

## Usage
Run the program using `./Animation-Cycles`


### Controls
#### Camera
- `W` - move camera forward
- `S` - move camera backward
- `A` - move camera to the left
- `D` - move camera to the right
- `R` - move camera upwards
- `F` - move camera downwards
- `Q` - turn camera to the left
- `E` - turn camera to the right

#### Character
- `Up Arrow` makes the character run forward in a sprint
- `Control` makes the character walk forward
- `Left Arrow` and `Right Arrow` turn the character left and right respectively
- `Down Arrow` stops the character 

#### Other
- `P` resets the character's position
- `X` closes the application
