# xstep
Mixture prorgamming with CUDA and C99

A very simple example with  cuda_kmeans

// Copyright (c) 2005 Wei-keng Liao
// Copyright (c) 2011 Serban Giuroiu

Notice: different version of nvidia cuda nvcc compiles generate with different library name symbols.

anyone who use fresh hardware or nvcc version above or equ nvcc-8. remove Makefile of "arch=compute_20,code=sm_20"

submodule needed
https://github.com/concurrencykit/ck


FFmpeg:
   --enable-shared when build

opencv-3.4.3
in sample/gpu/multi.cpp
  Comment this:
   -  //===MMMM==== tbb::parallel_do(devices, devices + 2, Worker());

opencv-3.4.3-cpu-only:
cmake -D CMAKE_BUILD_TYPE=RELEASE \
      -D CMAKE_C_COMPILER=clang-6.0 \
      -D CMAKE_CXX_COMPILER=clang++-6.0 \
      -D CMAKE_INSTALL_PREFIX=/usr/local \
      -D WITH_CUDA=OFF \
      -D WITH_TBB=ON \
      -D WITH_IPP=ON \
      -D WITH_V4L=ON \
      -DWITH_OPENGL=ON \
      -DWITH_OPENCL=OFF \
      -DWITH_QT=ON \
      -DFORCE_VTK=ON \
      -D WITH_EIGEN=ON \
      -DWITH_XINE=ON \
      -DWITH_GDAL=ON \
      -DWITH_1394=ON \
      -DWITH_FFMPEG=OFF \
      -D BUILD_PROTOBUF=ON \
      -D WITH_PROTOBUF=ON \
      -D ENABLE_FAST_MATH=1 \    
      -D CUDA_FAST_MATH=0 \
      -D HAVE_VIDEOIO=1 \
      -D WITH_CUBLAS=OFF \
      -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-3.4.3/modules \
      -D INSTALL_PYTHON_EXAMPLES=ON \
      -D INSTALL_C_EXAMPLES=ON \
      -D BUILD_EXAMPLES=ON ..

opencv-3.4.3-nvidia9.2
cmake -D CMAKE_BUILD_TYPE=RELEASE \
      -D CMAKE_INSTALL_PREFIX=/usr/local \
      -D WITH_CUDA=ON \
      -D CUDA_GENERATION=Pascal \
      -D CUDA_FAST_MATH=1 \
      -D WITH_CUBLAS=1 \
      -D WITH_TBB=ON \
      -D WITH_IPP=ON \
      -D WITH_V4L=ON \
      -DWITH_OPENGL=ON \
      -DWITH_OPENCL=OFF \
      -DWITH_QT=ON \
      -DFORCE_VTK=ON \
      -D WITH_EIGEN=ON \
      -DWITH_XINE=ON \
      -DWITH_GDAL=ON \
      -DWITH_1394=ON \
      -DWITH_FFMPEG=OFF \
      -D HAVE_VIDEOIO=1 \
      -D BUILD_PROTOBUF=ON \
      -D WITH_PROTOBUF=ON \
      -D ENABLE_FAST_MATH=1 \    
      -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-3.4.3/modules \
      -D INSTALL_PYTHON_EXAMPLES=ON \
      -D INSTALL_C_EXAMPLES=OFF \
      -D BUILD_EXAMPLES=ON ..
