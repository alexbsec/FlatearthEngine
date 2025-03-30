#!/bin/bash

if [ ! -d "$(pwd)/bin/assets/shaders" ]; then
    mkdir -p "$(pwd)/bin/assets/shaders"
fi

echo "Compiling shaders..."

echo "assets/shaders/Builtin.ObjectShader.vert.glsl -> bin/assets/shaders/Builtin.ObjectShader.vert.spv"
glslc -fshader-stage=vert assets/shaders/Builtin.ObjectShader.vert.glsl -o bin/assets/shaders/Builtin.ObjectShader.vert.spv
if [ $? -ne 0 ]; then
    echo "Error compiling vertex shader"
    exit 1
fi

echo "assets/shaders/Builtin.ObjectShader.frag.glsl -> bin/assets/shaders/Builtin.ObjectShader.frag.spv"
glslc -fshader-stage=frag assets/shaders/Builtin.ObjectShader.frag.glsl -o bin/assets/shaders/Bultin.ObjectShader.frag.spv
if [ $? -ne 0 ]; then
    echo "Error compiling vertex shader"
    exit 1
fi

echo "Copying assets..."
cp -r assets bin/

echo "Done."
