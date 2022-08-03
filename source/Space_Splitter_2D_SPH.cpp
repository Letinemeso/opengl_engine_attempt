#include "../include/Space_Splitter_2D_SPH.h"


using namespace LEti;


std::list<const Object_2D*> Space_Splitter_2D_SPH::m_registred_models;
unsigned int Space_Splitter_2D_SPH::m_precision = 0;
unsigned int Space_Splitter_2D_SPH::m_array_size = 0;
std::list<const Object_2D*>** Space_Splitter_2D_SPH::m_array = nullptr;
unsigned int Space_Splitter_2D_SPH::m_number_binary_length = 0;
std::map<Space_Splitter_2D_SPH::Collision_Data, bool> Space_Splitter_2D_SPH::m_possible_collisions;
std::list<Space_Splitter_2D_SPH::Collision_Data> Space_Splitter_2D_SPH::m_collisions;
Space_Splitter_2D_SPH::Space_Border Space_Splitter_2D_SPH::m_space_borders;


void Space_Splitter_2D_SPH::Collision_Data::update_collision_data()
{
	collision_data = first->is_colliding_with_other(*second);
}



unsigned int Space_Splitter_2D_SPH::get_number_binary_length(unsigned int _number)
{
	unsigned int result = 0;
	for(unsigned int i=0; i<sizeof(_number) * 8; ++i)
		if(_number & (1u << i))
			result = i + 1;
	return result;
}



void Space_Splitter_2D_SPH::set_precision(unsigned int _precision)
{
	ASSERT(_precision == 0);

	if(m_array)
	{
		for(unsigned int i=0; i<m_array_size; ++i)
			delete m_array[i];
		delete[] m_array;
		m_array = nullptr;
	}

	m_number_binary_length = get_number_binary_length(_precision);
	m_precision = _precision;
	m_array_size = ((m_precision + 1) << m_number_binary_length) | m_precision + 1;
	m_array = new objects_list*[m_array_size];
	for(unsigned int i=0; i<m_array_size; ++i)
		m_array[i] = nullptr;
}


void Space_Splitter_2D_SPH::register_object(const Object_2D *_model)
{
	std::list<const Object_2D*>::iterator check = m_registred_models.begin();
	while(check != m_registred_models.end())
	{
		ASSERT(*check == _model);
		++check;
	}

	m_registred_models.push_back(_model);
}

void Space_Splitter_2D_SPH::unregister_object(const Object_2D *_model)
{
	std::list<const Object_2D*>::iterator it = m_registred_models.begin();
	while(it != m_registred_models.end())
	{
		if(*it == _model) break;
		++it;
	}
	ASSERT(it == m_registred_models.end());
	m_registred_models.erase(it);
}



void Space_Splitter_2D_SPH::update_border()
{
	if(m_registred_models.size() == 0) return;

	std::list<const Object_2D*>::iterator model_it = m_registred_models.begin();

	const Physical_Model_2D::Rectangular_Border& first_rb = (*model_it)->is_dynamic() ?
				(*model_it)->get_dynamic_rb() :
				(*model_it)->get_physical_model()->curr_rect_border();
	float max_left = first_rb.left;
	float max_right = first_rb.right;
	float max_top = first_rb.top;
	float max_bottom = first_rb.bottom;
	++model_it;

	while(model_it != m_registred_models.end())
	{
		if((*model_it)->get_collision_possibility() == false)
		{
			++model_it;
			continue;
		}

		const Physical_Model_2D::Rectangular_Border& rb = (*model_it)->is_dynamic() ?
					(*model_it)->get_dynamic_rb() :
					(*model_it)->get_physical_model()->curr_rect_border();

		if(rb.left < max_left) max_left = rb.left;
		if(rb.right > max_right) max_right = rb.right;
		if(rb.top > max_top) max_top = rb.top;
		if(rb.bottom < max_bottom) max_bottom = rb.bottom;

		++model_it;
	}

	m_space_borders.min_x = max_left;
	m_space_borders.min_y = max_bottom;
	m_space_borders.width = max_right - max_left;
	m_space_borders.height= max_top - max_bottom;
}

void Space_Splitter_2D_SPH::reset_hash_array()
{
	ASSERT(m_array == nullptr);

	for(unsigned int i=0; i<m_array_size; ++i)
	{
		if(m_array[i] != nullptr)
		{
			delete m_array[i];
			m_array[i] = nullptr;
		}
	}
}

void Space_Splitter_2D_SPH::hash_objects()
{
	reset_hash_array();

	objects_list::const_iterator model_it = m_registred_models.cbegin();
	while(model_it != m_registred_models.end())
	{
		const Physical_Model_2D::Rectangular_Border& curr_rb = (*model_it)->is_dynamic() ?
					(*model_it)->get_dynamic_rb() : (*model_it)->get_physical_model()->curr_rect_border();

//		unsigned int min_index_x = (unsigned int)(m_space_borders.width / (curr_rb.left - m_space_borders.min_x));
//		unsigned int max_index_x = (unsigned int)(m_space_borders.width / (curr_rb.right - m_space_borders.min_x));
//		unsigned int min_index_y = (unsigned int)(m_space_borders.height / (curr_rb.bottom - m_space_borders.min_y));
//		unsigned int max_index_y = (unsigned int)(m_space_borders.height / (curr_rb.top - m_space_borders.min_y));
		unsigned int min_index_x = (unsigned int)((curr_rb.left - m_space_borders.min_x) / m_space_borders.width * 10);
		unsigned int max_index_x = (unsigned int)((curr_rb.right - m_space_borders.min_x) / m_space_borders.width * 10);
		unsigned int min_index_y = (unsigned int)((curr_rb.bottom - m_space_borders.min_y) / m_space_borders.height * 10);
		unsigned int max_index_y = (unsigned int)((curr_rb.top - m_space_borders.min_y) / m_space_borders.height * 10);

		for(unsigned int x = min_index_x; x <= max_index_x; ++x)
		{
			for(unsigned int y = min_index_y; y <= max_index_y; ++y)
			{
				unsigned int hash = (x << m_number_binary_length) | (y);

				if(m_array[hash])
				{
					bool copy = false;
					for(const Object_2D*& a : *(m_array[hash]))
					{
						if(a == *model_it)
							copy = true;
					}
					if(!copy)
						m_array[hash]->push_back(*model_it);
				}
				else
				{
					m_array[hash] = new objects_list;
					m_array[hash]->push_back(*model_it);
				}
			}
		}

		++model_it;
	}
}



void Space_Splitter_2D_SPH::check_for_possible_collisions()
{
	m_possible_collisions.clear();

	for(unsigned int i=0; i<m_array_size; ++i)
	{
		if(m_array[i] == nullptr) continue;
		const objects_list& curr_list = *(m_array[i]);
		if(curr_list.size() < 2) continue;

		objects_list::const_iterator first = curr_list.begin();
		while(first != curr_list.end())
		{
			objects_list::const_iterator second = first;
			++second;

			while(second != curr_list.end())
			{
				Collision_Data cd(*first, *second);
				if(m_possible_collisions.find(cd) == m_possible_collisions.end())
					m_possible_collisions.emplace(cd, false);

				++second;
			}

			++first;
		}
	}
}

void Space_Splitter_2D_SPH::save_actual_collisions()
{
	m_collisions.clear();

	std::map<Collision_Data, bool>::iterator poss_col_it = m_possible_collisions.begin();

	while(poss_col_it != m_possible_collisions.end())
	{
		Collision_Data cd = poss_col_it->first;
		cd.update_collision_data();
		if(cd.collision_data)
			m_collisions.push_back(cd);
		++poss_col_it;
	}
}



void Space_Splitter_2D_SPH::update()
{
	update_border();
	hash_objects();
	check_for_possible_collisions();
	save_actual_collisions();
}



const std::list<Space_Splitter_2D_SPH::Collision_Data>& Space_Splitter_2D_SPH::get_collisions()
{
	return m_collisions;
}






































