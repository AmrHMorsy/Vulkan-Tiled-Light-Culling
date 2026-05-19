cmake ../

make

glslc ../Resources/Shaders/Main/Main.vert -o ../Resources/Shaders/Main/vert.spv

glslc ../Resources/Shaders/Main/Main.frag -o ../Resources/Shaders/Main/frag.spv

glslc ../Resources/Shaders/LightCulling/LightCulling.comp -o ../Resources/Shaders/LightCulling/LightCulling.spv

glslc ../Resources/Shaders/LightCulling/LightCullingPrepass.comp -o ../Resources/Shaders/LightCulling/LightCullingPrepass.spv

glslc ../Resources/Shaders/DepthPrepass/DepthPrepass.vert -o ../Resources/Shaders/DepthPrepass/DepthPrepass_vs.spv

rm -f imgui.ini
