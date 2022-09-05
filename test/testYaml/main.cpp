
#include "cat/yaml.h"

#include "rapidyaml/ryml.hpp"
#include "c4/format.hpp" // needed for the examples below

#include "scl/file_mapping.h"
#include "scl/vector.h"

#include <iostream>

#ifdef _WIN32
#include <crtdbg.h>
#include <Windows.h> //#include "vld_runtime/include/vld.h" //#include <C:\\Program Files (x86)\\Visual Leak Detector\\include\\vld.h>
#endif

const char* const g_filename_raw = "./1.yaml";
const char* const g_filename = "./2.yaml";

typedef ryml::NodeRef node_raw;
typedef ryml::Tree tree;

node_raw add_scene_raw(node_raw& scenes, const char* const name)
{
	node_raw scene = scenes.append_child();
	scene |= ryml::MAP;
	scene["name"] << name;
	node_raw objects = scene["objects"];
	objects |= ryml::SEQ;
	return scene;
}

node_raw add_object_raw(node_raw objects, const char* const name, float x, float y, float z)
{
	node_raw obj = objects.append_child();
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


void test_write_raw()
{
	tree t;

	node_raw root = t.rootref();
	root |= ryml::MAP;

	node_raw scenes = root["scenes"] ;
	scenes |= ryml::SEQ;

	node_raw scene1 = add_scene_raw(scenes, "scene_1");

	add_object_raw(scene1["objects"], "obj1", 1, 3.22f, -0.003f);
	add_object_raw(scene1["objects"], "obj2", 1, 3.22f, -0.003f);
	
	node_raw scene2 = add_scene_raw(scenes, "scene_2");
	add_object_raw(scene2["objects"], "obj3", -01, -3.22f, 0.2003f);


	FILE* fp = fopen(g_filename_raw, "wb");

	size_t len = ryml::emit(t, t.root_id(), fp);

	fclose(fp);
	std::cout << t;
}

void test_read_raw()
{
	scl::file_mapping fm;
	fm.open(g_filename_raw);
	char* buffer = (char*)fm.map();

	//FILE* fp = fopen("d:/1.yaml", "rb");	
	//int bufferSize = fread(NULL, 1, 0, fp) + 1;
	//char* buffer = new char[bufferSize];
	//memset(buffer, 0, bufferSize);
	//fread(buffer, 1, bufferSize, fp);

    ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(buffer));
	//delete[] buffer;
	std::cout << tree;

	node_raw scenes = tree["scenes"];
	printf("scene count = %d\n", scenes.num_children());
	for (node_raw scene : scenes.children())
	{
		std::string name;
		node_raw nameNode = scene["name"];
		name.assign(nameNode.val().data(), nameNode.val().len);
		printf("  scene name = %s\n", name.c_str());

		const char* s = nameNode.val().data();
		node_raw objects = scene["objects"];
		for (node_raw obj : objects.children())
		{
			c4::csubstr obj_nameNode = obj["name"].val();
			std::string obj_name(obj_nameNode.data(), obj_nameNode.len);
			printf("    obj name = %s\n", obj_name.c_str());
		}
		//std::cout << nameNode;
	}

	fm.close();
}

void test_read()
{
	yaml::document doc;
	yaml::node root = doc.load(g_filename);
	yaml::node scenes = root["scenes"];
	for (yaml::node scene : scenes.children())
	{
		yaml::node sceneName	= scene["name"];
		yaml::node objects		= scene["objects"];
		printf("scene = %s\n", sceneName.value());
		for (yaml::node object: objects.children())
		{
			yaml::node		objectName	= object["name"];
			scl::vector3	pos			= object["pos"].to_vector3();
			printf("  name = %s, x, y, z = %f, %f, %f, pos = %f, %f, %f, enable = %d, visibule = %d\n", 
				objectName.value(), 
				object["x"].to_float(), 
				object["y"].to_float(), 
				object["z"].to_float(), 
				pos.x, 
				pos.y, 
				pos.z, 
				object["enable"].to_bool(), 
				object["visible"].to_bool());
		}
	}
}

void test_write()
{
	yaml::document doc;
	yaml::node scenes = doc.root().set_map().add_seq("scenes");
	yaml::node scene1 = scenes.add_map();
	scene1.add("name", "scene_1");
	yaml::node objects1 = scene1.add_seq("objects");

	yaml::node obj1 = objects1.add_map();
	obj1.add("name", "obj1");
	obj1.add("x", 1);
	obj1.add("y", 3.22f);
	obj1.add("z", -0.003f);
	obj1.add("pos", scl::vector3 { 1.2, -2.33, 0.23333 });

	yaml::node obj2 = objects1.add_map();
	obj2.add("name", "obj2");
	obj2.add("x", 1);
	obj2.add("y", 3.22f);
	obj2.add("enable", true);
	obj2.add("visible", false);
	obj2.add("pos", scl::vector3 { 1.2, -2.33, 0.23333 });
	
	/////////////

	yaml::node scene2 = scenes.add_map();
	scene2.add("name", "scene_2");
	yaml::node objects3 = scene2.add_seq("objects");

	yaml::node obj3 = objects3.add_map();
	obj3.add("name", "obj3");
	obj3.add("x", -01);
	obj3.add("y", -3.22f);
	obj3.add("z", 0.2003f);
	obj3.add("pos", scl::vector3 { 1.2, -2.33, 0.23333 });

	doc.save(g_filename);
}

int main()
{
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//如果发生泄漏，请将泄漏的内存序号填写在下面
	//_CrtSetBreakAlloc(1535);
#endif
	printf("\n\n****************** test raw ****************** \n\n");

	test_write_raw();

	test_read_raw();

	printf("\n\n****************** test cat::yaml ******************\n\n");

	test_write();

	test_read();

	getchar();

	return 0;
}


