#include <gtest/gtest.h>
#include "inferenceEngine.h"
#include "test_data.h"
#include <vector>
#include <iterator>

class InferenceEngineTest : public ::testing::Test {
protected:
    InferenceEngine engine;
    const float ANOMALY_THRESHOLD = 0.05f;

    // No normalizer needed — test_data.h values are already normalized
    std::vector<float> normal_test_data{std::begin(NORMAL_TEST_DATA), std::end(NORMAL_TEST_DATA)};
    std::vector<float> abnormal_test_data{std::begin(ABNORMAL_TEST_DATA), std::end(ABNORMAL_TEST_DATA)};

    void SetUp() override {
        ASSERT_TRUE(engine.loadModel("../model/autoencoder_anomaly_detector.tflite"));
    }
};

TEST_F(InferenceEngineTest, ReconstructsNormalDataWithLowLoss)
{
    std::vector<float> reconstructed(TEST_DATA_LENGTH);
    engine.runInference(normal_test_data, reconstructed);

    float mse = 0.0f;
    for (int i = 0; i < TEST_DATA_LENGTH; ++i) {
        float diff = normal_test_data[i] - reconstructed[i];
        mse += diff * diff;
    }
    mse /= TEST_DATA_LENGTH;
    EXPECT_LT(mse, ANOMALY_THRESHOLD)
        << "Healthy signal triggered anomaly! MSE: " << mse;
}

TEST_F(InferenceEngineTest, FailsToReconstructAnomalyDetectingFault)
{
    std::vector<float> reconstructed(TEST_DATA_LENGTH);
    engine.runInference(abnormal_test_data, reconstructed);

    float mse = 0.0f;
    for (int i = 0; i < TEST_DATA_LENGTH; ++i) {
        float diff = abnormal_test_data[i] - reconstructed[i];
        mse += diff * diff;
    }
    mse /= TEST_DATA_LENGTH;
    EXPECT_GT(mse, ANOMALY_THRESHOLD)
        << "Model failed to flag fault! MSE: " << mse;
}