#include "Window.h"
#include "Renderer.h"
#include "SceneGraph.h"
#include <stdio.h>
#include "OrbitControl.h"
#include "FreeControl.h"
int main()
{

    SceneGraph scene;
    // scene.load("resources/gltf/helmet/DamagedHelmet.gltf");
    scene.load("resources/gltf/alpha_blend/AlphaBlendModeTest.gltf");
    // scene.load("resources/gltf/bistro_exterior/bistro_exterior.gltf");
    // scene.load("resources/gltf/buggy/Buggy.gltf");
    // scene.load("resources/gltf/waterbottle/WaterBottle.gltf");
    // scene.load("resources/gltf/engine/2CylinderEngine.gltf");
    // scene.load("resources/gltf/scifihelmet/SciFiHelmet.gltf");
    // scene.load("resources/gltf/sponza/Sponza.gltf");


    Window window({});
    Renderer renderer(&window);
    scene.camera.eye = glm::vec3(0, 0.0, 6.0f);
    // scene.camera.eye = glm::vec3(0.0, 0.0, 3.0);
    scene.camera.target = glm::vec3(0.0, 1.0, 0.0);
    scene.camera.view = glm::lookAt(scene.camera.eye, scene.camera.target, glm::vec3(0.0, 1.0, 0.0));

    scene.camera.proj = glm::perspective(glm::radians(60.f), (F32)renderer.width() / (F32)renderer.height(), 0.1f, 1000.0f);

    renderer.initScene(scene);

    window.setSizeCallback([&](U32 width, U32 height)
                           { renderer.resize(width, height);
                             scene.camera.proj = glm::perspective(glm::radians(45.f), (F32)width / (F32)height, 0.1f, 1000.0f); });

    F32 radius = glm::length(scene.camera.eye - scene.camera.target);
    // OrbitControl control(scene.camera, window, radius);
    FreeControl control(scene.camera, window);
    F32 lastTime = glfwGetTime();
    while (!window.shouldClose())
    {
        F32 dt = glfwGetTime() - lastTime;
        F32 fps = 1.0f / dt;
        printf("fps: %f\n", fps);
        lastTime = glfwGetTime();
        control.update(dt);
        renderer.render(scene);
        window.swapBuffers();
        window.pollEvent();
    }

    scene.releaseGPUResource();

    window.cleanup();

    return 0;
}