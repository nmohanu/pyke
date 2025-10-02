# About 
Pyke is a highly optimized chess move generator / counter in progress. Pyke currently ranks as the second fastest chess move generator ever built.
<br>
Perft 7: 3195901860 nodes searched. 1600.46 MNPS on AMD 7900x.
<br>

# How to compile
From project root: 
<br>
mkdir build
<br>
cd build
<br>
cmake ..
<br>
make release
<br>
<br>
There should now be an executable in the build folder. 

# How to use
Currently, the program is hard coded from startposition on perft 7. You can change this in main.cpp. I'm working on features to change this in runtime.
