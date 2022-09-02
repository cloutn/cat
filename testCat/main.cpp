
#include "./client.h"



#include "scl/vector.h"
#include "rapidyaml/ryml.hpp"
#include "c4/format.hpp" // needed for the examples below

//namespace scl {
//
//template<class T> size_t to_chars(ryml::substr buf, scl::vector3 v) { return ryml::format(buf, "({},{},{})", v.x, v.y, v.z); }
//template<class T> bool from_chars(ryml::csubstr buf, scl::vector3* v) { size_t ret = ryml::unformat(buf, "({},{},{})", v->x, v->y, v->z); return ret != ryml::yml::npos; }
//
//}

//template<class T> bool from_chars(ryml::csubstr buf, vec3* v) { size_t ret = ryml::unformat(buf, "({},{},{})", v->x, v->y, v->z); return ret != ryml::yml::npos; }



namespace scl {

size_t to_chars(ryml::substr buf, scl::vector3 v) { return ryml::format(buf, "{x : {}, y : {}, z : {}}", v.x, v.y, v.z); }
bool from_chars(ryml::csubstr buf, scl::vector3* v) { size_t ret = ryml::unformat(buf, "{x : {}, y : {}, z : {}}", v->x, v->y, v->z); return ret != ryml::yml::npos; }
}


#include "scl/log.h"
#include "scl/file_mapping.h"

#include <iostream>


// ####################################
#ifdef _WIN32
#include <crtdbg.h>
#include <Windows.h>
//#include "vld_runtime/include/vld.h"
//#include <C:\\Program Files (x86)\\Visual Leak Detector\\include\\vld.h>
#endif
// ####################################

typedef ryml::NodeRef node;
typedef ryml::Tree tree;

void test()
{
	cat::Client* c = new cat::Client();
	c->init(1280, 768);
	c->run();
	delete c;
}


node add_scene(node& scenes, const char* const name)
{
	node scene = scenes.append_child();
	scene |= ryml::MAP;
	scene["name"] << name;
	node objects = scene["objects"];
	objects |= ryml::SEQ;
	return scene;
}

node add_object(node objects, const char* const name, float x, float y, float z)
{
	node obj = objects.append_child();
	obj |= ryml::MAP;
	obj["name"] << name;
	obj["x"] << x;
	obj["y"] << y;
	obj["z"] << z;
	//scl::vector3 pos { 1.2, -2.33, 0.23333 };
	scl::vector3 pos { 1.2, -2.33, 0.23333 };
	obj["pos"] << pos;
	return obj;
}


void test3()
{
	tree t;

	node root = t.rootref();
	root |= ryml::MAP;

	node scenes = root["scenes"] ;
	scenes |= ryml::SEQ;

	node scene1 = add_scene(scenes, "scene_1");
	//node scene1 = scenes.append_child();
	//scene1 |= ryml::MAP;
	//scene1["name"] << "scene_1";


	add_object(scene1["objects"], "obj1", 1, 3.22f, -0.003f);
	add_object(scene1["objects"], "obj2", 1, 3.22f, -0.003f);
	
	node scene2 = add_scene(scenes, "scene_2");
	add_object(scene2["objects"], "obj3", -01, -3.22f, 0.2003f);

	//node pos = obj1["position"];
	//pos |= ryml::MAP;
	//pos["x"] << 1.33f;
	//pos["y"] << 222.f;
	//pos["z"] << 222.f;

	//node scene2 = root.append_child();

	FILE* fp = fopen("d:/1.yaml", "wb");

	size_t len = ryml::emit(t, t.root_id(), fp);

	fclose(fp);
	std::cout << t;
}

void test4()
{
	scl::file_mapping fm;
	fm.open("d:/1.yaml");
	char* buffer = (char*)fm.map();


	//FILE* fp = fopen("d:/1.yaml", "rb");	
	//int bufferSize = fread(NULL, 1, 0, fp) + 1;
	//char* buffer = new char[bufferSize];
	//memset(buffer, 0, bufferSize);
	//fread(buffer, 1, bufferSize, fp);

    ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(buffer));
	//delete[] buffer;
	std::cout << tree;

	node scenes = tree["scenes"];
	printf("scene count = %d\n", scenes.num_children());
	for (node scene : scenes.children())
	{
		std::string name;
		node nameNode = scene["name"];
		name.assign(nameNode.val().data(), nameNode.val().len);
		printf("  scene name = %s\n", name.c_str());

		const char* s = nameNode.val().data();
		node objects = scene["objects"];
		for (node obj : objects.children())
		{
			c4::csubstr obj_nameNode = obj["name"].val();
			std::string obj_name(obj_nameNode.data(), obj_nameNode.len);
			printf("    obj name = %s\n", obj_name.c_str());
		}

		//std::cout << nameNode;
	}

	fm.close();
	getchar();
	//delete[] buffer;

}

int main()
{
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//如果发生泄漏，请将泄漏的内存序号填写在下面
	//_CrtSetBreakAlloc(1535);
#endif

	test4();

	scl::log::release();

	return 0;
}

