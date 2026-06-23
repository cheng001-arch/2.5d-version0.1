#include "TitleScene.h"
#include "../../UI/TitleMenuUI.h"
#include "../../GameObject/LevelTransitionController.h"

void TitleScene::Event()
{
}

void TitleScene::Init()
{
	auto titleMenu = std::make_shared<TitleMenuUI>();
	titleMenu->Init();
	AddObject(titleMenu);

	auto transition = std::make_shared<LevelTransitionController>();
	AddObject(transition);
}
