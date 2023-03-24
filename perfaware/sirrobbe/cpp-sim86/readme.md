# Sim86

## Requirements

- Python 3.11
- Visual Studio 2022 with Visual C++
- Clang

On your first setup create a virtualenv called venv in the ***cpp-sim86*** directory.
After that run:

`shell.bat`

To install all required python packages in your virtual environment call:

`pip install -r requirements.txt`

## Working on the project

When you start to work in the project open up your console and execute:

`shell.bat`

After that everything is initalized.
If you want to build the simulator, you can run:

`build.bat`

For rapid testing you can also build your simulator and run it on all listings by executing:

`python test.py`

If you add any new packages to the venv don't forget to run:

`pip freeze > requirements.txt`

Check that the new ***requirements.txt*** is not recognized as a binary file.
This sometimes happens on Windows.
