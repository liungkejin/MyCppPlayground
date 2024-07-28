//
// Created by LiangKeJin on 2024/7/18.
//

#pragma once

#include <inspireface.h>
#include <intypedef.h>
#include <herror.h>

struct DetectConfig {
    bool enable_recognition = false;               ///< Enable face recognition feature.
    bool enable_liveness = false;                  ///< Enable RGB liveness detection feature.
    bool enable_ir_liveness = false;               ///< Enable IR liveness detection feature.
    bool enable_mask_detect = false;               ///< Enable mask detection feature.
    bool enable_face_quality = false;              ///< Enable face quality detection feature.
    bool enable_face_attribute = false;            ///< Enable face attribute prediction feature.
    bool enable_interaction_liveness = false;      ///< Enable interaction for liveness detection feature.

    HFDetectMode detect_mode = HF_DETECT_MODE_ALWAYS_DETECT;
    int max_detect_faces = 1;
    int detect_pixel_level = -1;
    int track_fps = -1;
};

#define MAX_DETECT_FACES 5

class InspireFaceDetector;

class DetectResult;

class FaceData {
    friend class DetectResult;

public:
    inline int trackId() const {
        return m_track_id;
    }

    inline int x() const {
        return m_x;
    }

    inline int y() const {
        return m_y;
    }

    inline int width() const {
        return m_width;
    }

    inline int height() const {
        return m_height;
    }

    inline float pitch() const {
        return m_pitch;
    }

    inline float yaw() const {
        return m_yaw;
    }

    inline float roll() const {
        return m_roll;
    }

    inline int numLandmarks() const {
        return m_num_landmarks;
    }

    inline float landmarkX(int i) const {
        return m_points[i].x;
    }

    inline float landmarkY(int i) const {
        return m_points[i].y;
    }

private:
    void setup(HFMultipleFaceData &data, int index);

private:
    int m_index = 0;
    int m_track_id = 0;

    int m_x = 0, m_y = 0, m_width = 0, m_height = 0;
    float m_pitch = 0.f, m_yaw = 0.f, m_roll = 0.f;

    float m_liveness_confidence = 0;
    float m_face_mask_confidence = 0;

    float m_quality_confidence = 0;
    float m_left_eye_open_confidence = 0;
    float m_right_eye_open_confidence = 0;

    int m_attr_race = 0;
    int m_attr_gender = 0;
    int m_attr_age = 0;

    int m_num_landmarks = 0;
    HPoint2f m_points[256] = {0};
};

class DetectResult {
    friend class InspireFaceDetector;

public:
    inline int numFaces() const { return m_num_faces; }

    FaceData *face(int i) {
        if (i >= 0 && i < m_num_faces) {
            return m_face + i;
        }
        return nullptr;
    }

private:
    void onFaceDetected(HFMultipleFaceData &data);

    void setRGBLivenessConfidence(const HFRGBLivenessConfidence &confidence);

    void setFaceMaskConfidence(const HFFaceMaskConfidence &confidence);

    void setFaceQualityConfidence(const HFFaceQualityConfidence &confidence);

    void setFaceInteractionResult(const HFFaceIntereactionResult &result);

    void setFaceAttributeResult(const HFFaceAttributeResult &result);

private:
    int m_num_faces = 0;
    FaceData m_face[MAX_DETECT_FACES];
};

class InspireFaceDetector {
public:
    explicit InspireFaceDetector(const DetectConfig &config);

    ~InspireFaceDetector();

public:
    void setTrackPreviewSize(int size);

    void setFilterMiniFacePixelSize(int size);

    void setFaceDetectThreshold(float threshold);

    DetectResult &detect(uint8_t *data, int width, int height,
                         HFRotation rotation = HF_CAMERA_ROTATION_0, HFImageFormat format = HF_STREAM_BGR,
                         bool pipelineProcess = false);

private:
    DetectConfig m_config;
    HFSessionCustomParameter m_custom_parameter = {};

    HFSession m_session = nullptr;
    bool m_created_flag = false;

    DetectResult m_detect_result;
};

