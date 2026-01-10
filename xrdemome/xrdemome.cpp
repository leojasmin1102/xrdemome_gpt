// xrdemome.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//circlecicle butterfly

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <cstdlib>
#include <ctime>


// 1. 顶点着色器源码 (GLSL语言)
// 作用：告诉显卡，顶点在哪里
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in float aStress;\n"
 "out float vStress; \n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"vStress = aStress;\n"
"   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"}\0";

// 2. 片段着色器源码 (GLSL语言)
// 作用：告诉显卡，像素涂什么颜色

const char* fragmentShaderSource = "#version 330 core\n"
"in float vStress;\n"
"out vec4 FragColor;\n"
"uniform sampler1D colormap;\n"
"void main()\n"
"{\n"
"    float s = clamp(vStress, 0.0, 1.0);\n"
"    vec3 color = texture(colormap, s).rgb;\n"
"    FragColor = vec4(color, 1.0);\n"
"}\n";


struct CAEVertex
{
    glm::vec3 position;
    float stress;
};

inline float clampf(float x, float minVal, float maxVal)
{
    if (x < minVal) return minVal;
    if (x > maxVal) return maxVal;
    return x;
}


int main() {
    // --- 初始化 GLFW 和 窗口 (和你之前一样) ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello Triangle", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);


    // --- 编译着色器 (这是显卡能听懂的程序) ---
    // A. 编译顶点着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // (为了代码简洁，这里省略了编译错误检查，实际开发中需要加)

    // B. 编译片段着色器
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // C. 链接成一个着色器程序对象
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // 用完后删除中间件
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 在cpu侧生成1万个点 （VAO、VBO将会使用到）
    std::vector<CAEVertex> vertices;

    srand((unsigned int)time(nullptr));

    const int POINT_COUNT = 10000;

    for (int i = 0; i < POINT_COUNT; ++i) {
        CAEVertex v;

        // 随机分布在一个立方体里
        v.position = glm::vec3(
            ((rand() % 1000) / 500.0f - 1.0f),
            ((rand() % 1000) / 500.0f - 1.0f),
            ((rand() % 1000) / 500.0f - 1.0f)
        );

        // 模拟应力值（0~1）
        v.stress = (rand() % 1000) / 1000.0f;

        vertices.push_back(v);
    }

    // --- 核心步骤：VAO 和 VBO ---
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO); // 创建 VAO (管理状态)
    glGenBuffers(1, &VBO);      // 创建 VBO (存放数据)

    // 1. 绑定 VAO
    glBindVertexArray(VAO);

    // 2. 把顶点数据复制到缓冲中供 OpenGL 使用
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    //0.0.1 VBO上传数据 
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(CAEVertex), vertices.data(), GL_STATIC_DRAW);

    // 3. 告诉 OpenGL 怎么解析这些数据
    // 参数解释：0号属性, 3个值(xyz), float类型, 不标准化, 步长(3*float), 起始偏移量0

    //0.0.1 告诉 OpenGL 如何解析这个结构体:
    // position
      glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(CAEVertex), (void*)0);
      glEnableVertexAttribArray(0);
    // stress 
      glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(CAEVertex), (void*)offsetof(CAEVertex, stress));
      glEnableVertexAttribArray(1);


      // ----------------------------
  // 3.x 生成 Colormap（CPU）
  // ----------------------------
      std::vector<float> colormap;

      for (int i = 0; i < 256; ++i)
      {
          float t = i / 255.0f;

          float r = clampf(1.5f - fabs(4.0f * t - 3.0f), 0.0f, 1.0f);
          float g = clampf(1.5f - fabs(4.0f * t - 2.0f), 0.0f, 1.0f);
          float b = clampf(1.5f - fabs(4.0f * t - 1.0f), 0.0f, 1.0f);

          colormap.push_back(r);
          colormap.push_back(g);
          colormap.push_back(b);
      }

      // ----------------------------
      // 3.y 创建 1D 纹理（GPU）
      // ----------------------------
      GLuint colormapTex;
      //glActiveTexture(GL_TEXTURE0);
      glGenTextures(1, &colormapTex);
      glBindTexture(GL_TEXTURE_1D, colormapTex);

      glTexImage1D(
          GL_TEXTURE_1D,
          0,
          GL_RGB32F,
          256,
          0,
          GL_RGB,
          GL_FLOAT,
          colormap.data()
      );

      glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

      glUseProgram(shaderProgram);
      glUniform1i(glGetUniformLocation(shaderProgram, "colormap"), 0);


    
    // --- 渲染循环 ---
    while (!glfwWindowShouldClose(window)) {
        // [1] 输入处理
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // [2] 清空画布（颜色和深度缓冲区一次性清空）
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // [3] 激活程序（必须在设置 uniform 之前或过程中激活）
        glUseProgram(shaderProgram);

        // [4] 计算矩阵并传值
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_1D, colormapTex);


        // [5] 绘制
        glBindVertexArray(VAO);

        //0.0.1 点云
        glPointSize(4.0f);
        glDrawArrays(GL_POINTS, 0, POINT_COUNT);

        // [6] 交换缓冲
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ---释放资源---
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}

