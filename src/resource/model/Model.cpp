#include "Model.h"
#include "Tokenizer.h"
#include "Token.h"
#include "../../utils/file.h"
#include "../../Logger.h"

Model::Model(std::string name)
{
	this->name = name;
}


Model::~Model()
{
}

#define error(x) { logger.Error("%s: Invalid token at line %d position %d, %s expected", filename.c_str(), tokenizer.getLine(), tokenizer.getPosition(), (x)); return false;}

bool Model::load()
{
	_stat((name+".obj").c_str(), &info.info);
	info.name = name + ".obj";

	std::string filename = name + ".obj";

	auto buf = getFileContentsAsString(filename);
	if (buf.empty()) return false;

	bool texcoord = false;
	bool normal = false;

	std::vector<modelVertice> data;
	std::vector<Segment> segments;
	std::string objectName = "default";
	std::string materialName = "";
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> coords;
	Tokenizer tokenizer(buf);

	while (tokenizer.tokenAvailable())
	{
		Token t = tokenizer.getToken();
		if (t.type == Token::Type::Empty) continue;
		if (t.type != Token::Type::String) error("string");

		if (t.s == "v")
		{
			glm::vec3 v;
			
			for (int i = 0; i < 3; i++)
			{
				t = tokenizer.getToken();
				if (t.type != Token::Type::Float) error("float");
				v[i] = t.f;
			}
			vertices.push_back(v);
			tokenizer.nextLine();
		}
		else if (t.s == "vn")
		{
			glm::vec3 vn;

			for (int i = 0; i < 3; i++)
			{
				t = tokenizer.getToken();
				if (t.type != Token::Type::Float) error("float");
				vn[i] = t.f;
			}
			normals.push_back(vn);
			tokenizer.nextLine();
		}
		else if (t.s == "vt")
		{
			glm::vec2 vt;

			for (int i = 0; i < 2; i++)
			{
				t = tokenizer.getToken();
				if (t.type != Token::Type::Float) error("float");
				vt[i] = t.f;
			}
			vt[1] = -vt[1];
			coords.push_back(vt);
			tokenizer.nextLine();
		}
		else if (t.s == "f")
		{
			int v[3], vt[3], vn[3];
			bool load = true;
			for (int i = 0; i < 3; i++)
			{
				if (load) t = tokenizer.getToken();
				load = true;
				if (t.type == Token::Type::Int) v[i] = t.i;
				else error("int");

				t = tokenizer.getToken();
				if (t.type == Token::Type::Int) {
					load = false; 
					continue;
				} // Face only
				else if (t.type != Token::Type::Separator) error("separator");

				t = tokenizer.getToken();
				if (t.type == Token::Type::Int) {
					vt[i] = t.i;
					texcoord = true;
				}
				else if (t.type != Token::Type::Separator) error("separator");// No texcoords

				t = tokenizer.getToken();
				if (t.type == Token::Type::Separator) t = tokenizer.getToken();
				if (t.type == Token::Type::Int) {
					vn[i] = t.i;
					normal = true;
				}
			}

			t = tokenizer.getToken();
			if (t.type != Token::Type::Empty) 
			{
				logger.Error("%s: Mesh not triangulated, please save model with \"Triangulate\" options", filename.c_str());
				return false;
			}

			for (int i = 0; i < 3; i++)
			{
				modelVertice mv;
				if (v[i] - 1 >= vertices.size()) {
					logger.Error("%s: Vertices out of bound", filename.c_str(), tokenizer.getLine());
					return false;
				}
				if (texcoord && vt[i] - 1 >= coords.size()) {
					logger.Error("%s: Texcoords out of bound", filename.c_str(), tokenizer.getLine());
					return false;
				}
				if (normal && vn[i] - 1 >= normals.size()) {
					logger.Error("%s: Normals out of bound at line %d", filename.c_str(), tokenizer.getLine());
					return false;
				}
				mv.position = vertices[v[i] - 1];
				if (normal) mv.normal = normals[vn[i] - 1];
				if (texcoord) mv.texcoord = coords[vt[i] - 1];
				data.push_back(mv);
			}
		}
		else if (t.s == "mtllib")
		{
			t = tokenizer.getToken();
			materialFile = t.s;

			materials = parseMaterialFile( getPath(filename) + materialFile);
			// Parse material file
		}
		else if (t.s == "usemtl")
		{
			t = tokenizer.getToken();
			materialName = t.s;
		}
		else if (t.s == "o")
		{
			if (!data.empty() || !segments.empty()) commit(objectName, materialName, data, segments);// commit current changes
			t = tokenizer.getToken();
			objectName = t.s;
		}
		else if (t.s == "l")
		{
			Segment segment;
			t = tokenizer.getToken();
			segment.start = vertices[t.i-1];

			t = tokenizer.getToken();
			segment.end = vertices[t.i - 1];

			segments.push_back(segment);
		}
		else tokenizer.nextLine();
		
	}
	if (!data.empty()) commit(objectName, materialName, data, segments);
	vertices.clear();
	normals.clear();
	coords.clear();

	initialized = true;
	return true;
}

void Model::commit(std::string objectName, std::string materialName, std::vector<modelVertice> &data, std::vector<Segment> &segments)
{
	std::shared_ptr<ModelObject> object(new ModelObject());

	object->data = data;
	object->segments = segments;
	if (materials.find(materialName) != materials.end()) object->material = materials.find(materialName)->second;
	object->size = data.size();

	if (!data.empty()) {
		gl::GenVertexArrays(1, &object->vao);
		gl::BindVertexArray(object->vao);

		gl::GenBuffers(1, &object->vbo);
		gl::BindBuffer(gl::ARRAY_BUFFER, object->vbo);
		gl::BufferData(gl::ARRAY_BUFFER, object->data.size()*sizeof(modelVertice), &object->data[0], gl::STATIC_DRAW);

		gl::VertexAttribPointer(0, 3, gl::FLOAT, false, sizeof(modelVertice), 0);
		gl::VertexAttribPointer(1, 3, gl::FLOAT, false, sizeof(modelVertice), (void*)(3 * sizeof(GLfloat)));
		gl::VertexAttribPointer(2, 2, gl::FLOAT, false, sizeof(modelVertice), (void*)(6 * sizeof(GLfloat)));

		gl::EnableVertexAttribArray(0);
		gl::EnableVertexAttribArray(1);
		gl::EnableVertexAttribArray(2);

		gl::BindVertexArray(0);
	}

	objects.emplace(objectName, object);

	data.clear();
	segments.clear();
	// TODO: No buffers deallocation!!!
}

#undef error
#define error(x) { logger.Error("%s: Invalid token at line %d position %d, %s expected", filename.c_str(), tokenizer.getLine(), tokenizer.getPosition(), (x)); return materials;}
std::unordered_map<std::string, Material> Model::parseMaterialFile(std::string filename)
{
	std::unordered_map<std::string, Material> materials;
	Material material;

	auto buf = getFileContentsAsString(filename);
	if (buf.empty()) return materials;

	std::string materialName;
	Tokenizer tokenizer(buf);
	material.texture = filename;

	while (tokenizer.tokenAvailable())
	{
		Token t = tokenizer.getToken();
		if (t.type == Token::Type::Empty) continue;
		if (t.type != Token::Type::String) error("string");

		if (t.s == "newmtl")
		{
			if (!materialName.empty()) materials.emplace(materialName, material);// Commit

			t = tokenizer.getToken();
			materialName = t.s;

			material = Material();
		}
		else if (t.s == "Ka")
		{
			for (int i = 0; i < 3; i++)
			{
				t = tokenizer.getToken();
				if (t.type != Token::Type::Float) error("float");
				if (t.f < 0.3) t.f = 0.3;
				material.ambient[i] = t.f;
			}
			tokenizer.nextLine();
		}
		else if (t.s == "Kd")
		{
			for (int i = 0; i < 3; i++)
			{
				t = tokenizer.getToken();
				if (t.type != Token::Type::Float) error("float");
				material.diffuse[i] = t.f;
			}
			tokenizer.nextLine();
		}
		else if (t.s == "Ks")
		{
			for (int i = 0; i < 3; i++)
			{
				t = tokenizer.getToken();
				if (t.type != Token::Type::Float) error("float");
				material.specular[i] = t.f;
			}
			tokenizer.nextLine();
		}
		// Ni, d, illum
		else if (t.s == "map_Kd")
		{
			t = tokenizer.getToken();
			material.texture = getFilename(t.s);
		}
		else tokenizer.nextLine();

	}
	if (!materialName.empty()) materials.emplace(materialName, material);// Commit

	return materials;
}

bool Model::render()
{
	if (!initialized) return false;

	for (auto obj : objects) {
		obj.second->render();
	}
	return true;
}