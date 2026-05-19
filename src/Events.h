#pragma once
#include "Includes.h"
#include "Camera.h"



class Events{
    
public:
    
    static void Handle(GLFWwindow * window, SceneInfo& scene)
    {
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            Camera::Move(scene.camera, scene.camera.direction, scene.model, scene.depthPrepass);
        
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            Camera::Move(scene.camera, -scene.camera.direction, scene.model, scene.depthPrepass);
        
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            Camera::Move(scene.camera, -scene.camera.right, scene.model, scene.depthPrepass);
        
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            Camera::Move(scene.camera, scene.camera.right, scene.model, scene.depthPrepass);
        
        if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            Camera::Move(scene.camera, scene.camera.up, scene.model, scene.depthPrepass);
        
        if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            Camera::Move(scene.camera, -scene.camera.up, scene.model, scene.depthPrepass);
        
        if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            Camera::LookAround(scene.camera, scene.camera.right, 1.0f, scene.model, scene.depthPrepass);
        
        if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            Camera::LookAround(scene.camera, scene.camera.right, -1.0f, scene.model, scene.depthPrepass);
        
        if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            Camera::LookAround(scene.camera, scene.camera.up, 1.0f, scene.model, scene.depthPrepass);
        
        if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            Camera::LookAround(scene.camera, scene.camera.up, -1.0f, scene.model, scene.depthPrepass);
    }
};
