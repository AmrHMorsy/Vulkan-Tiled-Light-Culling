#pragma once
#include "Includes.h"
#include "Parser.h"




class Loader{
    
public:
    
    static void Load(EngineInfo& engine, ModelInfo& model, DepthPrepassInfo& depthPrepass, const std::vector<ModelConfig>& models)
    {
        std::vector<uint32_t> indices;
        std::vector<glm::vec3> vertices, normals;
        std::vector<glm::vec2> textureCoordinates;
        uint32_t vertexOffset = 0, indexOffset = 0;
        std::vector<VkDrawIndexedIndirectCommand> drawCommands;
        
        for(int i = 0; i < models.size(); i++)
            Loader::Load(engine, model, models[i].filePath, vertices, normals, indices, textureCoordinates, drawCommands, vertexOffset, indexOffset, i, model.max, model.min);
                
        model.indexBuffer = Buffer::BuildDataBuffer(engine.logicalDevice, engine.physicalDevice.handle, engine.graphicsQueue, indices.data(), sizeof(uint32_t) * indices.size(), engine.physicalDevice.queueIndex);
        
        model.vertexBuffer = Buffer::BuildDataBuffer(engine.logicalDevice, engine.physicalDevice.handle, engine.graphicsQueue, vertices.data(), sizeof(glm::vec3) * vertices.size(), engine.physicalDevice.queueIndex);
        
        model.normalBuffer = Buffer::BuildDataBuffer(engine.logicalDevice, engine.physicalDevice.handle, engine.graphicsQueue, normals.data(), sizeof(glm::vec3) * normals.size(), engine.physicalDevice.queueIndex);
        
        model.textureCoordinateBuffer = Buffer::BuildDataBuffer(engine.logicalDevice, engine.physicalDevice.handle, engine.graphicsQueue, textureCoordinates.data(), sizeof(glm::vec2) * textureCoordinates.size(), engine.physicalDevice.queueIndex);
        
        model.drawCommandsBuffer = Buffer::BuildDataBuffer(engine.logicalDevice, engine.physicalDevice.handle, engine.graphicsQueue, drawCommands.data(), sizeof(drawCommands[0]) * drawCommands.size(), engine.physicalDevice.queueIndex);
        
        model.numDrawCommands = drawCommands.size();
        model.vertexBuffers = {model.vertexBuffer.buffer, model.normalBuffer.buffer, model.textureCoordinateBuffer.buffer};
        model.vertexBufferOffsets = {0, 0, 0};
        depthPrepass.vertexBuffers = {model.vertexBuffer.buffer};
        depthPrepass.vertexBufferOffsets = {0};
    }
        
private:
    
    static void Load(EngineInfo& engine, ModelInfo& model, std::string filePath, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<uint32_t>& indices, std::vector<glm::vec2>& textureCoordinates, std::vector<VkDrawIndexedIndirectCommand>& drawCommands, uint32_t& vertexOffset, uint32_t& indexOffset, int& firstInstance, glm::vec3& max, glm::vec3& min)
    {
        std::vector<Mesh> meshes;
        if(!Parser::Parse(filePath, meshes))
            return;

        for(size_t i = 0; i < meshes.size(); i++){
            for(size_t j = 0; j < meshes[i].vertices.size(); j++){
                max = glm::max(max, meshes[i].vertices[j]);
                min = glm::min(min, meshes[i].vertices[j]);
                vertices.push_back(meshes[i].vertices[j]);
            }
            for(size_t j = 0; j < meshes[i].normals.size(); j++)
                normals.push_back(meshes[i].normals[j]);
            for(size_t j = 0; j < meshes[i].textureCoordinates.size(); j++)
                textureCoordinates.push_back(meshes[i].textureCoordinates[j]);
            for(size_t j = 0; j < meshes[i].indices.size(); j++)
                indices.push_back(meshes[i].indices[j]);
                        
            drawCommands.push_back(VkDrawIndexedIndirectCommand{
                .indexCount = static_cast<uint32_t>(meshes[i].indices.size()),
                .instanceCount = 1,
                .firstIndex = static_cast<uint32_t>(indexOffset),
                .vertexOffset = static_cast<int32_t>(vertexOffset),
                .firstInstance = static_cast<uint32_t>(firstInstance),
            });
            
            vertexOffset += meshes[i].vertices.size();
            indexOffset += meshes[i].indices.size();
            model.totalNumTriangles += (meshes[i].indices.size() / 3);
        }
    }
};
