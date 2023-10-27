# tensor

#Compilation
gcc -Wall -shared -fPIC -O3 -fstack-protector-strong $(python3-config --includes) src/tensor.c -o tensor.so
