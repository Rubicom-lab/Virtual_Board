cmake_minimum_required(VERSION 3.9)
project(Virtual_Board)

set(CMAKE_CXX_STANDARD 11)

add_executable(Virtual_Board
        BitsHandler.c
        Branch.c
        BubbleSort.c
        DataProcessing.c
        Debug.c
        Main.c
        MultiDataTransfer.c
        Parse.c
        Parse.h
        SingleDataTransfer.c
        VirtualMachine.c
        VirtualMachine.h SingleDataTransfer.h MultiDataTransfer.h BitsHandler.h DataProcessing.h Branch.h)
