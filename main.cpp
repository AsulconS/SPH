#include <HSGIL/hsgil.hpp>
#include <particle.hpp>

#include <random>

#define SCALE_FACTOR 10.0f

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
struct SIM_State
{
    GLuint VAO;
    GLuint VBO;
    float* vertexData;

    Particle* particles;
    unsigned int nParticles;

    float density;
    float gasConstant;

    float h;
    float h2;

    float mass;
    float viscosity;

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
float W(const float q, const float h)
{
    return (315.0f * q) / (65.0f * gil::constants::PI * pow(h, 9.0f));
}

float W1(const float q, const float h)
{
    return (-45.0f * q) / (gil::constants::PI * pow(h, 6.0f));
}

float W2(const float q, const float h)
{
    return (45.0f * q) / (gil::constants::PI * pow(h, 6.0f));
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
// Euler Solver
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
void eulerIntegrate(const SIM_State& sim, const float step)
{
    for(unsigned int i = 0; i < sim.nParticles; ++i)
    {
        Particle& pi = sim.particles[i];

        pi.v += step * pi.f / pi.density;
        pi.r += step * pi.v;

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
        /*
        if(pi.r.y + sim.margin > sim.boundaryHeight)
        {
            pi.v.y *= sim.damping;
            pi.r.y = sim.boundaryHeight - sim.margin;
        }
        */
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
void initGLParams(const SIM_State& sim, gil::RenderingWindow& window, gil::Shader& shader) // Fix this Internally (const Window)
{
    shader.use();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    shader.setFloat("pointSize", 32.0f);

    glm::vec3 viewPos {8.0f, 16.0f, 32.0f};
    glm::mat4 view = glm::lookAt(viewPos, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f});
    glm::mat4 projection = glm::perspective(45.0f, window.getAspectRatio(), 0.1f, 1000.0f);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setVec3("particleColor", {1.0f, 0.13f, 0.0f});
}

void initSPH(SIM_State& sim)
{
    sim.vertexData = new float[6 * sim.nParticles];
    sim.particles = new Particle[sim.nParticles];

    gil::Vec3f pos;
    unsigned int p {0};
    for(pos.x = sim.boundaryWidth * 0.0f; pos.x < sim.boundaryWidth * 0.5f; pos.x += sim.h * 0.6f)
	{
		for (pos.y = sim.boundaryHeight * 0.0f; pos.y < sim.boundaryHeight * 0.5f; pos.y += sim.h * 0.6f)
		{
			for (pos.z = sim.boundaryDepth * 0.0f; pos.z < sim.boundaryDepth * 0.5f; pos.z += sim.h * 0.6f)
			{
				sim.particles[p].r = pos;
                sim.particles[p].v = {8.0f, 32.0f, 8.0f};
                sim.particles[p].color = {1.0f, 0.13f, 0.0f};
                sim.vertexData[p * 3 + 0] = 1.0f;
                sim.vertexData[p * 3 + 1] = 0.13f;
                sim.vertexData[p * 3 + 2] = 0.0f;
                ++p;
			}
		}
	}
    sim.nParticles = p;

    glGenVertexArrays(1, &sim.VAO);
    glGenBuffers(1, &sim.VBO);

    glBindVertexArray(sim.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, sim.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sim.vertexData), sim.vertexData, GL_STATIC_DRAW);

    // Position Attrib
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Particle Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    std::cout << "Initialized with " << sim.nParticles << " particles" << std::endl;
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
    sim.nParticles = 100000;

    sim.density = 1000.0f;
    sim.gasConstant = 2000.0f;

    sim.h = 16.0f;
    sim.h2 = sim.h * sim.h;

    sim.mass = 64.0f;
    sim.viscosity = 2500.0f;

    sim.margin = sim.h;
    sim.damping = -0.125f;
    sim.boundaryWidth = 160.0f;
    sim.boundaryHeight = 160.0f;
    sim.boundaryDepth = 160.0f;

    initSPH(sim);

    // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
    gil::Timer timer(true);

    gil::Shader shader("shader");
    initGLParams(sim, window, shader);

    while(window.isActive())
    {
        window.pollEvents();

        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Compute Density-Pressure
        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        for(unsigned int i = 0; i < sim.nParticles; ++i)
        {
            Particle& pi = sim.particles[i];

            pi.density = 0;
            for(unsigned int j = 0; j < sim.nParticles; ++j)
            {
                Particle& pj = sim.particles[j];
                float r2 {gil::module(pj.r - pi.r) * gil::module(pj.r - pi.r)};

                if(r2 < sim.h2)
                {
                    pi.density += sim.mass * W((sim.h2 - r2) * (sim.h2 - r2) * (sim.h2 - r2), sim.h);
                }
                // ^ This makes it better (I don't know why profe :'v) ...sim.mass * W(r, sim.h)... antigua version
            }
            pi.density += 64.0f;
            pi.pressure = sim.gasConstant * (sim.particles[i].density - sim.density);
            // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        }

        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Compute Forces
        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        for(unsigned int i = 0; i < sim.nParticles; ++i)
        {
            Particle& pi = sim.particles[i];

            gil::Vec3f fp {0.0f, 0.0f, 0.0f};
            gil::Vec3f fv {0.0f, 0.0f, 0.0f};

            for(unsigned int j = 0; j < sim.nParticles; ++j)
            {
                if(&pi == &sim.particles[j])
                {
                    continue;
                }

                Particle& pj = sim.particles[j];
                float r {gil::module(pj.r - pi.r)};

                if(r < sim.h)
                {
                    fp += -1.0f * gil::normalize(pj.r - pi.r) * sim.mass * (pi.pressure + pj.pressure) / (2.0f * pj.density) * W1((sim.h - r) * (sim.h - r), sim.h); // <- Lo mismo aqui
                    fv += sim.viscosity * sim.mass * ((pj.v - pi.v) / pj.density) * W2(sim.h - r, sim.h);
                }
            }

            gil::Vec3f g {0.0f, -gil::constants::GAL, 0.0f};
            gil::Vec3f fg {g * pi.density};
            pi.f = fp + fv + fg;
        }
        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------

        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Integrate by Euler 1th Order
        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        eulerIntegrate(sim, timer.getDeltaTime());
        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------

        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Render
        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------
        shader.use();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model;
        for(unsigned int i = 0; i < sim.nParticles; ++i)
        {
            Particle& pi = sim.particles[i];

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3{pi.r.x / SCALE_FACTOR, pi.r.y / SCALE_FACTOR, pi.r.z / SCALE_FACTOR});
            shader.setMat4("model", model);

            glBindVertexArray(sim.VAO);
                glDrawArrays(GL_POINTS, 0, sim.nParticles);
            glBindVertexArray(0);
        }

        window.swapBuffers();
        // ---------------------------------------------------------------------------------------------------------------------------------------------------------------

        timer.tick();
    }

    glDeleteVertexArrays(1, &sim.VAO);
    glDeleteBuffers(1, &sim.VBO);

    delete[] sim.vertexData;
    delete[] sim.particles;
    return 0;
}
