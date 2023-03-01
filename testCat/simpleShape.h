#pragma once

namespace cat {

class Object;
class IRender;
class Env;

Object* _createGrid(IRender* render, Env* env);

Object* _createCube(IRender* render, Env* env);



} // namespace cat


