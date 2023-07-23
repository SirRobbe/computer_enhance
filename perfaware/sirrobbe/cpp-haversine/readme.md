# Haversine

## Requirements

- Visual Studio 2022 with Visual C++
- Clang

## Working on the project

When you start to work in the project open up your console and execute:

`shell.bat`

After that everything is initalized.
If you want to build the simulator, you can run:

`build.bat`

If you want to run the program you can use the following commands:

`build\haversine_clang_release.exe generate 10000000 4534642 uniform`

The first parameter is the amount of pairs that get generated. The second is a seed
for the rng and the third defines the generation mode. Either choose 'uniform' or 'cluster'.

`build\haversine_clang_release.exe compute`

This command computes the haversine distance for all the given pairs.