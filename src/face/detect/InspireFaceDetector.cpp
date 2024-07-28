//
// Created by LiangKeJin on 2024/7/18.
//

#include <cstdio>
#include <stdexcept>
#include "InspireFaceDetector.h"

#include <Playground.h>

static bool g_setup_face_model_flag = false;

static bool ginitInspireFace() {
    if (!g_setup_face_model_flag) {
        std::string assetPath = ASSERTS_PATH;
        assetPath += "/inspireface/pack/Megatron";
        HResult ret = HFLaunchInspireFace(assetPath.c_str());
        if (ret != HSUCCEED) {
            printf("HFLaunchInspireFace load resource error: %ld\n", ret);
            return false;
        }
        g_setup_face_model_flag = true;
    }
    return true;
}

void FaceData::setup(HFMultipleFaceData &data, int index) {
    m_index = index;
    m_track_id = data.trackIds[index];

    HFGetNumOfFaceDenseLandmark(&m_num_landmarks);
    HResult ret = HFGetFaceDenseLandmarkFromFaceToken(data.tokens[index], m_points, m_num_landmarks);
    if (ret != HSUCCEED) {
        printf("HFGetFaceDenseLandmarkFromFaceToken error: %lu\n", ret);
        m_num_landmarks = 0;
    }

    HFaceRect &rect = data.rects[index];
    m_x = rect.x;
    m_y = rect.y;
    m_width = rect.width;
    m_height = rect.height;

    m_pitch = data.angles.pitch[index];
    m_yaw = data.angles.yaw[index];
    m_roll = data.angles.roll[index];

    m_liveness_confidence = 0;
    m_face_mask_confidence = 0;
    m_quality_confidence = 0;
    m_left_eye_open_confidence = 0;
    m_right_eye_open_confidence = 0;
    m_attr_age = 0;
    m_attr_gender = 0;
    m_attr_age = 0;
}

void DetectResult::onFaceDetected(HFMultipleFaceData &data) {
    m_num_faces = data.detectedNum;
    for (int i = 0; i < m_num_faces; ++i) {
        m_face[i].setup(data, i);
    }
}

void DetectResult::setRGBLivenessConfidence(const HFRGBLivenessConfidence &confidence) {
    if (confidence.num != m_num_faces) {
        throw std::runtime_error("setRGBLivenessConfidence, num incorrect");
    }
    for (int i = 0; i < confidence.num; ++i) {
        m_face[i].m_liveness_confidence = confidence.confidence[i];
    }
}

void DetectResult::setFaceMaskConfidence(const HFFaceMaskConfidence &confidence) {
    if (confidence.num != m_num_faces) {
        throw std::runtime_error("setRGBLivenessConfidence, num incorrect");
    }
    for (int i = 0; i < confidence.num; ++i) {
        m_face[i].m_face_mask_confidence = confidence.confidence[i];
    }
}

void DetectResult::setFaceQualityConfidence(const HFFaceQualityConfidence &confidence) {
    if (confidence.num != m_num_faces) {
        throw std::runtime_error("setRGBLivenessConfidence, num incorrect");
    }
    for (int i = 0; i < confidence.num; ++i) {
        m_face[i].m_quality_confidence = confidence.confidence[i];
    }
}

void DetectResult::setFaceInteractionResult(const HFFaceIntereactionResult &result) {
    if (result.num != m_num_faces) {
        throw std::runtime_error("setRGBLivenessConfidence, num incorrect");
    }
    for (int i = 0; i < result.num; ++i) {
        m_face[i].m_left_eye_open_confidence = result.leftEyeStatusConfidence[i];
        m_face[i].m_right_eye_open_confidence = result.rightEyeStatusConfidence[i];
    }
}

void DetectResult::setFaceAttributeResult(const HFFaceAttributeResult &result) {
    if (result.num != m_num_faces) {
        throw std::runtime_error("setRGBLivenessConfidence, num incorrect");
    }
    for (int i = 0; i < result.num; ++i) {
        m_face[i].m_attr_race = result.race[i];
        m_face[i].m_attr_gender = result.gender[i];
        m_face[i].m_attr_age = result.ageBracket[i];
    }
}

InspireFaceDetector::InspireFaceDetector(const DetectConfig &config) {
    if (!ginitInspireFace()) {
        return;
    }
    m_config = config;
    m_custom_parameter = {
            .enable_recognition = config.enable_recognition,
            .enable_liveness = config.enable_liveness,
            .enable_ir_liveness = config.enable_ir_liveness,
            .enable_mask_detect = config.enable_mask_detect,
            .enable_face_quality = config.enable_face_quality,
            .enable_face_attribute = config.enable_face_attribute,
            .enable_interaction_liveness = config.enable_interaction_liveness
    };
    HResult ret = HFCreateInspireFaceSession(
            m_custom_parameter, config.detect_mode, config.max_detect_faces,
            config.detect_pixel_level, config.track_fps, &m_session);
    if (ret != HSUCCEED) {
        m_session = {nullptr};
        printf("HFCreateInspireFaceSessionOptional error: %ld\n", ret);
    } else {
        m_created_flag = true;
    }
}

InspireFaceDetector::~InspireFaceDetector() {
    if (!m_created_flag) {
        return;
    }
    m_created_flag = false;
    HResult ret = HFReleaseInspireFaceSession(m_session);
    if (ret != HSUCCEED) {
        printf("Release session error: %lu\n", ret);
    }
}

void InspireFaceDetector::setTrackPreviewSize(int size) {
    if (!m_created_flag) {
        throw std::runtime_error("Detector not init!");
    }
    HResult ret = HFSessionSetTrackPreviewSize(m_session, size);
    if (ret != HSUCCEED) {
        printf("HFSessionSetTrackPreviewSize error: %lu\n", ret);
    }
}

void InspireFaceDetector::setFilterMiniFacePixelSize(int size) {
    if (!m_created_flag) {
        throw std::runtime_error("Detector not init!");
    }
    HResult ret = HFSessionSetFilterMinimumFacePixelSize(m_session, size);
    if (ret != HSUCCEED) {
        printf("setFilterMiniFacePixelSize error: %lu\n", ret);
    }
}

void InspireFaceDetector::setFaceDetectThreshold(float threshold) {
    if (!m_created_flag) {
        throw std::runtime_error("Detector not init!");
    }
    HResult ret = HFSessionSetFaceDetectThreshold(m_session, threshold);
    if (ret != HSUCCEED) {
        printf("HFSessionSetFaceDetectThreshold error: %lu\n", ret);
    }
}

DetectResult &InspireFaceDetector::detect(uint8_t *data, int width, int height,
                                          HFRotation rotation, HFImageFormat format, bool pipelineProcess) {
    if (!m_created_flag) {
        throw std::runtime_error("Detector not init!");
    }

    HFImageData imageParam;
    imageParam.data = data;       // Data buffer
    imageParam.width = width;      // Target view width
    imageParam.height = height;      // Target view width
    imageParam.rotation = rotation;      // Data source rotate
    imageParam.format = format;      // Data source format

    HFImageStream stream = nullptr;
    HResult ret = HFCreateImageStream(&imageParam, &stream);
    if (ret != HSUCCEED) {
        printf("HFCreateImageStream error: %lu\n", ret);
        throw std::runtime_error("HFCreateImageStream error!!");
    }

    HFMultipleFaceData faceData = {0};
    ret = HFExecuteFaceTrack(m_session, stream, &faceData);
    m_detect_result.onFaceDetected(faceData);

    if (ret != HSUCCEED) {
        HFReleaseImageStream(stream);

        printf("HFExecuteFaceTrack error: %lu\n", ret);
        return m_detect_result;
    }

    if (pipelineProcess) {
        ret = HFMultipleFacePipelineProcess(m_session, stream, &faceData, m_custom_parameter);
        if (ret != HSUCCEED) {
            HFReleaseImageStream(stream);

            printf("HFMultipleFacePipelineProcess error: %lu\n", ret);
            return m_detect_result;
        }

        if (m_custom_parameter.enable_liveness) {
            HFRGBLivenessConfidence livenessConfidence = {0};
            ret = HFGetRGBLivenessConfidence(m_session, &livenessConfidence);
            if (ret != HSUCCEED) {
                printf("HFGetRGBLivenessConfidence error: %lu\n", ret);
            } else {
                m_detect_result.setRGBLivenessConfidence(livenessConfidence);
            }
        }

        if (m_custom_parameter.enable_mask_detect) {
            HFFaceMaskConfidence maskConfidence = {0};
            ret = HFGetFaceMaskConfidence(m_session, &maskConfidence);
            if (ret != HSUCCEED) {
                printf("HFMultipleFacePipelineProcess error: %lu\n", ret);
            } else {
                m_detect_result.setFaceMaskConfidence(maskConfidence);
            }
        }

        if (m_custom_parameter.enable_face_quality) {
            HFFaceQualityConfidence qualityConfidence = {0};
            ret = HFGetFaceQualityConfidence(m_session, &qualityConfidence);
            if (ret != HSUCCEED) {
                printf("HFGetFaceQualityConfidence error: %lu\n", ret);
            } else {
                m_detect_result.setFaceQualityConfidence(qualityConfidence);
            }
        }

        if (m_custom_parameter.enable_interaction_liveness) {
            HFFaceIntereactionResult faceIntereactionResult = {0};
            ret = HFGetFaceIntereactionResult(m_session, &faceIntereactionResult);
            if (ret != HSUCCEED) {
                printf("HFGetFaceInteractionResult error: %lu\n", ret);
            } else {
                m_detect_result.setFaceInteractionResult(faceIntereactionResult);
            }
        }

        if (m_custom_parameter.enable_face_attribute) {
            HFFaceAttributeResult faceAttributeResult = {0};
            ret = HFGetFaceAttributeResult(m_session, &faceAttributeResult);
            if (ret != HSUCCEED) {
                printf("HFGetFaceAttributeResult error: %lu\n", ret);
            } else {
                m_detect_result.setFaceAttributeResult(faceAttributeResult);
            }
        }
    }

    HFReleaseImageStream(stream);
    return m_detect_result;
}