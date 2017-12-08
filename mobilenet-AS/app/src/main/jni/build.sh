rm -rf build && mkdir build
cd build

cmake .. \
-DANDROID_ABI=armeabi-v7a \
-DANDROID_STANDALONE_TOOLCHAIN=/home/nlp/chenhoujiang/android-toolchain \
-DPADDLE_ROOT=/home/nlp/chenhoujiang/Paddle/build_android/install \
-DCMAKE_BUILD_TYPE=MinSizeRel \
-DBUILD_SHARED_LIBS=ON

make

#-DCMAKE_TOOLCHAIN_FILE=../android.toolchain.cmake \
