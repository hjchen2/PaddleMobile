#include <android/bitmap.h>
#include <android/log.h>

#include <jni.h>

#include <string>
#include <vector>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#include "paddle/capi.h"


static struct timeval tv_begin;
static struct timeval tv_end;
static double elasped;

static void bench_start() {
    gettimeofday(&tv_begin, NULL);
}

static void bench_end(const char* comment) {
    gettimeofday(&tv_end, NULL);
    elasped = (tv_end.tv_sec - tv_begin.tv_sec) * 1000.0f + 
                    (tv_end.tv_usec - tv_begin.tv_usec) / 1000.0f;
//     fprintf(stderr, "%.2fms   %s\n", elasped, comment);
    __android_log_print(ANDROID_LOG_DEBUG, "MobileNet",
                    "%.2fms   %s", elasped, comment);
}

static std::vector<unsigned char> mobilenet_bin;
static std::vector<std::string> mobilenet_words;
static paddle_gradient_machine machine;

static std::vector<std::string> split_string(const std::string& str,
                                             const std::string& delimiter) {
    std::vector<std::string> strings;
    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = str.find(delimiter, prev)) != std::string::npos) {
        strings.push_back(str.substr(prev, pos - prev));
        prev = pos + 1;
    }
    // To get the last substring (or only, if delimiter is not found)
    strings.push_back(str.substr(prev));
    return strings;
}

extern "C" {

// public native boolean Init(byte[] param, byte[] bin, byte[] words);
JNIEXPORT jboolean JNICALL Java_com_paddle_mobilenet_MobileNet_Init(JNIEnv* env,
        jobject thiz, jbyteArray bin, jbyteArray words) {
    // init paddle
    if (paddle_init(0 , NULL) != kPD_NO_ERROR) {
        __android_log_print(ANDROID_LOG_ERROR, "MobileNet", "init paddle failed");
    }

    // init model
    int len = env->GetArrayLength(bin);
    mobilenet_bin.resize(len);
    env->GetByteArrayRegion(bin, 0, len, (jbyte*)mobilenet_bin.data());
    paddle_error ret = paddle_gradient_machine_create_for_inference_with_parameters(
            &machine, mobilenet_bin.data(), len);
    __android_log_print(ANDROID_LOG_DEBUG, "MobileNet", "load_model %d %d", ret, len);

    // init words
    len = env->GetArrayLength(words);
    std::string words_buffer;
    words_buffer.resize(len);
    env->GetByteArrayRegion(words, 0, len, (jbyte*)words_buffer.data());
    mobilenet_words = split_string(words_buffer, "\n");

    return JNI_TRUE;
}

// public native String Detect(Bitmap bitmap);
JNIEXPORT jstring JNICALL Java_com_paddle_mobilenet_MobileNet_Detect(JNIEnv* env,
        jobject thiz, jobject bitmap) {
    // start to timing
    bench_start();

    int channel = 3;
    int width = 0;
    int height = 0;
    paddle_arguments in_args = paddle_arguments_create_none();
    paddle_arguments_resize(in_args, 1);
    paddle_matrix mat;
    paddle_real* array;
    {
        AndroidBitmapInfo info;
        AndroidBitmap_getInfo(env, bitmap, &info);
        width = info.width;
        height = info.height;
        if (width != 224 || height != 224) {
            __android_log_print(ANDROID_LOG_ERROR, "MobileNet",
                    "width is %d, height is %d", width, height);
            return NULL;
        }
        if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
            return NULL;

        void* indata;
        AndroidBitmap_lockPixels(env, bitmap, &indata);

        size_t input_size = 3 * width * height;
        mat = paddle_matrix_create(1, input_size, false);
        paddle_matrix_get_row(mat, 0, &array);
        paddle_real* array_a = array;
        paddle_real* array_b = array + width * height;
        paddle_real* array_c = array + 2 * width * height;
        unsigned char* indata_t = (unsigned char*)indata;
        // RGBA --> RGB
        const float mean_vals[3] = {103.94, 116.78, 123.68};
        const float norm_vals[3] = {0.017, 0.017, 0.017};
        for (int i = 0; i < width * height; ++i) {
            array_a[i] = (indata_t[2] - mean_vals[0]) * norm_vals[0];
            array_b[i] = (indata_t[1] - mean_vals[1]) * norm_vals[1];
            array_c[i] = (indata_t[0] - mean_vals[2]) * norm_vals[2];
            indata_t += 4;
        }
        AndroidBitmap_unlockPixels(env, bitmap);
    }

    size_t out_size = 0;
    paddle_error ret = kPD_NO_ERROR;
    {
        paddle_arguments_set_value(in_args, 0, mat);
        paddle_arguments out_args = paddle_arguments_create_none();
        if (paddle_gradient_machine_forward(machine, in_args, out_args, false)
                != kPD_NO_ERROR) {
            __android_log_print(ANDROID_LOG_ERROR, "MobileNet", "inference failed");
        }

        paddle_matrix out_mat = paddle_matrix_create_none();
        ret = paddle_arguments_get_value(out_args, 0, out_mat);
        __android_log_print(ANDROID_LOG_DEBUG, "MobileNet",
                "eaddle_arguments_get_value return %d", ret);

        uint64_t out_height = 0;
        uint64_t out_width = 0;
        ret = paddle_matrix_get_shape(out_mat, &out_height, &out_width);
        __android_log_print(ANDROID_LOG_DEBUG, "MobileNet",
                "output height %llu, width %llu, ret %d", out_height, out_width, ret);

        out_size = out_height * out_width;
        paddle_matrix_get_row(out_mat, 0, &array);
    }

    // return top class
    int top_class = 0;
    float max_score = 0.f;
    for (size_t i = 0; i < out_size; i++) {
        float s = array[i];
        if (s > max_score) {
            top_class = i;
            max_score = s;
        }
    }

    const std::string& word = mobilenet_words[top_class];
    char tmp[32];
    sprintf(tmp, "%.3f", max_score);
    std::string result_str = std::string(word.c_str() + 10) + " = " + tmp;
    // +10 to skip leading n03179701
    jstring result = env->NewStringUTF(result_str.c_str());

    bench_end("detect");

    return result;
}

}
