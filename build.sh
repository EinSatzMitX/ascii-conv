#g++ -std=c++11 -I/usr/include/opencv4 -o ascii-conv main.cpp -L/usr/lib/x86_64-linux-gnu 
g++ -std=c++11 `pkg-config --cflags opencv4` -o ascii-conv main.cpp `pkg-config --libs opencv4`

./ascii-conv -f test_vid.mp4
