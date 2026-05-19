#pragma once
#include "Includes.h"


const int MAX_NUM_LIGHTS = 1000;
const int MAX_NUM_MODELS = 50;
const int MAX_FRAMES_IN_FLIGHT = 3;
const glm::uvec2 tileResolution(16, 16);

struct LightConfig{
    float intensity = 1.0f;
    glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec4 position;
    float minLightIntensity = 0.1f;
};

struct CameraConfig{
    float speed;
    float fovDegrees;
    float lookAroundSpeed;
    float nearClippingPlane;
    float farClippingPlane;
    glm::vec4 position;
    glm::vec4 direction;
};

struct ModelConfig{
    std::string filePath;
    std::string albedoFilePath = "";
};

struct SceneConfig{
    std::string name;
    glm::vec4 lightSeed;
    CameraConfig cameraConfig;
    std::vector<ModelConfig> models;
};

const std::vector<SceneConfig> sceneConfigs{
    SceneConfig{
        .name = "Car Scene",
        .cameraConfig = CameraConfig{
            .speed = 0.01,
            .fovDegrees = 45.0,
            .lookAroundSpeed = 0.01,
            .nearClippingPlane = 0.1,
            .farClippingPlane = 500.0,
            .position = glm::vec4(6.93, 2.01, 2.73, 1.0),
            .direction = glm::vec4(-0.81, -0.16, -0.56, 0.0)
        },
        .models = std::vector<ModelConfig>{
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Garage/Concrete/Concrete.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Garage/Concrete/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Garage/DamageWall/DamageWall.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Garage/DamageWall/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Garage/dec_garage_sewage/dec_garage_sewage.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Garage/dec_garage_sewage/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Garage/mat_garage_cable/mat_garage_cable.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Garage/mat_garage_cable/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Garage/mat_garage_canister/mat_garage_canister.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Garage/mat_garage_canister/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Garage/mat_garage_car_jack/mat_garage_car_jack.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Garage/mat_garage_car_jack/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Garage/mat_garage_concrete_slabs/mat_garage_concrete_slabs.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Garage/mat_garage_concrete_slabs/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Garage/mat_garage_metal/mat_garage_metal.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Garage/mat_garage_metal/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Garage/mat_garage_tool_table/mat_garage_tool_table.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Garage/mat_garage_tool_table/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Garage/mat_garage_wheel/mat_garage_wheel.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Garage/mat_garage_wheel/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Garage/mat_garage_poster/mat_garage_poster.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Garage/mat_garage_poster/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Garage/trim_garage_metal/trim_garage_metal.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Garage/trim_garage_metal/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Garage/mat_garage_floor/mat_garage_floor.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Garage/mat_garage_floor/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Garage/mat_garage_lamp/mat_garage_lamp.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Garage/mat_garage_lamp/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Garage/mat_garage_wood/mat_garage_wood.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Garage/mat_garage_wood/Albedo.jpg"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Garage/mat_tool_stand/mat_tool_stand.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Garage/mat_tool_stand/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Camaro/Detail/Detail.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Camaro/Detail/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Camaro/Glass/Glass.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Camaro/Glass/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Camaro/Undercarriage/Undercarriage.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Camaro/Undercarriage/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Camaro/Door/Door.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Camaro/Door/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Camaro/Hood/Hood.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Camaro/Hood/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Camaro/Wheels/Wheels.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Camaro/Wheels/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Camaro/Exhaust/Exhaust.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Camaro/Exhaust/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Camaro/RearBumper/RearBumper.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Camaro/RearBumper/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Camaro/FrontBumper/FrontBumper.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Camaro/FrontBumper/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Camaro/FrontFender/FrontFender.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Camaro/FrontFender/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Camaro/RearQuarterPanel/RearQuarterPanel.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Camaro/RearQuarterPanel/Albedo.png"
            },
            ModelConfig{
                .filePath = "../Resources/assets/CarScene/Camaro/Roof/Roof.obj",
                .albedoFilePath = "../Resources/assets/CarScene/Camaro/Roof/Albedo.png"
            }
        }
    }
};
