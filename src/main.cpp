#include "UI.h"
#include "Scene.h"
#include "Engine.h"
#include "Events.h"
#include "SwapChain.h"


void Build(ApplicationInfo& application);
void Launch(ApplicationInfo& application);
void Destroy(ApplicationInfo& application);


int main()
{
    ApplicationInfo application;
    
    Build(application);
    Launch(application);
    Destroy(application);
    
    return 0;
}

void Build(ApplicationInfo& application)
{
    Engine::Build(application.engine, application.window);
    UI::Build(application.ui, application.engine, application.window);
    
    size_t numScenes = sceneConfigs.size();
    application.scenes.resize(numScenes);
    application.sceneNames.resize(numScenes);
    application.sceneNamesPtr.resize(numScenes);
    
    for(size_t i = 0; i < numScenes; i++){
        Scene::Build(application.scenes[i], application.engine, sceneConfigs[i]);
        application.sceneNames[i] = sceneConfigs[i].name;
        application.sceneNamesPtr[i] = application.sceneNames[i].c_str();
    }
}

void Launch(ApplicationInfo& application)
{
    while(!glfwWindowShouldClose(application.window.handle)){
        application.selectedScene = (application.selectedScene != application.nextSelectedScene)? application.nextSelectedScene: application.selectedScene;
        
        glfwPollEvents();
        Events::Handle(application.window.handle, application.scenes[application.selectedScene]);
        application.engine.imageIndex = SwapChain::AcquireNextImage(application.engine, application.window, application.frame);
        
        if(application.engine.imageIndex == -1)
            continue;
        
        Scene::Record(application.scenes[application.selectedScene], application.engine, application.frame);
        UI::Record(application.ui, application.scenes[application.selectedScene], application.window, application.engine, application.frame, application.sceneNamesPtr, application.nextSelectedScene);
        Scene::Submit(application.scenes[application.selectedScene], application.engine, application.frame);
        UI::Submit(application.ui, application.scenes[application.selectedScene], application.window, application.frame, application.engine);
        
        if(!SwapChain::PresentImage(application.engine, application.ui.renderFinishedSemaphores[application.frame]))
            continue;

        application.frame = (application.frame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
}

void Destroy(ApplicationInfo& application)
{
    vkDeviceWaitIdle(application.engine.logicalDevice) ;
    glfwSetWindowUserPointer(application.window.handle, nullptr);
    glfwDestroyWindow(application.window.handle);
    vkDestroyDevice(application.engine.logicalDevice, NULL) ;
    vkDestroySurfaceKHR(application.engine.vulkanInstance, application.engine.windowSurface, NULL);
    vkDestroyInstance(application.engine.vulkanInstance, NULL);
    glfwTerminate();
}
