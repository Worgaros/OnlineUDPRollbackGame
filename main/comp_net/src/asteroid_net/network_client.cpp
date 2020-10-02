#include "asteroid_net/network_client.h"
#include "engine/engine.h"
#include "imgui.h"
#include "engine/conversion.h"

namespace neko::net
{

void ClientNetworkManager::Init()
{
    const auto& config = BasicEngine::GetInstance()->config;
    windowSize_ = config.windowSize;
    gameManager_.SetWindowSize(windowSize_ / Vec2u(2, 1));


    clientId_ = RandomRange(std::numeric_limits<ClientId>::lowest(),
                            std::numeric_limits<ClientId>::max());
    //JOIN packet
    gameManager_.Init();
}

void ClientNetworkManager::Update(seconds dt)
{
    gameManager_.Update(dt);
}

void ClientNetworkManager::Destroy()
{
    gameManager_.Destroy();
}

void ClientNetworkManager::DrawImGui()
{
    const auto windowName = "Client "+std::to_string(clientId_);
    ImGui::Begin(windowName.c_str());
    const size_t bufferSize = 100;
    char hostBuffer[bufferSize];
    std::strncpy(hostBuffer, host_.c_str(), bufferSize);
    if(ImGui::InputText("Host", hostBuffer, bufferSize))
    {
        host_ = hostBuffer;
    }
    int portBuffer = tcpPort_;
    if(ImGui::InputInt("Port", &portBuffer))
    {
        tcpPort_ = portBuffer;
    }
    if(gameManager_.GetPlayerNumber() == INVALID_PLAYER &&
        ImGui::Button("Join"))
    {

        sf::Socket::Status status = tcpSocket_.connect(host_, tcpPort_);
        if(status == sf::Socket::Done)
        {
            tcpSocket_.setBlocking(false);

            auto joinPacket = std::make_unique<asteroid::JoinPacket>();
            auto* clientIdPtr = reinterpret_cast<std::uint8_t*>(&clientId_);
            for (int i = 0; i < sizeof(clientId_); i++)
            {
                joinPacket->clientId[i] = clientIdPtr[i];
            }
            SendReliablePacket(std::move(joinPacket));
            //Need to send the packet on the unreliable channel
            joinPacket = std::make_unique<asteroid::JoinPacket>();
            clientIdPtr = reinterpret_cast<std::uint8_t*>(&clientId_);
            for (int i = 0; i < sizeof(clientId_); i++)
            {
                joinPacket->clientId[i] = clientIdPtr[i];
            }
            SendUnreliablePacket(std::move(joinPacket));

        }
        else
        {
            logDebug("Error trying to connect to "+host_+" with port: "+std::to_string(tcpPort_));
        }
    }
    gameManager_.DrawImGui();
    ImGui::End();
}

void ClientNetworkManager::Render()
{
    const auto& config = BasicEngine::GetInstance()->config;

    if (config.windowSize != windowSize_)
    {
        windowSize_ = config.windowSize;
        gameManager_.SetWindowSize(windowSize_ / Vec2u(2, 1));

    }

    gameManager_.Render();
}

void ClientNetworkManager::SendReliablePacket(std::unique_ptr<asteroid::Packet> packet)
{
    sf::Packet tcpPacket;
    GeneratePacket(tcpPacket, *packet);
    auto status = sf::Socket::Partial;
    while(status == sf::Socket::Partial)
    {
        status = tcpSocket_.send(tcpPacket);
    }
}

void ClientNetworkManager::SendUnreliablePacket(std::unique_ptr<asteroid::Packet> packet)
{
    sf::Packet udpPacket;
    GeneratePacket(udpPacket, *packet);
    udpSocket_.send(udpPacket, host_, udpPort_);
}

void ClientNetworkManager::ReceivePacket(sf::Packet &packet)
{
  asteroid::Packet tmpPacket;
  packet >> tmpPacket;
  switch (tmpPacket.packetType)
  {

  case asteroid::PacketType::JOIN:
  {

  }
  case asteroid::PacketType::SPAWN_PLAYER:
    break;
  case asteroid::PacketType::INPUT:
    break;
  case asteroid::PacketType::SPAWN_BULLET:
    break;
  case asteroid::PacketType::VALIDATE_STATE:
    break;
  case asteroid::PacketType::START_GAME:
    break;
  case asteroid::PacketType::JOIN_ACK:
  {
    auto joinAckPacket = asteroid::GenerateReceivedPacket<asteroid::JoinAckPacket>(packet);
    ClientId clientId = ConvertFromBinary<ClientId>(joinAckPacket->clientId);

    break;
  }
  case asteroid::PacketType::NONE:
    break;
  default: ;
  }
}


ClientNetworkManager::ClientNetworkManager() : gameManager_(*this)
{

}
}
