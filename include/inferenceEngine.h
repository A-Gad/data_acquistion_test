#ifndef INFERENCE_ENGINE_H
#define INFERENCE_ENGINE_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/core/c/common.h"

// Struct for keeping metadata of input/output tensors
struct ModelTensorInfo {
    std::vector<int> shape;
    TfLiteType type;
    std::string name;
};
struct MinMaxNormalizer {
    float dataMin;
    float dataMax;

    // Mirrors sklearn MinMaxScaler with feature_range=(0,1)
    float normalize(float x) const {
        if (dataMax == dataMin) return 0.0f;
        return (x - dataMin) / (dataMax - dataMin);
    }

    std::vector<float> normalizeWindow(const std::vector<float>& raw) const {
        std::vector<float> out;
        out.reserve(raw.size());
        for (float v : raw)
            out.push_back(normalize(v));
        return out;
    }
};
class InferenceEngine {
public:
    InferenceEngine();
    ~InferenceEngine();

    // Prevent copy constructor and assignment
    InferenceEngine(const InferenceEngine&) = delete;
    InferenceEngine& operator=(const InferenceEngine&) = delete;

    // Loads the model file and builds the TFLite interpreter
    bool loadModel(const std::string& modelPath);

    // Runs inference on a given float input vector and returns the output vector
    bool runInference(const std::vector<float>& inputData, std::vector<float>& outputData);

    // Getters for tensor metadata
    const ModelTensorInfo& getInputInfo() const { return inputInfo_; }
    const ModelTensorInfo& getOutputInfo() const { return outputInfo_; }

    // Helper to print debug information about the model inputs/outputs
    void printModelDetails() const;

private:
    std::unique_ptr<tflite::FlatBufferModel> model_;
    std::unique_ptr<tflite::Interpreter> interpreter_;
    tflite::ops::builtin::BuiltinOpResolver resolver_;

    ModelTensorInfo inputInfo_;
    ModelTensorInfo outputInfo_;
    
    bool isInitialized_ = false;
};

#endif // INFERENCE_ENGINE_H
