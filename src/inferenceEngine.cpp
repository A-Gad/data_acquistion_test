#include "inferenceEngine.h"
#include <iostream>
#include <algorithm>

InferenceEngine::InferenceEngine() = default;
InferenceEngine::~InferenceEngine() = default;

bool InferenceEngine::loadModel(const std::string& modelPath) {
    // 1. Build model from file
    model_ = tflite::FlatBufferModel::BuildFromFile(modelPath.c_str());
    if (!model_) {
        std::cerr << "[InferenceEngine] Failed to load model from: " << modelPath << std::endl;
        return false;
    }

    // 2. Construct the interpreter
    tflite::InterpreterBuilder builder(*model_, resolver_);
    if (builder(&interpreter_) != kTfLiteOk || !interpreter_) {
        std::cerr << "[InferenceEngine] Failed to construct TFLite interpreter." << std::endl;
        return false;
    }

    // 3. Allocate tensor buffers
    if (interpreter_->AllocateTensors() != kTfLiteOk) {
        std::cerr << "[InferenceEngine] Failed to allocate tensors." << std::endl;
        return false;
    }

    // 4. Cache input tensor details
    if (interpreter_->inputs().empty()) {
        std::cerr << "[InferenceEngine] Model does not define any input tensors.\n";
        return false;
    }
    int inputTensorIndex = interpreter_->inputs()[0];
    TfLiteTensor* inputTensor = interpreter_->tensor(inputTensorIndex);
    inputInfo_.name = inputTensor->name ? inputTensor->name : "";
    inputInfo_.type = inputTensor->type;
    inputInfo_.shape.clear();
    for (int i = 0; i < inputTensor->dims->size; ++i) {
        inputInfo_.shape.push_back(inputTensor->dims->data[i]);
    }

    // 5. Cache output tensor details
    if (interpreter_->outputs().empty()) {
        std::cerr << "[InferenceEngine] Model does not define any output tensors.\n";
        return false;
    }
    int outputTensorIndex = interpreter_->outputs()[0];
    TfLiteTensor* outputTensor = interpreter_->tensor(outputTensorIndex);
    outputInfo_.name = outputTensor->name ? outputTensor->name : "";
    outputInfo_.type = outputTensor->type;
    outputInfo_.shape.clear();
    for (int i = 0; i < outputTensor->dims->size; ++i) {
        outputInfo_.shape.push_back(outputTensor->dims->data[i]);
    }

    isInitialized_ = true;
    std::cout << "[InferenceEngine] Loaded model " << modelPath << " successfully.\n";
    return true;
}

bool InferenceEngine::runInference(const std::vector<float>& inputData, std::vector<float>& outputData) {
    if (!isInitialized_) {
        std::cerr << "[InferenceEngine] Engine not initialized. Load model first.\n";
        return false;
    }

    // Calculate total expected input size
    size_t expectedInputSize = 1;
    for (int dim : inputInfo_.shape) {
        expectedInputSize *= dim;
    }

    if (inputData.size() != expectedInputSize) {
        std::cerr << "[InferenceEngine] Input size mismatch. Expected " << expectedInputSize 
                  << " elements, but got " << inputData.size() << ".\n";
        return false;
    }

    // Get input tensor float buffer
    float* inputBuffer = interpreter_->typed_input_tensor<float>(0);
    if (!inputBuffer) {
        std::cerr << "[InferenceEngine] Failed to get input tensor buffer.\n";
        return false;
    }
    
    // Copy input data into interpreter
    std::copy(inputData.begin(), inputData.end(), inputBuffer);

    // Invoke model
    if (interpreter_->Invoke() != kTfLiteOk) {
        std::cerr << "[InferenceEngine] Failed to invoke interpreter.\n";
        return false;
    }

    // Calculate total expected output size
    size_t expectedOutputSize = 1;
    for (int dim : outputInfo_.shape) {
        expectedOutputSize *= dim;
    }
    outputData.resize(expectedOutputSize);

    // Get output tensor float buffer
    const float* outputBuffer = interpreter_->typed_output_tensor<float>(0);
    if (!outputBuffer) {
        std::cerr << "[InferenceEngine] Failed to get output tensor buffer.\n";
        return false;
    }
    
    // Copy output predictions
    std::copy(outputBuffer, outputBuffer + expectedOutputSize, outputData.begin());

    return true;
}

void InferenceEngine::printModelDetails() const {
    if (!isInitialized_) {
        std::cout << "[InferenceEngine] No model loaded.\n";
        return;
    }

    std::cout << "\n================= Model Summary =================\n";
    std::cout << "Input Tensor:\n";
    std::cout << "  Name:  " << inputInfo_.name << "\n";
    std::cout << "  Type:  " << TfLiteTypeGetName(inputInfo_.type) << "\n";
    std::cout << "  Shape: [";
    for (size_t i = 0; i < inputInfo_.shape.size(); ++i) {
        std::cout << inputInfo_.shape[i] << (i + 1 < inputInfo_.shape.size() ? ", " : "");
    }
    std::cout << "]\n\n";

    std::cout << "Output Tensor:\n";
    std::cout << "  Name:  " << outputInfo_.name << "\n";
    std::cout << "  Type:  " << TfLiteTypeGetName(outputInfo_.type) << "\n";
    std::cout << "  Shape: [";
    for (size_t i = 0; i < outputInfo_.shape.size(); ++i) {
        std::cout << outputInfo_.shape[i] << (i + 1 < outputInfo_.shape.size() ? ", " : "");
    }
    std::cout << "]\n";
    std::cout << "=================================================\n\n";
}
