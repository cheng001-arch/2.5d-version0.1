#include "TitleScene.h"
#include "../../UI/TitleMenuUI.h"

void TitleScene::Event()
{
}

void TitleScene::Init()
{
	auto titleMenu = std::make_shared<TitleMenuUI>();
	titleMenu->Init();
	AddObject(titleMenu);
}
