#include "py_helper.hpp"

struct MyData
{
	int i;
	int j;
	std::string name;

	static inline char** kwlists() {
		static char* _kwlists[] = { "i", "j", "name",NULL };
		return _kwlists;
	}
};

std::ostream& operator<<(std::ostream& out, const MyData& data)
{
	out << data.i << " " << data.j << " " << data.name;
	return out;
}

template<>
PyObject* pytype_from(const MyData& value) {
	PyObject* dict = PyDict_New();
	auto kwlists = MyData::kwlists();
	int i = 0;
	py_dict_set_item(dict, kwlists[i++], value.i);
	py_dict_set_item(dict, kwlists[i++], value.j);
	py_dict_set_item(dict, kwlists[i++], value.name);
	return dict;
}

template<>
int pytype_to(PyObject* obj, MyData& value) {
	auto kwlists = MyData::kwlists();
	int i = 0;
	py_dict_get_item(obj, kwlists[i++], value.i);
	py_dict_get_item(obj, kwlists[i++], value.j);
	py_dict_get_item(obj, kwlists[i++], value.name);
	return 0;
}