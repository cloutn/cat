# cat
cat is a simple vulkan renderer with gltf supported.

## How to run
1. Microsoft Windows 10 or higher, nvidia GTX 770 or higher.
2. Setup Visual Studio 2022 with msvc++ tools.
3. Run setup.bat
4. Open testCat\build64\testCat.sln, press F5

if you need 32 bit version, open setup.bat, change line:
	"./python/python" "script/setup.py" all
to:
	"./python/python" "script/setup.py" all32
run setup.bat again.

## Features
Load and display gltf model.
VertexShader skinned-mesh.
Imgui embeded.
