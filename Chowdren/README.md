This is Chowdren, the blazingly fast C++ runtime and recompiler for
Clickteam Fusion. It converts events and objects to native C++, resulting
in much faster applications.

To use it, run the following from this directory:

```python -m chowdren.run <exename> <outdir>```

See `python -m chwdren.run --help` for more extra options.

You will need CMake to generate the C++ makefile or projectfile, after
which you can use your native compilers to compile the application.