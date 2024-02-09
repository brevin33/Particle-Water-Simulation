#include <particle.h>
#include <algorithm>

ParticleSystem* setupParticles(int numParticles, float area, glm::vec3 center){
	ParticleSystem* particles = new ParticleSystem;

	glm::vec3 pos;
	int bucketX;
	int bucketY;
	int bucketZ;
	particles->center = center;
	particles->area = area;
	particles->smoothingRadius = area / NUMBUCKETSPERAXIS;
	for (int i = 0; i < numParticles; i++) {
		pos.x = (fmod(rand(), area) - area / 2) + center.x;
		pos.y = (fmod(rand(), area) - area / 2) + center.y;
		//pos.z = (rand() * zArea - zArea / 2) + center.z;
		pos.z = 0;
		bucketX = ((pos.x - center.x + area / 2) / area) * NUMBUCKETSPERAXIS;
		bucketY = ((pos.y - center.y + area / 2) / area) * NUMBUCKETSPERAXIS;
		//bucketZ = ((pos.z - center.z + zArea / 2) / zArea) * NUMBUCKETSPERAXIS;
		particles->data.push_back({pos, glm::vec3(0), glm::vec3(0)});
		particles->buckets[bucketX][bucketY].push_back(particles->data.size() - 1);
	}
	return particles;
}

float smothingFunction(float dist) {
	return 1 / (dist + 1);
}

void updateParticles(ParticleSystem* particles, float dt){
	for (uint16_t j = 0; j < particles->data.size(); j++) {
		PhysicsData& pd = particles->data[j];
		int bucketX = ((pd.pos.x - particles->center.x + particles->area / 2) / particles->area) * NUMBUCKETSPERAXIS;
		int bucketY = ((pd.pos.y - particles->center.y + particles->area / 2) / particles->area) * NUMBUCKETSPERAXIS;
		int bucketI;
		glm::vec3 force = glm::vec3(0);
		for (int x = -1; x <= 1; x++) {
			if (bucketX + x < 0 || bucketX + x >= NUMBUCKETSPERAXIS) continue;
			for (int y = -1; y <= 1; y++) {
				if (bucketY + y < 0 || bucketY + y >= NUMBUCKETSPERAXIS) continue;
				for (int i = 0; i < particles->buckets[bucketX + x][bucketY + y].size(); i++) {
					PhysicsData& other = particles->data[particles->buckets[bucketX + x][bucketY + y][i]];
					if (&other == &pd) { 
						bucketI = i; continue;
					}
					float dist = std::max(glm::distance(other.pos, pd.pos),0.01f);
					if (dist > particles->smoothingRadius) continue;
					force += smothingFunction(dist) * (pd.pos - other.pos);
				}
			}
		}
		pd.acc = force;
		pd.vel += pd.acc * dt;
		pd.pos += pd.vel * dt;
		pd.pos.x = std::min((particles->center.x + particles->area / 2) - 0.001f, pd.pos.x);
		pd.pos.y = std::min((particles->center.y + particles->area / 2) - 0.001f, pd.pos.y);
		pd.pos.x = std::max((particles->center.x - particles->area / 2) + 0.001f, pd.pos.x);
		pd.pos.y = std::max((particles->center.y - particles->area / 2) + 0.001f, pd.pos.y);
		std::swap(particles->buckets[bucketX][bucketY][bucketI], particles->buckets[bucketX][bucketY].back());
		particles->buckets[bucketX][bucketY].resize(particles->buckets[bucketX][bucketY].size() - 1);
		int bucketX2 = ((pd.pos.x - particles->center.x + particles->area / 2) / particles->area) * NUMBUCKETSPERAXIS;
		int bucketY2 = ((pd.pos.y - particles->center.y + particles->area / 2) / particles->area) * NUMBUCKETSPERAXIS;
		particles->buckets[bucketX2][bucketY2].push_back(j);
	}
}


struct Triangle
{
	uint32_t vertex[3];
};

struct v3
{
	float pos[3];
};

v3 normalize(v3 v) {
	float magnitude = glm::sqrt(v.pos[0] * v.pos[0] + v.pos[1] * v.pos[1] + v.pos[2] * v.pos[2]);
	v.pos[0] = v.pos[0] / magnitude;
	v.pos[1] = v.pos[1] / magnitude;
	v.pos[2] = v.pos[2] / magnitude;
	return v;
}

v3 add(v3 first, v3 second) {
	first.pos[0] += second.pos[0];
	first.pos[1] += second.pos[1];
	first.pos[2] += second.pos[2];
	return first;
}

using TriangleList = std::vector<Triangle>;
using VertexList = std::vector<v3>;
using Lookup = std::map<std::pair<uint32_t, uint32_t>, uint32_t>;

namespace icosahedron
{
	const float X = .525731112119133606f;
	const float Z = .850650808352039932f;
	const float N = 0.f;

	static const VertexList vertices =
	{
	  {-X,N,Z}, {X,N,Z}, {-X,N,-Z}, {X,N,-Z},
	  {N,Z,X}, {N,Z,-X}, {N,-Z,X}, {N,-Z,-X},
	  {Z,X,N}, {-Z,X, N}, {Z,-X,N}, {-Z,-X, N}
	};

	static const TriangleList triangles =
	{
	  {0,4,1},{0,9,4},{9,5,4},{4,5,8},{4,8,1},
	  {8,10,1},{8,3,10},{5,3,8},{5,2,3},{2,7,3},
	  {7,10,3},{7,6,10},{7,11,6},{11,0,6},{0,1,6},
	  {6,1,10},{9,0,11},{9,11,2},{9,2,5},{7,2,11}
	};
}

uint32_t vertex_for_edge(Lookup& lookup,
	VertexList& vertices, uint32_t first, uint32_t second)
{
	Lookup::key_type key(first, second);
	if (key.first > key.second)
		std::swap(key.first, key.second);

	auto inserted = lookup.insert({ key, vertices.size() });
	if (inserted.second)
	{
		auto& edge0 = vertices[first];
		auto& edge1 = vertices[second];
		auto point = normalize(add(edge0, edge1));
		vertices.push_back(point);
	}

	return inserted.first->second;
}

TriangleList subdivide(VertexList& vertices,
	TriangleList triangles)
{
	Lookup lookup;
	TriangleList result;

	for (auto&& each : triangles)
	{
		std::array<uint32_t, 3> mid;
		for (int edge = 0; edge < 3; ++edge)
		{
			mid[edge] = vertex_for_edge(lookup, vertices,
				each.vertex[edge], each.vertex[(edge + 1) % 3]);
		}

		result.push_back({ each.vertex[0], mid[0], mid[2] });
		result.push_back({ each.vertex[1], mid[1], mid[0] });
		result.push_back({ each.vertex[2], mid[2], mid[1] });
		result.push_back({ mid[0], mid[1], mid[2] });
	}

	return result;
}

using IndexedMesh = std::pair<VertexList, TriangleList>;

IndexedMesh make_icosphere(int subdivisions)
{
	VertexList vertices = icosahedron::vertices;
	TriangleList triangles = icosahedron::triangles;

	for (int i = 0; i < subdivisions; ++i)
	{
		triangles = subdivide(vertices, triangles);
	}

	return{ vertices, triangles };
}

std::pair<MZ::VertexBufferID, MZ::IndexBufferID> makeCircle(int subdivisions)
{
	std::vector<v3> vertices;
	std::vector<uint32_t> indices;
	IndexedMesh circlePosAndIndies = make_icosphere(subdivisions);
	for (size_t i = 0; i < circlePosAndIndies.first.size(); i++)
	{
		v3 v = circlePosAndIndies.first[i];
		vertices.push_back(v);
	}

	for (size_t i = 0; i < circlePosAndIndies.second.size(); i++)
	{
		uint32_t i1 = circlePosAndIndies.second[i].vertex[0];
		uint32_t i2 = circlePosAndIndies.second[i].vertex[1];
		uint32_t i3 = circlePosAndIndies.second[i].vertex[2];
		indices.push_back(i1);
		indices.push_back(i2);
		indices.push_back(i3);
	}
	return { MZ::createConstVertexBuffer(vertices.data(), vertices.size(), vertices.size() * sizeof(v3)), MZ::createConstIndexBuffer(indices.data(), indices.size() * sizeof(uint32_t)) };
}
