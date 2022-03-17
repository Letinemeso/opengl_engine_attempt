#include "../include/Object.h"

using namespace LEti;

Object::Object()
{
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	translation_matrix =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	rotation_matrix = glm::rotate(0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	rotation_axis[0] = 0.0f;
	rotation_axis[1] = 1.0f;
	rotation_axis[2] = 0.0f;
	
	scale_matrix =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
}

Object::~Object()
{
	glDeleteBuffers(2, buffer);
	glDeleteVertexArrays(1, &vertex_array);
}



void Object::init_texture(const char* _tex_path, float* tex_coords, unsigned int _tex_coords_count)
{
	glBindVertexArray(vertex_array);
	texture.init(_tex_path, tex_coords, _tex_coords_count);
	texture.setup_tex_coords_buffer(&buffer[1], 1);
}

void Object::init_vertices(float* _coords, unsigned int _coords_count)
{
	glBindVertexArray(vertex_array);
	vertices.load(_coords, _coords_count);
	vertices.setup_vertex_buffer(&buffer[0], 0);

	//glGenBuffers(1)
}



void Object::draw() const
{
	glm::mat4x4 result_matrix = translation_matrix * rotation_matrix * scale_matrix;
	LEti::Shader::set_matrix(result_matrix);

	glBindVertexArray(vertex_array);
	
	LEti::Shader::set_texture(texture);
	glDrawArrays(GL_TRIANGLES, 0, vertices.get_vertices_count());
}

void Object::update(float _dt)
{

}



void Object::set_pos(float _x, float _y, float _z)
{
	translation_matrix[3][0] = _x;
	translation_matrix[3][1] = _y;
	translation_matrix[3][2] = _z;
}

void Object::move(float _x, float _y, float _z)
{
	translation_matrix[3][0] += _x;
	translation_matrix[3][1] += _y;
	translation_matrix[3][2] += _z;
}


void Object::set_rotation_axis(float _x, float _y, float _z)
{
	rotation_axis[0] = _x;
	rotation_axis[1] = _y;
	rotation_axis[2] = _z;

	rotation_matrix = glm::rotate(rotation_angle, rotation_axis);
}

void Object::set_rotation_angle(float _angle)
{
	rotation_angle = _angle;

	while (rotation_angle >= 6.28318f)
		rotation_angle -= 6.28318f;
	while (rotation_angle <= -6.28318f)
		rotation_angle += -6.28318f;

	rotation_matrix = glm::rotate(rotation_angle, rotation_axis);
}

void Object::set_rotation_data(float _axis_x, float _axis_y, float _axis_z, float _angle)
{
	rotation_axis[0] = _axis_x;
	rotation_axis[1] = _axis_y;
	rotation_axis[2] = _axis_z;
	rotation_angle = _angle;

	rotation_matrix = glm::rotate(rotation_angle, rotation_axis);
}

void Object::rotate(float _angle)
{
	rotation_angle += _angle;

	if (rotation_angle >= 6.28318f)
		rotation_angle -= 6.28318f;
	if (rotation_angle <= -6.28318f)
		rotation_angle += -6.28318f;

	rotation_matrix = glm::rotate(rotation_angle, rotation_axis);
}


void Object::set_scale(float _scale_x, float _scale_y, float _scale_z)
{
	scale_matrix[0][0] = _scale_x;
	scale_matrix[1][1] = _scale_y;
	scale_matrix[2][2] = _scale_z;
}

void Object::set_overall_scale(float _scale)
{
	Object::set_scale(_scale, _scale, _scale);
}