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

// config
bool showHitbox = false;
float testx = 0.0f;
float testy = 0.0f;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float angleBetween(
    glm::vec3 a,
    glm::vec3 b,
    glm::vec3 origin=glm::vec3(0.0f, 0.0f, 0.0f)
){
    glm::vec3 da=glm::normalize(a-origin);
    glm::vec3 db=glm::normalize(b-origin);
    return glm::acos(glm::dot(da, db));
}

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
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build and compile shaders
    // -------------------------
    Shader modelShader("shader_model.vs", "shader_model.fs");
    Shader colorShader("shader_color.vs", "shader_color.fs");
    Shader skyboxShader("shader_skybox.vs", "shader_skybox.fs");
    // load models
    // -----------
    //Model backModel(FileSystem::getPath("resources/objects/backpack/backpack.obj"));
    Model mapModel(FileSystem::getPath("resources/objects/map3/range_cut3.obj"));
    Model botModel(FileSystem::getPath("resources/objects/bot/training_bot5.obj"));
    Model boxModel(FileSystem::getPath("resources/objects/box/box.obj"));
    Model transboxModel(FileSystem::getPath("resources/objects/transbox/transbox.obj"));

    // skybox
    std::vector<std::string> faces
    {
        FileSystem::getPath("resources/objects/skybox/right.jpg"),
        FileSystem::getPath("resources/objects/skybox/left.jpg"),
        FileSystem::getPath("resources/objects/skybox/top.jpg"),
        FileSystem::getPath("resources/objects/skybox/bottom.jpg"),
        FileSystem::getPath("resources/objects/skybox/front.jpg"),
        FileSystem::getPath("resources/objects/skybox/back.jpg")
    };
    CubemapTexture skyboxTexture = CubemapTexture(faces);
    unsigned int VAOskybox, VBOskybox;
    getPositionVAO(skybox_positions, sizeof(skybox_positions), VAOskybox, VBOskybox);

    // colored cube
	unsigned int cubeVBO, cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_positions_colors), cube_positions_colors, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    skyboxShader.use();
    skyboxShader.setInt("skyboxTexture1", 0);
    colorShader.use();
    colorShader.setFloat("alpha", 1.0f);

    // Generate positions & rotations 
	std::vector<float> respawnLeft(6);
	std::vector<glm::vec3> positions(6);
	std::vector<glm::vec3> positions_top(6);
	std::vector<glm::quat> orientations(6);
	for(int i=0; i<5; i++){
        respawnLeft[i] = 4.0f;
        positions[i] = glm::vec3(3.0f*i, 0.0f, 1.0f);
		orientations[i] = glm::quat(glm::vec3(0.0f, rand()%360, 0.0f));
        positions_top[i] = glm::vec3(3.0f*i, 2.0f, 1.0f);
	}
    respawnLeft[5] = 4.0f;
    positions[5] = glm::vec3(-5.0f, 0.0f, 1.0f);
    orientations[5] = glm::quat(glm::vec3(0.0f, 45.0f, 0.0f));
    positions_top[5] = glm::vec3(-2.0f, 2.0f, 1.0f);
    
    TextRenderer *Text;
    Text = new TextRenderer(SCR_WIDTH, SCR_HEIGHT);
    Text->Load(FileSystem::getPath("resources/fonts/OCRAEXT.TTF").c_str(), 48);
    std::string message;

    glm::vec3 last_ray_origin;
    glm::vec3 last_ray_direction;
    // render loop
    // -----------
    bool isFirst = true;
    bool handledMouse = false;
    float decayAlpha = 0.7f;
    float botAlpha = 0.6f;
    bool isForward = false;

    int hitted = 0;
    int missed = 0;
    bool showMarker = false;
    int last_hit_type = 0;
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        decayAlpha = decayAlpha - deltaTime;
        if(decayAlpha < 0.0f){
            decayAlpha = 0.0f;
            showMarker = false;
        }
        if(isFirst){
            std::cout << "isFirst" << std::endl;
            last_ray_origin = glm::vec3(0,0,0);
            last_ray_direction = glm::vec3(0,0,0);
            isFirst = false;
        }

        // moving bot
        if(isForward){
            positions[5].x += 2*deltaTime;
            positions[5].z += 2*deltaTime;
            if(positions[5].x > -1.0f) isForward = false;
        }
        else{
            positions[5].x -= 2*deltaTime;
            positions[5].z -= 2*deltaTime;
            if(positions[5].x < -5.0f) isForward = true;
        }
        positions_top[5] = glm::vec3(positions[5].x, positions[5].y+ 2.0f, positions[5].z);

        for(int i=0; i<6; i++){
            if(respawnLeft[i] <= 3.0f){
                respawnLeft[i] = respawnLeft[i] - deltaTime;
            }
            if(respawnLeft [i] <= 0.0f){
                respawnLeft[i] = 4.0f;
            }
        }
        message = "background";
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(4.0);
        glColor3f(1.0, 0.0, 0.0);
        glBegin(GL_LINES);
            glVertex3f(0.0, 0.0, 0.0);
            glVertex3f(15, 0, 1);
        glEnd();

        // don't forget to enable shader before setting uniforms

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        modelShader.use();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        modelShader.setFloat("alpha", 1.0f);
        colorShader.use();
        colorShader.setMat4("projection", projection);
        colorShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        modelShader.use();

		// PICKING IS DONE HERE
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;
        int hit_type = 0;
		if (handledMouse == false && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)){
			handledMouse = true;
			ScreenPosToWorldRay(
				SCR_WIDTH/2, SCR_HEIGHT/2,
				SCR_WIDTH, SCR_HEIGHT, 
				view, 
				projection, 
				ray_origin, 
				ray_direction
			);	
            if(ray_direction != glm::vec3(0,0,0)){
                last_ray_origin = ray_origin;
                last_ray_direction = ray_direction;
                decayAlpha = 0.7f;
            }

			// Test each each Oriented Bounding Box (OBB).
			// A physics engine can be much smarter than this, 
			// because it already has some spatial partitionning structure, 
			// like Binary Space Partitionning Tree (BSP-Tree),
			// Bounding Volume Hierarchy (BVH) or other.
			for(int i=0; i<6; i++){

				float intersection_distance; // Output of TestRayOBBIntersection()
				glm::vec3 aabb_min(-1.0f, -0.5f, -1.0f);
				glm::vec3 aabb_max( 1.0f,  1.0f,  1.0f);

                model = glm::mat4(1.0f);
				glm::mat4 RotationMatrix = glm::toMat4(orientations[i]);
				glm::mat4 TranslationMatrix = translate(model, glm::vec3(positions[i].x, positions[i].y+0.5f, positions[i].z));
				glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix;

				if ( TestRayOBBIntersection(
					ray_origin, 
					ray_direction, 
					aabb_min, 
					aabb_max,
					ModelMatrix,
					intersection_distance) && respawnLeft[i] > 3.99f
				){
					std::ostringstream oss;
					oss << "mesh " << i;
					message = oss.str();
                    respawnLeft[i] = 3.0f;
                    hit_type = 1;
					break;
				}

                aabb_min = glm::vec3(-1.0f, -1.0f, -1.0f);
                aabb_max = glm::vec3( 1.0f,  1.0f,  1.0f);

				TranslationMatrix = translate(model, positions_top[i]);
				ModelMatrix = TranslationMatrix * RotationMatrix;

				if ( TestRayOBBIntersection(
					ray_origin, 
					ray_direction, 
					aabb_min, 
					aabb_max,
					ModelMatrix,
					intersection_distance) && respawnLeft[i] > 3.99f
				){
					std::ostringstream oss;
					oss << "mesh " << i;
					message = oss.str();
                    respawnLeft[i] = 3.0f;
                    hit_type = 2;
					break;
				}
			}
            if (hit_type == 0) {
                missed += 1;
                showMarker = false;
            }
            else{
                hitted += 1;
                showMarker = true;
                last_hit_type = hit_type;
            }
		}
        else if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == false){
            handledMouse = false;
        }

        // render the loaded model
        model = glm::mat4(1.0f);
        modelShader.setMat4("model", model);
        mapModel.Draw(modelShader);

        // render bot
        for(int i=0; i<6; i++){
            if(respawnLeft[i]>=3.99f){
                model = glm::mat4(1.0f);
                glm::mat4 RotationMatrix = glm::toMat4(orientations[i]);
                glm::mat4 TranslationMatrix = translate(model, positions[i]);
                glm::mat4 ScaleMatrix = glm::scale(model, glm::vec3(0.2f));
                glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix * ScaleMatrix;
                modelShader.setMat4("model", ModelMatrix);
                modelShader.setFloat("alpha", 1.0f);
                botModel.Draw(modelShader);
            }
            else{
                model = glm::mat4(1.0f);
                glm::vec3 euler = glm::eulerAngles(orientations[i]) * 3.14159f / 180.f;;
                glm::mat4 RotationMatrix = glm::toMat4(glm::quat(glm::vec3(euler.x, euler.y, euler.z)));
                if(respawnLeft[i]>=2.8f && respawnLeft[i]<=3.0f){
                    float angle = 0.5 * 3.14159f * (3.0f-respawnLeft[i]) / 0.2f;
                    RotationMatrix = glm::toMat4(glm::quat(glm::vec3(euler.x - angle, euler.y, euler.z)));
                }
                else{
                    float angle = 0.5 * 3.14159f *1.0f;
                    RotationMatrix = glm::toMat4(glm::quat(glm::vec3(euler.x - angle, euler.y, euler.z)));
                }
                glm::mat4 TranslationMatrix = translate(model, positions[i]);
                glm::mat4 ScaleMatrix = glm::scale(model, glm::vec3(0.2f));
                glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix * ScaleMatrix;
                modelShader.setMat4("model", ModelMatrix);
                modelShader.setFloat("alpha", 0.3f);
                botModel.Draw(modelShader);
            }
		}
        modelShader.setFloat("alpha", 1.0f);


        // render trans box
        if(showHitbox){
            for(int i=0; i<6; i++){
                model = glm::mat4(1.0f);
                glm::mat4 RotationMatrix = glm::toMat4(orientations[i]);
                glm::mat4 TranslationMatrix = translate(model, glm::vec3(positions[i].x, positions[i].y+1.0f, positions[i].z));
                glm::mat4 ScaleMatrix = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
                glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix * ScaleMatrix;
                modelShader.setMat4("model", ModelMatrix);
                transboxModel.Draw(modelShader);
            }
            for(int i=0; i<6; i++){
                model = glm::mat4(1.0f);
                glm::mat4 RotationMatrix = glm::toMat4(orientations[i]);
                glm::mat4 TranslationMatrix = translate(model, glm::vec3(positions_top[i].x, positions_top[i].y+0.5f, positions_top[i].z));
                glm::mat4 ScaleMatrix = glm::scale(model, glm::vec3(2.0f, 1.0f, 2.0f));
                glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix * ScaleMatrix;
                modelShader.setMat4("model", ModelMatrix);
                transboxModel.Draw(modelShader);
            }
        }

        if(last_ray_direction!=glm::vec3(0,0,0)){
            // align object to the ray, with stretching towards the ray direction
            glm::vec3 ray_direction_normalized = glm::normalize(last_ray_direction);
            glm::vec3 object_direction = ray_direction_normalized;
            glm::vec3 object_up = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
            glm::vec3 object_right = glm::normalize(glm::cross(object_direction, object_up));
            glm::vec3 object_forward = glm::cross(object_right, object_up);
            glm::vec3 object_scale = glm::vec3(0.01f);
            glm::mat4 object_rotation = glm::mat4(1.0f);
            object_rotation = glm::rotate(object_rotation, glm::radians(90.0f), object_up);
            object_rotation = glm::rotate(object_rotation, glm::radians(90.0f), object_right);
            object_rotation = glm::rotate(object_rotation, glm::radians(90.0f), object_forward);
            if(decayAlpha>0.0f){
                for(int i=0; i<1000; i++){
                    model = glm::mat4(1.0f);
                    glm::vec3 object_position = last_ray_origin + 
                                                ray_direction_normalized * (0.5f + 0.02f*i) + 
                                                glm::vec3(0.0f, -0.05f, 0.0f);
                    model = glm::translate(model, object_position);
                    model = model * object_rotation;
                    model = glm::scale(model, object_scale);
                    
                    colorShader.use();
                    colorShader.setMat4("model", model);
                    colorShader.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f));
                    colorShader.setFloat("alpha", decayAlpha);
                    boxModel.Draw(colorShader);
                }
            }
        }

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

        // render crosshair & hit mark
        glDisable(GL_DEPTH_TEST);

        int accuracy = (hitted==0 && missed==0 )? 0 : int(100 * (float(hitted) / float(hitted + missed)));

        Text->RenderText("Killed  : " + std::to_string(hitted), 0.0f, 0.0f, 1.0f, glm::vec3(0.0f, 0.0f, 0.2f));
        Text->RenderText("Missed  : " + std::to_string(missed), 0.0f, 40.0f, 1.0f, glm::vec3(0.0f, 0.0f, 0.2f));
        Text->RenderText("Accuracy: " + std::to_string(accuracy) + "%", 0.0f, 80.0f, 1.0f, glm::vec3(0.0f, 0.0f, 0.2f));

        Text->RenderText("+", (SCR_WIDTH-24.0f)/2, (SCR_HEIGHT-24.0f) / 2, 1.0f, glm::vec3(0.1f, 1.0f, 0.1f));
        if(last_hit_type == 1 && showMarker){
            Text->RenderText("x", (SCR_WIDTH ) / 2 - 27.5, SCR_HEIGHT/ 2 - 35, 2.0f, glm::vec3(0.8f, 0.8f, 0.8f), decayAlpha);
        }
        else if(last_hit_type == 2 && showMarker){
            Text->RenderText("x", (SCR_WIDTH - 48.0f) / 2, (SCR_HEIGHT - 48.0f) / 2, 2.0f, glm::vec3(0.8f, 0.35f, 0.35f), decayAlpha);
        }
        glEnable(GL_DEPTH_TEST);

        // floating point 

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
bool processedKey1 = false;
bool isFirstJump = true;
float jumpStartTime = 0.0f;

float jump(bool jumpOngoing){
    if (jumpOngoing){
        float t = static_cast<float>(glfwGetTime()) - jumpStartTime;
        float v = 5.0f;
        float g = 16.0f;
        float h = 1.8f + v*t - (g*(t*t))/2;
        return h;
    }
    else return 1.8f;
}
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, 3*deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, 3*deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, 3*deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, 3*deltaTime);

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        testx += 0.5f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        testx -= 0.5f;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        testy += 0.5f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        testy -= 0.5f;

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && processedKey1 == false){
        showHitbox = !showHitbox;
        processedKey1 = true;   
    }
    else if (glfwGetKey(window, GLFW_KEY_1) != GLFW_PRESS){
        processedKey1 = false;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        if(isFirstJump){
            jumpStartTime = static_cast<float>(glfwGetTime());
            isFirstJump = false;
        }
    }
    camera.Position.y = jump(!isFirstJump);
    if(camera.Position.y < 1.8f){
        camera.Position.y = 1.8f;
        isFirstJump = true;
    }
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
