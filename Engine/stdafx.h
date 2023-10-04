#pragma once

#include "Framework.h"
#pragma comment(lib, "Framework.lib")

#include "Systems/D3D.h"

// Assimp
#include "Assimp/Importer.hpp"
#include "Assimp/scene.h"
// 파일을 어떻게 불를 것인가 내용 결정
#include "Assimp/postprocess.h"
// library file이 기본으로 지정되어 있지 않을수 잇따.
// project 에 경로에 존재해야된다.
// 환경변수를 이용ㅎ에 너ㅗㅎ기
#pragma comment(lib, "Assimp/assimp-vc142-mtd.lib")