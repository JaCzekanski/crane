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

#define error(x) { logger.Error("%s: Invalid token, %s expected", filename.c_str(), (x)); return false;}

bool Model::load()
{
	_stat((name+".obj").c_str(), &info.info);
	info.name = name + ".obj";

	std::string filename = name + ".obj";

	auto buf = getFileContentsAsString(filename);
	if (buf.empty()) return false;

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> coords;
	std::vector<modelVertice> data;
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
			bool texcoord = false;
			bool normal = false;
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
		else tokenizer.nextLine();
		
	}
	vertices.clear();
	normals.clear();
	coords.clear();
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