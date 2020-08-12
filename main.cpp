#include <HSGIL/hsgil.hpp>

#include <vector>

#include <particle.hpp>

#define SQD(v) pow(v, 2.0f)
#define e gil::constants::E

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
struct SIM_State
{
    GLuint VAO;
    GLuint VBO;
    GLuint stride;
    std::vector<float> vertexData;
    std::vector<Particle> particles;

    float timeStep;
    float restDensity;
    float mass;
    float viscosity;
    float surfaceTension;
    float threshold;
    float gasStiffness;
    float restitution;
    float supportRadius;

    float damping;
    float margin;
    float boundaryWidth;
    float boundaryHeight;
    float boundaryDepth;
};
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
// Kernels
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
float poly6DefaultKernel(const gil::Vec3f r, const float h)
{
    return (315.0f / (64.0f * gil::constants::PI * pow(h, 9.0f))) * pow(SQD(h) - SQD(gil::module(r)), 3.0f);
}

gil::Vec3f poly6GradientKernel(const gil::Vec3f r, const float h)
{
    return -(945.0f / (32.0f * gil::constants::PI * pow(h, 9.0f))) * r * SQD(SQD(h) - SQD(gil::module(r)));
}

float poly6LaplacianKernel(const gil::Vec3f r, const float h)
{
    return -(945.0f / (32.0f * gil::constants::PI * pow(h, 9.0f))) * (SQD(h) - SQD(gil::module(r))) * (3.0f * SQD(h) - 7.0f * SQD(gil::module(r)));
}

gil::Vec3f spikyGradientKernel(const gil::Vec3f r, const float h)
{
    return (-45.0f / (gil::constants::PI * pow(h, 6.0f))) * gil::normalize(r) * SQD(h - gil::module(r));
}

float viscosityLaplacianKernel(const gil::Vec3f r, const float h)
{
    return (45.0f / (gil::constants::PI * pow(h, 6.0f))) * (h - gil::module(r));
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
// Volcano Equations
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
float f(const float x, const float y)
{
    return (sin(2.0f * sqrt(SQD(x) + SQD(y))) / sqrt(SQD(x) + SQD(y))) - 2.0f;
}

float fx(const float x, const float y)
{
    return (2.0f * x * cos(2.0f * sqrt(SQD(x) + SQD(y))) * sqrt(SQD(x) + SQD(y)) - x * sin(2.0f * sqrt(SQD(x) + SQD(y)))) / ((SQD(x) + SQD(y)) * sqrt(SQD(x) + SQD(y)));
}

float fy(const float x, const float y)
{
    return (2.0f * y * cos(2.0f * sqrt(SQD(x) + SQD(y))) * sqrt(SQD(x) + SQD(y)) - y * sin(2.0f * sqrt(SQD(x) + SQD(y)))) / ((SQD(x) + SQD(y)) * sqrt(SQD(x) + SQD(y)));
}

float f3(const float x, const float y)
{
    return 2.0f * (sin(pow(e, fabs(x))) + sin(pow(e, fabs(y)) / 2.0f)) / (pow(e, fabs(x)) + pow(e, fabs(y))) - 2.0f;
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
// Leap-Frog Solver
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
void leapFrogIntegrate(SIM_State& sim)
{
    for(unsigned int i = 0; i < sim.particles.size(); ++i)
    {
        Particle& pi = sim.particles[i];

        pi.v += sim.timeStep * pi.f / pi.density;
        pi.r += sim.timeStep * pi.v;

        // Dumping Method
/*
        if(pi.r.y - sim.margin < f(pi.r.z, pi.r.x))
        {
            pi.v.y *= sim.damping;
            pi.r.y = f(pi.r.z, pi.r.x);
        }
*/

        // Venom Method
/*
        if(pi.r.y - sim.margin < f(pi.r.z, pi.r.x))
        {
            float d {2.0f};
            glm::vec3 A  {pi.r.z, pi.r.x, pi.r.y};
            glm::vec3 Ax {A.x + d, A.y, d * fx(A.x, A.y)};
            glm::vec3 Ay {A.x, A.y + d, d * fy(A.x, A.y)};
            glm::vec3 Dp {glm::normalize(glm::cross(Ax - A, Ay - A))};

            pi.v += gil::Vec3f{Dp.y, Dp.z, Dp.x} * sim.damping;
            pi.r.y = f(pi.r.z, pi.r.x) + sim.margin;
        }
*/

        // Perpendicular Method
/*
        if(pi.r.y - sim.margin < f(pi.r.z, pi.r.x))
        {
            float d {2.0f};
            glm::vec3 A  {pi.r.z, pi.r.x, pi.r.y};
            glm::vec3 Ax {A.x + d, A.y, d * fx(A.x, A.y)};
            glm::vec3 Ay {A.x, A.y + d, d * fy(A.x, A.y)};
            glm::vec3 Dp {glm::normalize(glm::cross(Ay - A, Ax - A))};

            pi.v = gil::Vec3f{Dp.y, Dp.z, Dp.x} * sim.damping;
            pi.r.y = f(pi.r.z, pi.r.x) + sim.margin;
        }
*/

        // Gradient Method

        if(pi.r.y - sim.margin < f(pi.r.z, pi.r.x))
        {
            float d {2.0f};
            gil::Vec3f gradientVector {fy(pi.r.z, pi.r.x), 0.0f, fx(pi.r.z, pi.r.x)};

            pi.v += gil::normalize(gradientVector) * sim.damping;
            pi.r.y = f(pi.r.z, pi.r.x) + sim.margin;
        }


        // Box Method
/*
        if(pi.r.x - sim.margin < 0.0f)
        {
            pi.v.x *= sim.damping;
            pi.r.x = sim.margin;
        }
        if(pi.r.x + sim.margin > sim.boundaryWidth)
        {
            pi.v.x *= sim.damping;
            pi.r.x = sim.boundaryWidth - sim.margin;
        }
        if(pi.r.y - sim.margin < 0.0f)
        {
            pi.v.y *= sim.damping;
            pi.r.y = sim.margin;
        }
        if(pi.r.y + sim.margin > sim.boundaryHeight)
        {
            pi.v.y *= sim.damping;
            pi.r.y = sim.boundaryHeight - sim.margin;
        }
        if(pi.r.z - sim.margin < 0.0f)
        {
            pi.v.z *= sim.damping;
            pi.r.z = sim.margin;
        }
        if(pi.r.z + sim.margin > sim.boundaryDepth)
        {
            pi.v.z *= sim.damping;
            pi.r.z = sim.boundaryDepth - sim.margin;
        }
*/
    }
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------

// Init Functions
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
void initGLParams(const SIM_State& sim, const gil::RenderingWindow& window, gil::Shader& particleShader, gil::Shader& volcanoShader, const glm::vec3& viewPos)
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

    glm::mat4 view = glm::lookAt(viewPos, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f});
    glm::mat4 projection = glm::perspective(45.0f, window.getAspectRatio(), 0.1f, 1000.0f);

    particleShader.use();
    particleShader.setMat4("view", view);
    particleShader.setMat4("projection", projection);
    particleShader.setVec3("particleColor", {1.0f, 0.13f, 0.0f});

    volcanoShader.use();
    volcanoShader.setMat4("view", view);
    volcanoShader.setMat4("projection", projection);
    gil::setupDefaultLights(volcanoShader, viewPos);
}

void initSPH(SIM_State& sim)
{
    Particle p;
    gil::Vec3f pos;
    for(pos.x = sim.margin; pos.x < sim.boundaryWidth * 0.5f; pos.x += sim.supportRadius * 0.6f)
	{
		for(pos.y = sim.margin; pos.y < sim.boundaryHeight * 0.5f; pos.y += sim.supportRadius * 0.6f)
		{
			for(pos.z = sim.margin; pos.z < sim.boundaryDepth * 0.5f; pos.z += sim.supportRadius * 0.6f)
			{
				p.r = pos;
                p.v = {0.0f, 0.0f, 0.0f};
                sim.particles.push_back(std::move(p));

                // Positions
                sim.vertexData.push_back(0.0f);
                sim.vertexData.push_back(0.0f);
                sim.vertexData.push_back(0.0f);
                // Colors
                sim.vertexData.push_back(1.0f);
                sim.vertexData.push_back(0.13f);
                sim.vertexData.push_back(0.0f);
			}
		}
	}
    sim.boundaryWidth  *= 0.6f;
    sim.boundaryHeight *= 0.6f;
    sim.boundaryDepth  *= 0.6f;
    sim.stride = 6;

    glGenVertexArrays(1, &sim.VAO);
    glGenBuffers(1, &sim.VBO);

    glBindVertexArray(sim.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, sim.VBO);
    glBufferData(GL_ARRAY_BUFFER, sim.stride * sim.particles.size() * sizeof(float), sim.vertexData.data(), GL_STATIC_DRAW);

    // Position Attrib
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sim.stride * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Particle Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sim.stride * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    std::cout << "Initialized with " << sim.particles.size() << " particles" << std::endl;
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------

int main()
{
    gil::RenderingWindow window {800, 600, "SPH"};
    if(!window.isReady())
    {
        return EXIT_FAILURE;
    }

    gil::InputControl yRot;

    gil::EventHandler eventHandler;
    eventHandler.addKeyControl(gil::KEY_A, yRot, -1.0f);
    eventHandler.addKeyControl(gil::KEY_D, yRot,  1.0f);

    window.setEventHandler(eventHandler);

    float yRotationAngle  {0.0f};
    float yRotationWeight {1.0f};

    SIM_State sim;

    sim.timeStep = 0.01f;
    sim.restDensity = 998.29f;
    sim.mass = 0.02f;
    sim.viscosity = 3.5f;
    sim.surfaceTension = 0.0728f;
    sim.threshold = 7.065f;
    sim.gasStiffness = 3.0f;
    sim.restitution = 0.0f;
    sim.supportRadius = 0.0457f;

    sim.margin = sim.supportRadius;
    sim.damping = -0.6f;
    sim.boundaryWidth = 0.6f;
    sim.boundaryHeight = 0.6f;
    sim.boundaryDepth = 0.6f;

    initSPH(sim);

    // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
    gil::Timer timer(true);

    //glm::vec3 viewPos {1.0f, 1.0f, 2.0f};
    //glm::vec3 viewPos {0.4f, 0.8f, 1.6f};
    glm::vec3 viewPos {4.0f, 8.0f, 16.0f};

    gil::Shader shader("shader");
    gil::Shader volcanoShader("volcano");
    initGLParams(sim, window, shader, volcanoShader, viewPos);
    // ---------------------------------------------------------------------------------------------------------------------------------------------------------------

    gil::Model volcano("models/volcano.obj", nullptr, true, false);
    glm::vec3 volcanoPos {0.0f, -2.0f, 0.0f};

    while(window.isActive())
    {
        window.pollEvents();

        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Compute Mass-Density and Pressure
        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        for(unsigned int i = 0; i < sim.particles.size(); ++i)
        {
            Particle& pi = sim.particles[i];

            pi.density = 0;
            for(unsigned int j = 0; j < sim.particles.size(); ++j)
            {
                Particle& pj = sim.particles[j];
                gil::Vec3f r {pi.r - pj.r};

                if(gil::module(r) < sim.supportRadius)
                {
                    pi.density += sim.mass * poly6DefaultKernel(r, sim.supportRadius);
                }
            }
            pi.pressure = sim.gasStiffness * (sim.particles[i].density - sim.restDensity);
            // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        }

        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Compute Internal and External Forces
        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        for(unsigned int i = 0; i < sim.particles.size(); ++i)
        {
            Particle& pi = sim.particles[i];

            gil::Vec3f pressureForce  {0.0f, 0.0f, 0.0f};
            gil::Vec3f viscosityForce {0.0f, 0.0f, 0.0f};
            gil::Vec3f sfTensionForce {0.0f, 0.0f, 0.0f};
            gil::Vec3f gravityForce   {0.0f, -gil::constants::GAL, 0.0f};

            float colorLaplacian {0.0f};
            gil::Vec3f surfaceNormal {0.0f, 0.0f, 0.0f};

            for(unsigned int j = 0; j < sim.particles.size(); ++j)
            {
                if(&pi == &sim.particles[j])
                {
                    continue;
                }

                Particle& pj = sim.particles[j];
                gil::Vec3f r {pi.r - pj.r};

                if(gil::module(r) < sim.supportRadius)
                {
                    pressureForce  += ((pi.pressure / SQD(pi.density)) + (pj.pressure / SQD(pj.density))) * sim.mass * spikyGradientKernel(r, sim.supportRadius);
                    viscosityForce += (pj.v - pi.v) * (sim.mass / pj.density) * viscosityLaplacianKernel(r, sim.supportRadius);
                    surfaceNormal  += (sim.mass / pj.density) * poly6GradientKernel(r, sim.supportRadius);
                    colorLaplacian += (sim.mass / pj.density) * poly6LaplacianKernel(r, sim.supportRadius);
                }
            }
            pressureForce  *= -pi.density;
            viscosityForce *= sim.viscosity;
            gravityForce *= sim.restDensity;

            if(gil::module(surfaceNormal) >= sim.threshold)
            {
                sfTensionForce = -sim.surfaceTension * colorLaplacian * gil::normalize(surfaceNormal);
            }
            pi.f = pressureForce + viscosityForce + gravityForce + sfTensionForce;
        }
        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------

        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Integrate by Euler 1th Order
        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        leapFrogIntegrate(sim);
        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------

        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Render
        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view;
        glm::mat4 model;

        float deltaTime = timer.getDeltaTime();
        yRotationAngle += yRot.getMagnitude() * yRotationWeight * deltaTime;
        glm::vec4 nvp4 = glm::rotate(glm::mat4(1.0f), yRotationAngle, glm::vec3{0.0f, 1.0f, 0.0f}) * glm::vec4{viewPos.x, viewPos.y, viewPos.z, 1.0f};
        glm::vec3 nvp3 = {nvp4.x, nvp4.y, nvp4.z};
        view = glm::lookAt(nvp3, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f});

        model = glm::translate(glm::mat4(1.0f), volcanoPos);
        volcanoShader.use();
        volcanoShader.setMat4("view", view);
        volcanoShader.setMat4("model", model);
        volcano.draw(volcanoShader);

        shader.use();
        shader.setMat4("view", view);
        for(unsigned int i = 0; i < sim.particles.size(); ++i)
        {
            Particle& pi = sim.particles[i];

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3{pi.r.x, pi.r.y, pi.r.z});
            shader.setMat4("model", model);

            float zColorDepth {pi.r.z / sim.boundaryDepth};
            shader.setVec3("colorDepth", {zColorDepth, zColorDepth, zColorDepth});

            glBindVertexArray(sim.VAO);
                glDrawArrays(GL_POINTS, 0, sim.particles.size());
            glBindVertexArray(0);
        }

        window.swapBuffers();
        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------

        timer.tick();
    }

    glDeleteVertexArrays(1, &sim.VAO);
    glDeleteBuffers(1, &sim.VBO);

    return 0;
}
