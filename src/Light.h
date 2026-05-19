#pragma once
#include "Data.h"
#include "Includes.h"
#include "Structs.h"
#include <random>


class Light{
    
public:
    
    static void Build(EngineInfo& engine, SceneInfo& scene)
    {
        std::vector<LightConfig> lightConfigs = GenerateLights(scene.model.max, scene.model.min);
        for(int i = 0; i < lightConfigs.size(); i++){
            LightInfo light;
            light.config = lightConfigs[i];
            light.maxLightIntensityComponent = ComputeMaxLightIntensityComponent(lightConfigs[i].color, lightConfigs[i].intensity);
            light.boundingSphere = ComputeBoundingSphere(lightConfigs[i].position, light.maxLightIntensityComponent, lightConfigs[i].minLightIntensity);
            scene.lights.push_back(light);
            scene.lightNames.push_back("Light " + std::to_string(i+1));
        }
        
        scene.lightNamesPtr.resize(scene.lights.size());
        for(uint32_t i = 0; i < scene.lights.size(); i++)
            scene.lightNamesPtr[i] = scene.lightNames[i].c_str();
    }
    
    static float ComputeMaxLightIntensityComponent(glm::vec3 lightColor, float lightIntensity)
    {
        glm::vec3 color = lightIntensity * lightColor;
        
        return std::max({color.r, color.g, color.b});
    }
    
    static glm::vec4 ComputeBoundingSphere(glm::vec4 lightPosition, float maxLightIntensityComponent, float minLightIntensity)
    {
        glm::vec3 center = lightPosition;
        float radius = sqrt(maxLightIntensityComponent/minLightIntensity);
        
        return glm::vec4(center, radius);
    }
    
    static std::vector<LightConfig> GenerateLights(glm::vec3 max, glm::vec3 min)
    {
        std::vector<LightConfig> lightConfigs;
        
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> distX(min.x, max.x);
        std::uniform_real_distribution<float> distY(min.y, max.y);
        std::uniform_real_distribution<float> distZ(min.z, max.z);
        for (uint32_t i = 0; i < MAX_NUM_LIGHTS; ++i){
            glm::vec3 pos = {distX(rng), distY(rng), distZ(rng)};
            lightConfigs.push_back(LightConfig{
                .intensity = 1.0,
                .minLightIntensity = 0.1,
                .color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                .position = glm::vec4(pos, 1.0f)
            });
        }

        return lightConfigs;
    }
};
