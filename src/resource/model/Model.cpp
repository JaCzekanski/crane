#include "Model.h"

Model::Model(std::string name)
{
	this->name = name;
}


Model::~Model()
{
}


bool Model::load()
{
	_stat((name+".obj").c_str(), &info.info);
	info.name = name + ".obj";

	char buf[1024];
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<modelVertice> data;
	FILE *obj = fopen((name+".obj").c_str(), "rb");
	if (!obj) return false;

	while (!feof(obj))
	{
		fgets(buf, 1024, obj);
		if (buf[0] == '#') continue;
		if (buf[0] == 'v' && buf[1] == 'n')
		{
			glm::vec3 v;
			sscanf(buf, "%*c%*c %f %f %f", &v.x, &v.y, &v.z);
			normals.push_back(v);
		}
		else if (buf[0] == 'v' && buf[1] == 't')
		{
			glm::vec2 v;
			sscanf(buf, "%*c%*c %f %f", &v.x, &v.y);
		}
		else if (buf[0] == 'v')
		{
			glm::vec3 v;
			sscanf(buf, "%*c %f %f %f", &v.x, &v.y, &v.z);
			vertices.push_back(v);
		}
		else if (buf[0] == 'f')
		{
			int v[3];
			int vn[3];
			sscanf(buf, "%*c %d%*c%*c%d %d%*c%*c%d %d%*c%*c%d",
				&v[0], &vn[0],
				&v[1], &vn[1],
				&v[2], &vn[2]);

			for (int i = 0; i < 3; i++)
			{
				modelVertice mv;
				mv.position = vertices[v[i] - 1];
				mv.normal = normals[vn[i] - 1];
				data.push_back(mv);
			}
		}

	}
	vertices.clear();
	normals.clear();
	gl::GenBuffers(1, &vbo);
	gl::BindBuffer(gl::ARRAY_BUFFER, vbo);
	gl::BufferData(gl::ARRAY_BUFFER, data.size()*sizeof(modelVertice), &data[0], gl::STATIC_DRAW);
	size = data.size();
	return true;
}

GLuint Model::get()
{
	return vbo;
}

bool Model::use()
{
	gl::BindBuffer(gl::ARRAY_BUFFER, vbo);
	return true;
}