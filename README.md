# drm

## This is a basic window manager for Linux 
### The window manager is based on Linux's DRM (on which most of the modern Window Managers are built).

## The repository mainly is structured into 2 components :
## Window server -
 Which the client applications will connect to and send request to. This is responsible for handling the main buffer coordination between multiple applications and sending the composited framebuffer to the GPU. It allocates a unique buffer to each application and the buffer is itself shared through shared memory mapped IPC. It also ports the keyboard and mice events to the application under context (the currently selected one). It supports moving the windows around the screen. 

## Client side -
 The clients are the applications which connect to the window manager. Relavant API exists for each Window Manager for the clients to send requests and commands to the window manager. A similar API exists in the client_lib.c, and all the client applications that have been built in this repository use this as a base.

## There are 4 applications built that use this Window Manager: 
### 1) Font Making App - client_font.c
### 2) Raytracing App (Which makes a sphere and does some basic Phong Shading) - client_render.c
### 3) Terminal Emulator (A GUI app which executes a shell (ex - /bin/bash) for an interactive session) - term_pty.c
### 4) Text Writing App - client.c

#### The font used by the text writing app was made by client_font.c and all the printable ASCII characters reside in font.h (8 * 8 = 64 binary values)

## To build the server
### Run
``` make server ```

## You can run
``` make client
 make terminal
 make render 

 for building the client library, and the applications

### OR simply run 
 make all 
to build everything
