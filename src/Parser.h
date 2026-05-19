#pragma once
#include "Includes.h"



class Parser{
    
public:
    
    static bool Parse(std::string filePath, std::vector<Mesh>& meshes)
    {
        Assimp::Importer importer ;
        const aiScene * scene = importer.ReadFile(filePath, aiProcess_JoinIdenticalVertices) ;
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            return false;
        
        TraverseMeshes(scene->mRootNode, scene, NULL, meshes);
        
        return true;
    }

private:
    
    static void TraverseMeshes(aiNode * node, const aiScene * scene, aiNode * parent, std::vector<Mesh>& meshes)
    {
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
            meshes.push_back(ParseMesh(scene->mMeshes[node->mMeshes[i]], scene));

        for(unsigned int i = 0; i < node->mNumChildren; i++)
            TraverseMeshes(node->mChildren[i], scene, node, meshes) ;
    }

    static Mesh ParseMesh(aiMesh * mesh, const aiScene * scene)
    {
        Mesh m;
        
        m.vertices = ExtractVertices(mesh);
        m.normals = ExtractNormals(mesh);
        m.indices = ExtractIndices(mesh);
        m.textureCoordinates = ExtractTextureCoordinates(mesh);
        
        return m;
    }

    static std::vector<glm::vec3> ExtractVertices(aiMesh * mesh)
    {
        std::vector<glm::vec3> vertices ;
        for(unsigned int j = 0; j < mesh->mNumVertices; j++)
            vertices.push_back(glm::vec3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z));
        
        return vertices ;
    }

    static std::vector<glm::vec3> ExtractNormals(aiMesh * mesh)
    {
        std::vector<glm::vec3> normals ;
        for( unsigned int j = 0 ; j < mesh->mNumVertices ; j++ )
            normals.push_back(glm::normalize(glm::vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z))) ;

        return normals ;
    }

    static std::vector<uint32_t> ExtractIndices(aiMesh * mesh)
    {
        std::vector<uint32_t> indices ;
        
        for( unsigned int j = 0 ; j < mesh->mNumFaces ; j++ ) {
            for( unsigned int k = 0 ; k < mesh->mFaces[j].mNumIndices ; k++ )
                indices.push_back( mesh->mFaces[j].mIndices[k] ) ;
        }
        
        return indices;
    }
    
    static std::vector<glm::vec2> ExtractTextureCoordinates(aiMesh * mesh)
    {
        std::vector<glm::vec2> textureCoordinates ;
        for( unsigned int j = 0 ; j < mesh->mNumVertices ; j++ )
            textureCoordinates.push_back(glm::vec2(mesh->mTextureCoords[0][j].x, 1.0f - mesh->mTextureCoords[0][j].y)) ;

        return textureCoordinates;
    }
};
