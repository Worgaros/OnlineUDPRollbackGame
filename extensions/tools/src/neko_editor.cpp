//
// Created by efarhan on 7/31/19.
//
#include <tools/neko_editor.h>
#include <utilities/file_utility.h>
#include <engine/log.h>

namespace editor
{
void NekoEditor::Init()
{
    BasicEngine::Init();
    sceneRenderTexture_.create(config.gameWindowSize.x, config.gameWindowSize.y);
    neko::IterateDirectory(config.dataRootPath, [this](std::string_view path)
    {
        if (!neko::IsRegularFile(path) || !textureManager_.HasValidExtension(path))
            return;
        textureManager_.LoadTexture(path);
    }, true);
}

void NekoEditor::Update(float dt)
{
    BasicEngine::Update(dt);
    const ImVec2 windowSize = ImVec2(config.realWindowSize.x, config.realWindowSize.y);
    const static float yOffset = 20.0f;
    ImGui::SetNextWindowPos(ImVec2(0.0f, windowSize.y * 0.7f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x * 0.8f, windowSize.y * 0.3f), ImGuiCond_Always);

    ImGui::Begin("Debug Window", nullptr,
                 ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoCollapse);
    static ImGui::FileBrowser fileDialog;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New", "CTRL+N"))
            {
                for (neko::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
                {
                    entityManager_.DestroyEntity(entity);
                    entityViewer_.Reset();
                }
            }
            if (ImGui::MenuItem("Open", "CTRL+O"))
            {
                fileDialog = ImGui::FileBrowser();
                fileDialog.SetPwd("../"+config.dataRootPath);
                fileDialog.Open();
                fileOperationStatus_ = FileOperation::OPEN;
            }
            if (ImGui::MenuItem("Save", "CTRL+S"))
            {
                auto& path = sceneManager_.GetCurrentScene().scenePath;
                if (path.empty() || !neko::FileExists(path))
                {
                    fileDialog = ImGui::FileBrowser(
                            ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir);
                    fileDialog.SetPwd("../"+config.dataRootPath);
                    fileDialog.Open();
                    fileOperationStatus_ = FileOperation::SAVE;
                }
                else
                {
                    auto sceneJson = sceneManager_.SerializeScene();
                    auto sceneTxt = sceneJson.dump(4);
                    neko::WriteStringToFile(path, sceneTxt);
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    fileDialog.Display();


    if (fileDialog.HasSelected())
    {
        switch (fileOperationStatus_)
        {
            case FileOperation::OPEN:
            {
                auto sceneJsonPath = fileDialog.GetSelected().string();
                if (!neko::FileExists(sceneJsonPath))
                {
                    logDebug("[Error] Could not open " + sceneJsonPath);
                    break;
                }
                auto sceneJson = neko::LoadJson(sceneJsonPath);
                sceneManager_.ParseSceneJson(*sceneJson);
                break;
            }
            case FileOperation::SAVE:
            {
                auto sceneJsonPath = fileDialog.GetSelected().string();
                auto sceneJson = sceneManager_.SerializeScene();
                auto sceneTxt = sceneJson.dump(4);
                neko::WriteStringToFile(sceneJsonPath, sceneTxt);
                sceneManager_.GetCurrentScene().scenePath = sceneJsonPath;
                sceneManager_.GetCurrentScene().sceneName = neko::GetFilename(sceneJsonPath);
                break;
            }
            default:
                break;
        }
        fileDialog.ClearSelected();
        fileDialog.Close();
    }
    ImGui::End();
    sceneRenderTexture_.clear(config.bgColor);

    //Draw things into the graphics manager
    spriteManager_.CopyAllTransformPositions(entityManager_, positionManager_);
    spriteManager_.CopyAllTransformScales(entityManager_, scaleManager_);
    spriteManager_.CopyAllTransformAngles(entityManager_, angleManager_);

    spriteManager_.PushAllCommands(entityManager_, graphicsManager_);

    graphicsManager_.Render(&sceneRenderTexture_);
    sceneRenderTexture_.display();

    ImGui::SetNextWindowPos(ImVec2(0.0f, yOffset), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x * 0.2f, windowSize.y * 0.7f - yOffset), ImGuiCond_Always);
    entityViewer_.Update(entityManager_, sceneManager_);

    ImGui::SetNextWindowPos(ImVec2(windowSize.x * 0.2f, yOffset), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x * 0.6f, windowSize.y * 0.7f - yOffset), ImGuiCond_Always);
    ImGui::Begin("Central Viewer", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    if (ImGui::BeginTabBar("Central Tab"))
    {
        sceneViewer_.Update(sceneRenderTexture_);
        //TODO add animator and other things in the central viewer
        ImGui::EndTabBar();
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(windowSize.x * 0.8f, yOffset), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x * 0.2f, windowSize.y * 0.7f - yOffset), ImGuiCond_Always);
    inspector_.BeginWindow();
    inspector_.ShowEntityInfo(entityViewer_.GetSelectedEntity());
    inspector_.EndWindow();
}

void NekoEditor::Destroy()
{
    BasicEngine::Destroy();
}

neko::BodyDef2dManager& NekoEditor::GetBodyDefManager()
{
    return bodyDefManager_;
}

neko::EntityManager& NekoEditor::GetEntityManager()
{
    return entityManager_;
}

neko::Position2dManager& NekoEditor::GetPositionManager()
{
    return positionManager_;
}

neko::Scale2dManager& NekoEditor::GetScaleManager()
{
    return scaleManager_;
}

neko::Angle2dManager& NekoEditor::GetAngleManager()
{
    return angleManager_;
}

neko::SceneManager& NekoEditor::GetSceneManager()
{
    return sceneManager_;
}

neko::BasicSpriteManager& NekoEditor::GetSpriteManager()
{
    return spriteManager_;
}

neko::TextureManager& NekoEditor::GetTextureManager()
{
    return textureManager_;
}

}