#ifndef __JNI__
#ifndef __CLIB__
#include <cstdio>
#include "main.h"
#include "version.h"
#include "OcrLite.h"
#include "OcrUtils.h"
#ifdef _WIN32
#include <windows.h>
#endif

void printHelp(FILE *out, char *argv0) {
    fprintf(out, " ------- Usage -------\n");
    fprintf(out, "%s %s", argv0, usageMsg);
    fprintf(out, " ------- Required Parameters -------\n");
    fprintf(out, "%s", requiredMsg);
    fprintf(out, " ------- Optional Parameters -------\n");
    fprintf(out, "%s", optionalMsg);
    fprintf(out, " ------- Other Parameters -------\n");
    fprintf(out, "%s", otherMsg);
    fprintf(out, " ------- Examples -------\n");
    fprintf(out, example1Msg, argv0);
    fprintf(out, example2Msg, argv0);
}

int main(int argc, char **argv) {
    if (argc <= 1) {
        printHelp(stderr, argv[0]);
        return -1;
    }
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::string modelsDir, modelDetPath, modelClsPath, modelRecPath, keysPath;
    std::string imgPath;
    int numThread = 4;
    int padding = 50;
    int maxSideLen = 1024;
    float boxScoreThresh = 0.6f;
    float boxThresh = 0.3f;
    float unClipRatio = 2.0f;
    bool doAngle = true;
    int flagDoAngle = 1;
    bool mostAngle = true;
    int flagMostAngle = 1;

    int opt;
    int optionIndex = 0;
    while ((opt = getopt_long(argc, argv, "d:1:2:3:4:i:t:p:s:b:o:u:a:A:v:h", long_options, &optionIndex)) != -1) {
        switch (opt) {
            case 'd':
                modelsDir = optarg;
                break;
            case '1':
                modelDetPath = modelsDir + "/" + optarg;
                break;
            case '2':
                modelClsPath = modelsDir + "/" + optarg;
                break;
            case '3':
                modelRecPath = modelsDir + "/" + optarg;
                break;
            case '4':
                keysPath = modelsDir + "/" + optarg;
                break;
            case 'i':
                imgPath.assign(optarg);
                break;
            case 't':
                numThread = (int) strtol(optarg, NULL, 10);
                break;
            case 'p':
                padding = (int) strtol(optarg, NULL, 10);
                break;
            case 's':
                maxSideLen = (int) strtol(optarg, NULL, 10);
                break;
            case 'b':
                boxScoreThresh = strtof(optarg, NULL);
                break;
            case 'o':
                boxThresh = strtof(optarg, NULL);
                break;
            case 'u':
                unClipRatio = strtof(optarg, NULL);
                break;
            case 'a':
                flagDoAngle = (int) strtol(optarg, NULL, 10);
                if (flagDoAngle == 0) {
                    doAngle = false;
                } else {
                    doAngle = true;
                }
                break;
            case 'A':
                flagMostAngle = (int) strtol(optarg, NULL, 10);
                if (flagMostAngle == 0) {
                    mostAngle = false;
                } else {
                    mostAngle = true;
                }
                break;
            case 'v':
                printf("%s\n", VERSION);
                return 0;
            case 'h':
                printHelp(stdout, argv[0]);
                return 0;
            default:
                printf("other option %c: %s\n", opt, optarg);
        }
    }
    if (modelDetPath.empty()) {
        modelDetPath = modelsDir + "/" + "dbnet.onnx";
    }
    if (modelClsPath.empty()) {
        modelClsPath = modelsDir + "/" + "angle_net.onnx";
    }
    if (modelRecPath.empty()) {
        modelRecPath = modelsDir + "/" + "crnn_lite_lstm.onnx";
    }
    if (keysPath.empty()) {
        keysPath = modelsDir + "/" + "keys.txt";
    }
    if (!isFileExists(imgPath)) {
        fprintf(stderr, "Target image not found: %s\n", imgPath.c_str());
        return -1;
    }
    if (!isFileExists(modelDetPath)) {
        fprintf(stderr, "Model dbnet file not found: %s\n", modelDetPath.c_str());
        return -1;
    }
    if (!isFileExists(modelClsPath)) {
        fprintf(stderr, "Model angle file not found: %s\n", modelClsPath.c_str());
        return -1;
    }
    if (!isFileExists(modelRecPath)) {
        fprintf(stderr, "Model crnn file not found: %s\n", modelRecPath.c_str());
        return -1;
    }
    if (!isFileExists(keysPath)) {
        fprintf(stderr, "keys file not found: %s\n", keysPath.c_str());
        return -1;
    }
    OcrLite ocrLite;
    ocrLite.setNumThread(numThread);
    ocrLite.initModels(modelDetPath, modelClsPath, modelRecPath, keysPath);
    ocrLite.detect(imgPath.c_str(), padding, maxSideLen,
                   boxScoreThresh, boxThresh, unClipRatio, doAngle, mostAngle);
    return 0;
}

#endif
#endif
