#include <HSGIL/hsgil.hpp>

#include <vector>

#include <particle.hpp>

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
    float gasStiffness;
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
    return (315.0f / (64.0f * gil::constants::PI * pow(h, 9.0f))) * pow(h * h - pow(gil::module(r), 2.0f), 3.0f);
}

gil::Vec3f spikyGradientKernel(const gil::Vec3f r, const float h)
{
    return (-45.0f / (gil::constants::PI * pow(h, 6.0f))) * gil::normalize(r) * pow(h - gil::module(r), 2.0f);
}

float viscosityLaplacianKernel(const gil::Vec3f r, const float h)
{
    return (45.0f / (gil::constants::PI * pow(h, 6.0f))) * (h - gil::module(r));
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
// Euler Solver
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
void eulerIntegrate(SIM_State& sim)
{
    for(unsigned int i = 0; i < sim.particles.size(); ++i)
    {
        Particle& pi = sim.particles[i];

        pi.v += sim.timeStep * pi.f / pi.density;
        pi.r += sim.timeStep * pi.v;

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
    }
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------

// Init Functions
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
void initGLParams(const SIM_State& sim, const gil::RenderingWindow& window, gil::Shader& shader)
{
    shader.use();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    shader.setFloat("pointSize", 32.0f);

    //glm::vec3 viewPos {1.0f, 1.0f, 2.0f};
    glm::vec3 viewPos {0.4f, 0.8f, 1.6f};
    glm::mat4 view = glm::lookAt(viewPos, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f});
    glm::mat4 projection = glm::perspective(45.0f, window.getAspectRatio(), 0.1f, 1000.0f);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setVec3("particleColor", {1.0f, 0.13f, 0.0f});
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
                p.color = {1.0f, 0.13f, 0.0f};
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

    SIM_State sim;

    sim.timeStep = 0.01f;
    sim.restDensity = 998.29f;
    sim.mass = 0.02f;
    sim.viscosity = 3.5f;
    sim.gasStiffness = 3.0f;
    sim.supportRadius = 0.0457f;

    sim.margin = sim.supportRadius;
    sim.damping = -0.5f;
    sim.boundaryWidth = 0.5f;
    sim.boundaryHeight = 0.5f;
    sim.boundaryDepth = 0.5f;

    initSPH(sim);

    // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
    gil::Timer timer(true);

    gil::Shader shader("shader");
    initGLParams(sim, window, shader);

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
            gil::Vec3f gravityForce   {0.0f, -gil::constants::GAL, 0.0f};

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
                    pressureForce  += ((pi.pressure / pow(pi.density, 2)) + (pj.pressure / pow(pj.density, 2))) * sim.mass * spikyGradientKernel(r, sim.supportRadius);
                    viscosityForce += (pj.v - pi.v) * (sim.mass / pj.density) * viscosityLaplacianKernel(r, sim.supportRadius);
                }
            }
            pressureForce  *= -pi.density;
            viscosityForce *= sim.viscosity;
            gravityForce *= sim.restDensity;
            pi.f = pressureForce + viscosityForce + gravityForce;
        }
        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------

        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Integrate by Euler 1th Order
        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        timer.getDeltaTime();
        eulerIntegrate(sim);
        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------

        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Render
        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        shader.use();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model;
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
