#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <learnopengl/filesystem.h>
//#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

#include "shader.h"
#include "text_renderer.h"
#include "picking.hpp"

#include "texture.h"
#include "texture_cube.h"
#include "opengl_utils.h"
#include "geometry_primitives.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader modelShader("../../shaders/model_shader.vs", "../../shaders/model_shader.fs");
    Shader skyboxShader("../../shaders/shader_skybox.vs", "../../shaders/shader_skybox.fs");
    // load models
    // -----------
    //Model backModel(FileSystem::getPath("resources/objects/backpack/backpack.obj"));
    Model mapModel(FileSystem::getPath("resources/objects/map3/range_cut3.obj"));
    Model backModel(FileSystem::getPath("resources/objects/backpack/backpack.obj"));
    Model boxModel(FileSystem::getPath("resources/objects/box/box.obj"));

    // skybox
    std::vector<std::string> faces
    {
        FileSystem::getPath("resources/skybox/right.jpg"),
        FileSystem::getPath("resources/skybox/left.jpg"),
        FileSystem::getPath("resources/skybox/top.jpg"),
        FileSystem::getPath("resources/skybox/bottom.jpg"),
        FileSystem::getPath("resources/skybox/front.jpg"),
        FileSystem::getPath("resources/skybox/back.jpg")
    };
    CubemapTexture skyboxTexture = CubemapTexture(faces);
    unsigned int VAOskybox, VBOskybox;
    getPositionVAO(skybox_positions, sizeof(skybox_positions), VAOskybox, VBOskybox);
    skyboxShader.use();
    skyboxShader.setInt("skyboxTexture1", 0);


    // Generate positions & rotations for 100 monkeys
	std::vector<glm::vec3> positions(5);
	std::vector<glm::quat> orientations(5);
	for(int i=0; i<5; i++){
        positions[i] = glm::vec3(rand()%10-5, rand()%10-5, rand()%10-5);
		orientations[i] = glm::quat(glm::vec3(rand()%360, rand()%360, rand()%360));
	}
    
    // TextRenderer *Text;
    // Text = new TextRenderer(SCR_WIDTH, SCR_HEIGHT);
    // Text->Load(FileSystem::getPath("resources/fonts/OCRAEXT.TTF"), 24);
    std::string message;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        modelShader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        modelShader.setInt("is_billboard", 0);
        glm::mat4 model = glm::mat4(1.0f);

		// PICKING IS DONE HERE
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)){
			
			glm::vec3 ray_origin;
			glm::vec3 ray_direction;
			ScreenPosToWorldRay(
				SCR_WIDTH/2, SCR_HEIGHT/2,
				SCR_WIDTH, SCR_HEIGHT, 
				view, 
				projection, 
				ray_origin, 
				ray_direction
			);	
			// std::cout << "Left mouse button pressed" << std::endl;
            // std::cout << "  Ray origin: " << ray_origin.x << " " << ray_origin.y << " " << ray_origin.z << std::endl;
            // std::cout << "  Ray direction: " << ray_direction.x << " " << ray_direction.y << " " << ray_direction.z << std::endl;

			//ray_direction = ray_direction*20.0f;

			// Test each each Oriented Bounding Box (OBB).
			// A physics engine can be much smarter than this, 
			// because it already has some spatial partitionning structure, 
			// like Binary Space Partitionning Tree (BSP-Tree),
			// Bounding Volume Hierarchy (BVH) or other.
			for(int i=0; i<5; i++){

				float intersection_distance; // Output of TestRayOBBIntersection()
				glm::vec3 aabb_min(-1.0f, -1.0f, -1.0f);
				glm::vec3 aabb_max( 1.0f,  1.0f,  1.0f);

				// The ModelMatrix transforms :
				// - the mesh to its desired position and orientation
				// - but also the AABB (defined with aabb_min and aabb_max) into an OBB
                model = glm::mat4(1.0f);
				glm::mat4 RotationMatrix = glm::toMat4(orientations[i]);
				glm::mat4 TranslationMatrix = translate(model, positions[i]);
				glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix;

				if ( TestRayOBBIntersection(
					ray_origin, 
					ray_direction, 
					aabb_min, 
					aabb_max,
					ModelMatrix,
					intersection_distance)
				){
					std::ostringstream oss;
					oss << "mesh " << i;
					message = oss.str();
                    std::cout << "its bullet !!!!!!!!!!!!    /    i :" << i << std::endl;
					break;
				}
			}

		}

        // render the loaded model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.1f));
        modelShader.setMat4("model", model);
        mapModel.Draw(modelShader);

        for(int i=0; i<5; i++){
            model = glm::mat4(1.0f);
            model = glm::scale(model, glm::vec3(0.3f));
			glm::mat4 RotationMatrix = glm::toMat4(orientations[i]);
			glm::mat4 TranslationMatrix = translate(model, positions[i]);
			model = TranslationMatrix * RotationMatrix;
            modelShader.setMat4("model", model);
            backModel.Draw(modelShader);
            boxModel.Draw(modelShader);
		}


        // render crosshair
        // modelShader.setInt("is_billboard", 0);
        // model = glm::mat4(1.0f);
        // glBindVertexArray(quadVAO);
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, texture_grass_ground.ID);
        // shader.setInt("is_billboard", 0);
        // glm::vec3 grassGroundPosition = glm::vec3(0.0f, 0.0f, 0.0f);
        // model = glm::mat4(1.0f);
        // model = glm::translate(model, grassGroundPosition);
        // model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        // model = glm::scale(model, glm::vec3(grassGroundSize));
        // shader.setMat4("model", model);
        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);    
        // glBindVertexArray(0);

        // if(int(currentFrame)%20 == 0){
        //     std::cout << "message: " << message << std::endl;
        // }
        //cout << "i: " << i << "  / positions: " << glm::to_string(positions[i]) << endl;
        //model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(3.0f, 5.0f, 5.0f));
        //model = glm::scale(model, glm::vec3(0.1f));
        //ourShader.setMat4("model", model);
        //backModel.Draw(ourShader);

        //Text->RenderText("message : " + message, 5.0f, 5.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));

        // use skybox Shader
        skyboxShader.use();
        glDepthFunc(GL_LEQUAL);
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        // render a skybox
        glBindVertexArray(VAOskybox);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture.textureID);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
