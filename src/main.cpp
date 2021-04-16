/*
 Base code
 Currently will make 2 FBOs and textures (only uses one in base code)
 and writes out frame as a .png (Texture_output.png)
 
 Winter 2017 - ZJW (Piddington texture write)
 2017 integration with pitch and yaw camera lab (set up for texture mapping lab)
 */

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "WindowManager.h"
#include "GLTextureWriter.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

class Application : public EventCallbacks
{
   
public:
   
   WindowManager * windowManager = nullptr;
   
   // Our shader program
   std::shared_ptr<Program> prog;
   std::shared_ptr<Program> texProg;
 
   std::shared_ptr<Texture> htmap;
   std::shared_ptr<Texture> terrain;

   // Shape to be used (from obj file)
   shared_ptr<Shape> shape;
   shared_ptr<Shape> shape2;
   
   // Contains vertex information for OpenGL
   GLuint VertexArrayID;
   
   // Data necessary to give our triangle to OpenGL
   GLuint VertexBufferID;
   
   //geometry for texture render
   GLuint quad_VertexArrayID;
   GLuint quad_vertexbuffer;
  
   GLuint screenTex;
   
   //reference to texture FBO
   GLuint frameBuf;
   GLuint depthBuf;
   
   bool left = false;
   bool right = false;
   bool forward = false;
   bool back = false;
   bool up = false;
   bool down = false;
   bool fast = false;
   
   float sTheta = 0;
   int transX = 0;
   float elbowTheta = 0;
   float shoulderTheta = 0;
   int waving = 1;
   int elbowRotate = 0;
   int shoulderRotate = 0;
   
   float charX = 0;
   float charY = 0;
   float charZ = 0;
   
   int winX = 512;
   int winY = 512;

   int sideLength = 1024;

   const int size = 1024;

   int vertCount = 0;

   float heightscale = 300.0;
   float worldscale = 1000.0;

   float downForce = 0;
   float gravity = .0981;

   float cTheta = 0;
   float cPhi = 0;
   
   int writeOnce = 1;
   unsigned char *pixels;
   
   void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
   {
      if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      {
         glfwSetWindowShouldClose(window, GL_TRUE);
      }
      else if (key == GLFW_KEY_A && action == GLFW_PRESS)
      {
         left = true;
      }
      else if (key == GLFW_KEY_D && action == GLFW_PRESS)
      {
         right = true;
      }
      else if (key == GLFW_KEY_A && action == GLFW_RELEASE)
      {
         left = false;
      }
      else if (key == GLFW_KEY_D && action == GLFW_RELEASE)
      {
         right = false;
      }
      else if (key == GLFW_KEY_W && action == GLFW_PRESS)
      {
         forward = true;
      }
      else if (key == GLFW_KEY_S && action == GLFW_PRESS)
      {
         back = true;
      }
      else if (key == GLFW_KEY_W && action == GLFW_RELEASE)
      {
         forward = false;
      }
      else if (key == GLFW_KEY_S && action == GLFW_RELEASE)
      {
         back = false;
      }
      else if (key == GLFW_KEY_P && action == GLFW_PRESS)
      {
         fast = true;
      }
      else if (key == GLFW_KEY_P && action == GLFW_RELEASE)
      {
         fast = false;
      }
      else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
      {
         up = true;
      }
      else if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
      {
         up = false;
      }
      else if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
      {
         down = true;
      }
      else if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
      {
         down = false;
      }
      else if (key == GLFW_KEY_G && action == GLFW_PRESS)
      {
         if(gravity == 0)
         {
            gravity = .0981;
         }
         else
         {
            gravity = 0;
         }
      }

      
   }
   
   void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
   {
      cTheta += (float) deltaX;
   }
   
   void motionCallback(GLFWwindow* window, double deltaX, double deltaY)
   {
      cTheta = (float) deltaX;
      
      cPhi = (float) deltaY;
      
      cPhi = winY - cPhi;
      
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
   }
   
   
   void mouseCallback(GLFWwindow *window, int button, int action, int mods)
   {
   }
   
   void resizeCallback(GLFWwindow *window, int width, int height)
   {
      glViewport(0, 0, width, height);
   }
   
   void init(const std::string& resourceDirectory)
   {
      int width, height;
      glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
      GLSL::checkVersion();
      
      cTheta = 0;
      // Set background color.
      glClearColor(.12f, .34f, .56f, 1.0f);
      // Enable z-buffer test.
      glEnable(GL_DEPTH_TEST);
      
      // Initialize the GLSL program.
      prog = make_shared<Program>();
      prog->setVerbose(true);
      prog->setShaderNames(
                           resourceDirectory + "/simple_vert.glsl",
                           resourceDirectory + "/simple_frag.glsl");
      if (! prog->init())
      {
         std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
         exit(1);
      }
      prog->addUniform("P");
      prog->addUniform("MV");
      prog->addUniform("MatAmb");
      prog->addUniform("MatDif");
      prog->addUniform("shine");
      prog->addUniform("MatSpec");
      prog->addUniform("lightPos");
      prog->addAttribute("vertPos");
      prog->addAttribute("vertNor");
      
      //create two frame buffer objects to toggle between
      glGenFramebuffers(1, &frameBuf);
      glGenTextures(1, &screenTex);
      glGenRenderbuffers(1, &depthBuf);
      createFBO(frameBuf, screenTex);
      
      //set up depth necessary as rendering a mesh that needs depth test
      glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);
      
      //more FBO set up
      GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
      glDrawBuffers(1, DrawBuffers);
      
      
      htmap = make_shared<Texture>();
      htmap->setFilename(resourceDirectory + "/heightmap.jpg");
      htmap->init();
      pixels = new unsigned char[size * size];
      glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
      glBindTexture(GL_TEXTURE_2D, 0);
      htmap->setUnit(0);
      htmap->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      
      terrain = make_shared<Texture>();
      terrain->setFilename(resourceDirectory + "/terrainmap.jpg");
      terrain->init();
      glBindTexture(GL_TEXTURE_2D, 0);
      terrain->setUnit(1);
      terrain->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);



      //set up the shaders to blur the FBO just a placeholder pass thru now
      //next lab modify and possibly add other shaders to complete blur
      texProg = make_shared<Program>();
      texProg->setVerbose(true);
      texProg->setShaderNames(
                              resourceDirectory + "/pass_vert.glsl",
                              resourceDirectory + "/tex_fragH.glsl");
      if (! texProg->init())
      {
         std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
         exit(1);
      }
      texProg->addUniform("P");
      texProg->addUniform("MV");
      texProg->addUniform("MatAmb");
      texProg->addUniform("MatDif");
      texProg->addUniform("shine");
      texProg->addUniform("MatSpec");
      texProg->addUniform("lightPos");
      texProg->addUniform("htmap");
      texProg->addUniform("terrain");
      texProg->addUniform("heightscale");
      texProg->addAttribute("vertPos");
      texProg->addAttribute("vertNor");
      
   }
   
   void initGeom(const std::string& resourceDirectory)
   {
      // Initialize the obj mesh VBOs etc
      shape = make_shared<Shape>();
      shape->loadMesh(resourceDirectory + "/bunny.obj");
      shape->resize();
      shape->init();
      shape2 = make_shared<Shape>();
      shape2->loadMesh(resourceDirectory + "/cube.obj");
      shape2->resize();
      shape2->init();


      glGenVertexArrays(1, &VertexArrayID);
      glBindVertexArray(VertexArrayID);
      glGenBuffers(1, &VertexBufferID);
      glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

      std::vector<GLfloat> mesh = makeMesh(sideLength);

      glBufferData(GL_ARRAY_BUFFER, mesh.size() * sizeof(GLfloat), mesh.data(), GL_DYNAMIC_DRAW);

      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

      glBindVertexArray(0);


      //Initialize the geometry to render a quad to the screen
      initQuad();
   }

   //Mesh will be drawn using triangle strips
   //
   std::vector<GLfloat> makeMesh(int sideLength)
   {
      std::vector<GLfloat> verts;
      const GLfloat delta = 1./sideLength;
      GLfloat dx = 0;
      GLfloat dz = 0;

      int rowBase = 1 - sideLength;
      int rowCap = sideLength - 1;
      int i = rowBase;
      int j = rowBase;
   
      for(i = rowBase; i <= rowCap; i++)
      {
         dx = delta * (float)(i);
         
         for(j = rowBase; j <= rowCap; j++)
         {
            dz = delta * (float)(j);
            if(i > rowBase && j == rowBase )
            {
               //first vert of next row
               verts.push_back(dx - delta);
               verts.push_back(dz - delta);
               verts.push_back(0.f);
               vertCount++;
            }
               
            verts.push_back(dx - delta);
            verts.push_back(dz - delta);
            verts.push_back(0.f);
            vertCount++;

            verts.push_back(dx);
            verts.push_back(dz - delta);
            verts.push_back(0.f);
            vertCount++;

            verts.push_back(dx - delta);
            verts.push_back(dz);
            verts.push_back(0.f);
            vertCount++;
  
            verts.push_back(dx);
            verts.push_back(dz);
            verts.push_back(0.f);
            vertCount++;
         
            if(i < rowCap  && j == rowCap )
            {
               verts.push_back(dx);
               verts.push_back(dz);
               verts.push_back(0.f); 
               vertCount++;
            }
         }
      }
      return verts;
   }
   
   /**** geometry set up for a quad *****/
   void initQuad()
   {
      //now set up a simple quad for rendering FBO
      glGenVertexArrays(1, &quad_VertexArrayID);
      glBindVertexArray(quad_VertexArrayID);
      
      static const GLfloat g_quad_vertex_buffer_data[] =
      {
         -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         -1.0f,  1.0f, 0.0f,
         -1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
      };
      
      glGenBuffers(1, &quad_vertexbuffer);
      glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
      glBindVertexArray(0);
   }
   
   /* Helper function to create the framebuffer object and
    associated texture to write to */
   void createFBO(GLuint& fb, GLuint& tex)
   {
      //initialize FBO
      int width, height;
      glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
      
      //set up framebuffer
      glBindFramebuffer(GL_FRAMEBUFFER, fb);
      //set up texture
      glBindTexture(GL_TEXTURE_2D, tex);
      
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
      
      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      {
         cout << "Error setting up frame buffer - exiting" << endl;
         exit(0);
      }
   }
   
   void render()
   {
         // Get current frame buffer size.
         int width, height;
         glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
         glViewport(0, 0, width, height);
         
         float dist = 1;
         
         if(fast)
         {
            dist = 10;
         }
         
         glBindFramebuffer(GL_FRAMEBUFFER, 0);
         prog->bind();
         // Clear framebuffer.
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
         
         /* Leave this code to just draw the meshes alone */
         float aspect = width/(float)height;
         
         // Create the matrix stacks
         auto P = make_shared<MatrixStack>();
         auto MV = make_shared<MatrixStack>();
         // Apply perspective projection.
         P->pushMatrix();
         P->perspective(45.0f, aspect, 1.f, 5000.0f);
         
         //Draw our scene - two meshes - right now to a texture
         glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
         // globl transforms for 'camera' (you will fix this now!)
         MV->pushMatrix();
         MV->loadIdentity();
         
         //Camera direction
         float x, y, z;
         float phi, angle;
         float radius = 1;
         
         //Light source position
         float lx = 10000;
         float ly = 10000;
         float lz = 5000;
         
         //Calculate lookat angles
         angle = ((cTheta/winX * 2) - 1);
         phi = ((cPhi/winY * 2) - 1);
         if(phi < radians(-89.9))
         {
            phi = radians(-89.9);
         }
         if(phi > radians(89.9))
         {
            phi = radians(89.9);
         }

         
         //Calculate camera position
         if(left)
         {
            charX += (dist * sin(angle));
            charZ -= (dist * cos(angle));
         }
         if(right)
         {
            charX -= (dist * sin(angle));
            charZ += (dist * cos(angle));
         }
         if(forward)
         {
            charX += (dist * cos(angle));
            charZ += (dist * sin(angle));
         }
         if(back)
         {
            charX -= (dist * cos(angle));
            charZ -= (dist * sin(angle));
         }
         if(up)
         {
            downForce = dist;
         }
         else
         {
            if(charY >= 0)
            {
               downForce -= gravity;
            }
            else
            {
               downForce = 0;
            }
         }
         if(down)
         {
            downForce = -dist;
         }
            
         charY += downForce;
         //cout << downForce << "==>";
         //cout << charY << endl;
          
         //Calculate lookat position
         x = radius*cos(phi)*cos(angle);
         y = radius*sin(phi);
         z = radius*cos(phi)*sin(angle);
         
         unsigned int yFloor = 0;
         
         //Bind heightmap to calculate floor minimum
         glBindVertexArray(VertexArrayID);
         glUniform3f(prog->getUniform("lightPos"), lx, ly, lz);
         texProg->bind();
         htmap->bind(texProg->getUniform("htmap"));

         //Translate current world coords into nearest texel coords
         int adjX = ((((int) charX) + worldscale) / (2 * worldscale)) * (size);
         if(adjX >= size)
         {
            adjX = size - 1;
         }
         else if(adjX < 0)
         {
            adjX = 0;
         }

         int adjZ = ((((int) charZ) + worldscale) / (2 * worldscale)) * (size);
         if(adjZ >= size)
         {
            adjZ = size - 1;
         }
         else if(adjZ < 0)
         {
            adjZ = 0;
         }

         int X, Z;

         Z = adjX;
         X = size - adjZ;

         
         yFloor = pixels[X * size + Z];

         yFloor = (yFloor / 255.0) * heightscale;

         if(up)
         {
            downForce = dist;
         }
         else
         {
            if(charY >= yFloor)
            {
               downForce -= gravity;
            }
            else
            {
               downForce = 0;
            }
         }
         if(down)
         {
            downForce = 0;
         }
            
         charY += downForce;

         if(charY < yFloor)
         {
            charY = yFloor;
         }

         MV->lookAt(vec3(charX, charY, charZ), 
                    vec3(charX, charY, charZ) + vec3(x, y, z),
                    vec3(0, 1, 0));
         MV->translate(vec3(0, heightscale * -.15, 0));
         MV->rotate(radians(-90.), vec3(1, 0, 0));
         MV->scale(vec3(worldscale, worldscale, 1));

         glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE,value_ptr(P->topMatrix()));
         glUniformMatrix4fv(texProg->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()));
         glUniform1f(texProg->getUniform("heightscale"), heightscale);
         terrain->bind(texProg->getUniform("terrain"));

         glDrawArrays(GL_TRIANGLE_STRIP, 0, vertCount);

         texProg->unbind();
         glBindVertexArray(0);
         
         P->popMatrix();
   }
   
   void setMaterial(int i)
   {
      switch (i)
      {
         case 0: // shiny blue plastic
            glUniform3f(prog->getUniform("MatAmb"), 0.02, 0.04, 0.2);
            glUniform3f(prog->getUniform("MatDif"), 0.0, 0.16, 0.9);
            glUniform3f(prog->getUniform("MatSpec"), 0.14, 0.2, 0.8);
            glUniform1f(prog->getUniform("shine"), 120.0);
            break;
         case 1: // flat grey
            glUniform3f(prog->getUniform("MatAmb"), 0.13, 0.13, 0.14);
            glUniform3f(prog->getUniform("MatDif"), 0.3, 0.3, 0.4);
            glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.3, 0.4);
            glUniform1f(prog->getUniform("shine"), 4.0);
            break;
         case 2: // brass
            glUniform3f(prog->getUniform("MatAmb"), 0.3294, 0.2235, 0.02745);
            glUniform3f(prog->getUniform("MatDif"), 0.7804, 0.5686, 0.11373);
            glUniform3f(prog->getUniform("MatSpec"), 0.9922, 0.941176, 0.80784);
            glUniform1f(prog->getUniform("shine"), 27.9);
            break;
         case 3: // tar
            glUniform3f(prog->getUniform("MatAmb"), 0.0, 0.0, 0.0);
            glUniform3f(prog->getUniform("MatDif"), 0.0, 0.0, 0.0);
            glUniform3f(prog->getUniform("MatSpec"), 0.6, 0.6, 0.6);
            glUniform1f(prog->getUniform("shine"), 19.0);
            break;
         case 4: // copper
            glUniform3f(prog->getUniform("MatAmb"), 0.3, 0.1, 0.0);
            glUniform3f(prog->getUniform("MatDif"), 0.5, 0.2, 0.0);
            glUniform3f(prog->getUniform("MatSpec"), 0.9, 0.5, 0.0);
            glUniform1f(prog->getUniform("shine"), 10.0);
            break;
         case 5: // chrome
            glUniform3f(prog->getUniform("MatAmb"), 0.2, 0.2, 0.2);
            glUniform3f(prog->getUniform("MatDif"), 0.25, 0.25, 0.25);
            glUniform3f(prog->getUniform("MatSpec"), 0.65, 0.65, 0.65);
            glUniform1f(prog->getUniform("shine"), 5.0);
            break;
         case 6: // grass
            glUniform3f(prog->getUniform("MatAmb"), 0.1, 0.2, 0.0);
            glUniform3f(prog->getUniform("MatDif"), 0.2, 0.5, 0.0);
            glUniform3f(prog->getUniform("MatSpec"), 0.0, 0.0, 0.0);
            glUniform1f(prog->getUniform("shine"), 0.0);
            break;
      }
      
   }
};

int main(int argc, char **argv)
{
   // Where the resources are loaded from
   std::string resourceDir = "../resources";
   
   if (argc >= 2)
   {
      resourceDir = argv[1];
   }
   
   Application *application = new Application();
   
   // Your main will always include a similar set up to establish your window
   // and GL context, etc.
   
   WindowManager *windowManager = new WindowManager();
   windowManager->init(1024, 700);
   
   windowManager->setEventCallbacks(application);
   application->windowManager = windowManager;
   
   // This is the code that will likely change program to program as you
   // may need to initialize or set up different data and state
   
   application->init(resourceDir);
   application->initGeom(resourceDir);
   
   // Loop until the user closes the window.
   while (! glfwWindowShouldClose(windowManager->getHandle()))
   {
      // Render scene.
      application->render();
      
      // Swap front and back buffers.
      glfwSwapBuffers(windowManager->getHandle());
      // Poll for and process events.
      glfwPollEvents();
   }
   
   // Quit program.
   windowManager->shutdown();
   return 0;
}
