#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Mesmerize/Renderer.h>
#define NUMBUCKETSPERAXIS 10

struct PhysicsData {
	glm::vec3 pos;
	glm::vec3 vel;
	glm::vec3 acc;
};

struct ParticleSystem {
	glm::vec3 center;
	float area;
	float smoothingRadius;
	std::array<std::array<std::vector<uint16_t>, NUMBUCKETSPERAXIS>, NUMBUCKETSPERAXIS> buckets;
	std::vector<PhysicsData> data;
};

ParticleSystem* setupParticles(int numParticles, float area, glm::vec3 center);
void updateParticles(ParticleSystem* particles, float dt);
std::pair<MZ::VertexBufferID, MZ::IndexBufferID> makeCircle(int subdivisions);


