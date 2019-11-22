/**
 * @file
 */

#pragma once

#include "ServerMessages_generated.h"
#include "frontend/ClientEntity.h"
#include "frontend/Movement.h"
#include "voxelrender/WorldRenderer.h"
#include "voxelfont/VoxelFont.h"
#include "core/Var.h"
#include "core/Common.h"
#include "voxelworld/WorldEvents.h"
#include "network/ClientNetwork.h"
#include "network/ClientMessageSender.h"
#include "network/NetworkEvents.h"
#include "ui/turbobadger/UIApp.h"
#include "ui/turbobadger/WaitingMessage.h"
#include "animation/chr/CharacterCache.h"
#include "video/Camera.h"
#include "stock/StockDataProvider.h"

#include <stdlib.h>
#include <SDL.h>

// client states
constexpr uint32_t CLIENT_DISCONNECTED = 1 << 0;
constexpr uint32_t CLIENT_CONNECTING   = 1 << 1;
constexpr uint32_t CLIENT_CONNECTED    = 1 << 2;

class Client: public ui::turbobadger::UIApp, public core::IEventBusHandler<network::NewConnectionEvent>, public core::IEventBusHandler<
		network::DisconnectEvent>, public core::IEventBusHandler<voxelworld::WorldCreatedEvent> {
protected:
	using Super = ui::turbobadger::UIApp;
	video::Camera _camera;
	animation::CharacterCachePtr _characterCache;
	network::ClientNetworkPtr _network;
	voxelworld::WorldMgrPtr _world;
	network::ClientMessageSenderPtr _messageSender;
	voxelrender::WorldRenderer _worldRenderer;
	flatbuffers::FlatBufferBuilder _moveFbb;
	frontend::Movement _movement;
	network::MoveDirection _lastMoveMask = network::MoveDirection::NONE;
	core::VarPtr _rotationSpeed;
	core::VarPtr _maxTargetDistance;
	frontend::ClientEntityPtr _player;
	voxel::VoxelFont _voxelFont;
	ui::turbobadger::WaitingMessage _waiting;
	stock::StockDataProviderPtr _stockDataProvider;
	voxelformat::VolumeCachePtr _volumeCache;

	uint64_t _lastMovement = 0l;
	uint32_t _state = CLIENT_DISCONNECTED;

	float _fieldOfView = 60.0f;
	float _targetDistance = 28.0f;
	glm::vec3 _cameraPosition {1.0f, 0.4f, 1.0f};

	void setState(uint32_t flag);
	bool hasState(uint32_t flag) const;
	void removeState(uint32_t flag);

	frontend::ClientEntityId id() const;

	void sendMovement();
	void handleLogin();
	int renderMap(video::Shader& shader, const voxelworld::WorldMgrPtr& world, const glm::mat4& view, float aspect);
public:
	Client(const metric::MetricPtr& metric, const animation::CharacterCachePtr& characterCache,
			const stock::StockDataProviderPtr& stockDataProvider,
			const network::ClientNetworkPtr& network, const voxelworld::WorldMgrPtr& world,
			const network::ClientMessageSenderPtr& messageSender, const core::EventBusPtr& eventBus, const core::TimeProviderPtr& timeProvider,
			const io::FilesystemPtr& filesystem, const voxelformat::VolumeCachePtr& volumeCache);
	~Client();

	core::AppState onConstruct() override;
	core::AppState onInit() override;
	core::AppState onRunning() override;
	core::AppState onCleanup() override;
	void beforeUI() override;
	void afterRootWidget() override;
	bool onKeyPress(int32_t key, int16_t modifier) override;
	void onWindowResize(int windowWidth, int windowHeight) override;

	/**
	 * @brief We send the user connect message to the server and we get the seed and a user spawn message back.
	 *
	 * @note If auth failed, we get an auth failed message
	 */
	void onEvent(const network::NewConnectionEvent& event) override;
	void onEvent(const voxelworld::WorldCreatedEvent& event) override;
	void onEvent(const network::DisconnectEvent& event) override;

	bool connect(uint16_t port, const std::string& hostname);
	void authFailed();
	void signup(const std::string& email, const std::string& password);
	void lostPassword(const std::string& email);
	void disconnect();
	/** @brief spawns our own player */
	void spawn(frontend::ClientEntityId id, const char *name, const glm::vec3& pos, float orientation);

	void entitySpawn(frontend::ClientEntityId id, network::EntityType type, float orientation, const glm::vec3& pos);
	void entityUpdate(frontend::ClientEntityId id, const glm::vec3& pos, float orientation);
	void entityRemove(frontend::ClientEntityId id);
	frontend::ClientEntityPtr getEntity(frontend::ClientEntityId id) const;
};

inline frontend::ClientEntityPtr Client::getEntity(frontend::ClientEntityId id) const {
	return _worldRenderer.getEntity(id);
}

inline void Client::setState(uint32_t flag) {
	_state |= flag;
}

inline bool Client::hasState(uint32_t flag) const {
	return (_state & flag) != 0;
}

inline void Client::removeState(uint32_t flag) {
	_state &= ~flag;
}

inline frontend::ClientEntityId Client::id() const {
	if (!_player) {
		return -1;
	}
	return _player->id();
}

typedef std::shared_ptr<Client> ClientPtr;
