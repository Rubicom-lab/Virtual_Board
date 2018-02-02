# Virtual Board v 1.1.2

## What is Virtual Board?
Soongsil Univ., Department of Smart Systems Software, Rubicom Laboratory Virtual Machine Project.

This is a project to simulate an arm 32-bit program on a computer with x86 64-bit CPU architecture.
<br/><br/>

Advisor: Kim Kang-hee.

1st Contributor: Lee Du-Sol, Lee Hyo-Eun, Choi Kyu-Jin.

2nd Contributor: Woo Hyun, Cho Sung-Jae.

## Features
#### BitsHandler.h
* logical shift left
* logical shift right
* arithmetic shift right
* rotate right

#### DataProcessing.h
* mov
* add
* sub
* cmp
* mul
* bx

#### MultiDataTransfer.h
* push
* pop

#### SingleDataTransfer.h
* str immediate
* ldr immediate
* str global data
* ldr global data

#### VirtualMachine.h
* virtual cpu allocate
* virtual cpu delete
* virtual cpu set pc
* virtual cpu load
* fetch
* decode
* virtual cpu run (exec)

## Build
```
make clean
make
```
CMakeLists.txt are provided: you can use cmake/CLion.

## Usage
```
./Main bubbleSort
```
The "bubbleSort" is an executable file that is compiled on raspberry pi 2(ARM v7).

You need a file compiled on arm 32-bit computer.

## License

