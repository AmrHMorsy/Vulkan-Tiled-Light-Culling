#pragma once
#include "Model.h"
#include "Includes.h"
#include "Engine.h"



class Camera {
    
public:
    
    static void Build(EngineInfo& engine, CameraInfo& camera, ModelInfo& model, DepthPrepassInfo& depthPrepass, const CameraConfig& cameraConfig, bool displayDepth)
    {
        camera.config = cameraConfig;
        camera.direction = glm::vec4(glm::normalize(glm::vec3(camera.config.direction)), 0.0f);
        camera.fovRadians = glm::radians(camera.config.fovDegrees);
        camera.resolution = engine.swapChain.extent;
        camera.up = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
        camera.right = glm::vec4(glm::normalize(glm::cross(glm::vec3(camera.direction), glm::vec3(camera.up))), 0.0f);
        camera.up = glm::vec4(glm::normalize(glm::cross(glm::vec3(camera.right), glm::vec3(camera.direction))), 0.0f);
        camera.viewMatrix = glm::lookAt(glm::vec3(camera.config.position), glm::vec3(camera.config.position + camera.direction), glm::vec3(camera.up));
        
        float aspectRatio = (float)engine.swapChain.extent.width / (float)engine.swapChain.extent.height;
        
        
        //camera.projectionMatrix = glm::perspectiveRH_ZO(camera.fovRadians, aspectRatio, camera.config.nearClippingPlane, camera.config.farClippingPlane);
        camera.projectionMatrix = glm::perspective(camera.fovRadians, aspectRatio, camera.config.nearClippingPlane, camera.config.farClippingPlane);
        camera.projectionMatrix[1][1] *= -1;
                
        model.fsData.cameraPosition_isDepth = glm::vec4(camera.config.position.x, camera.config.position.y, camera.config.position.z, displayDepth);
        model.fsData.nearClippingPlane = camera.config.nearClippingPlane;
        model.fsData.farClippingPlane = camera.config.farClippingPlane;
        
        UpdateUniformData(camera, model, depthPrepass);
    }

    static void LookAround(CameraInfo& camera, glm::vec4 axis, float sign, ModelInfo& model, DepthPrepassInfo& depthPrepass)
    {
        camera.rotation = camera.rotation * glm::rotate(glm::mat4(1.0f), sign * camera.config.lookAroundSpeed, glm::vec3(axis));
        camera.lookAroundRotation = glm::rotate(glm::mat4(1.0f), sign * camera.config.lookAroundSpeed, glm::vec3(axis));
        camera.direction = glm::vec4(camera.lookAroundRotation * glm::vec4(glm::vec3(camera.direction), 0.0f));
        camera.up = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
        camera.right = glm::vec4(glm::normalize(glm::cross(glm::vec3(camera.direction), glm::vec3(camera.up))), 0.0f);
        camera.up = glm::vec4(glm::normalize(glm::cross(glm::vec3(camera.right), glm::vec3(camera.direction))), 0.0f);
        camera.viewMatrix = glm::lookAt(glm::vec3(camera.config.position), glm::vec3(camera.config.position + camera.direction), glm::vec3(camera.up));
        
        UpdateUniformData(camera, model, depthPrepass);
    }

    static void Move(CameraInfo& camera, glm::vec4 direction, ModelInfo& model, DepthPrepassInfo& depthPrepass)
    {
        camera.config.position = camera.config.position + (camera.config.speed * direction);
            
        camera.viewMatrix = glm::lookAt(glm::vec3(camera.config.position), glm::vec3(camera.config.position + camera.direction), glm::vec3(camera.up));
        
        UpdateUniformData(camera, model, depthPrepass);
    }
    
    static void UpdateUniformData(CameraInfo& camera, ModelInfo& model, DepthPrepassInfo& depthPrepass)
    {
        model.vsData.view = camera.viewMatrix;
        model.vsData.projection = camera.projectionMatrix;
        depthPrepass.vsData.cameraViewMatrix = camera.viewMatrix;
        depthPrepass.vsData.cameraProjectionMatrix = camera.projectionMatrix;
    }
};
