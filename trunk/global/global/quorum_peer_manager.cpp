/*
 * Copyright (C) Lichuang
 */
#include <google/protobuf/text_format.h>
#include <eventrpc/log.h>
#include <eventrpc/file_utility.h>
#include "quorum_peer_manager.h"
namespace global {
struct QuorumPeerManager::Impl {
 public:
  Impl();
  ~Impl();

  uint64 server_id() const;

  bool ParseConfigFile(const string &config_file);

  QuorumPeer* FindQuorumPeerById(uint64 server_id);
 private:
  map<uint64, QuorumPeer*> quorum_peer_map_;
  global::ServerConfig server_config_;
};

QuorumPeerManager::Impl::Impl() {
}

QuorumPeerManager::Impl::~Impl() {
  map<uint64, QuorumPeer*>::iterator iter;
  for (iter = quorum_peer_map_.begin();
       iter != quorum_peer_map_.end(); ) {
      QuorumPeer *peer = iter->second;
      ++iter;
      delete peer;
  }
  quorum_peer_map_.clear();
}

QuorumPeer* QuorumPeerManager::Impl::FindQuorumPeerById(uint64 server_id) {
  map<uint64, QuorumPeer*>::iterator iter;
  iter = quorum_peer_map_.find(server_id);
  if (iter == quorum_peer_map_.end()) {
    return NULL;
  }
  return iter->second;
}

uint64 QuorumPeerManager::Impl::server_id() const {
  return server_config_.server_id();
}

bool QuorumPeerManager::Impl::ParseConfigFile(const string &config_file) {
  string content;
  if (!eventrpc::FileUtility::ReadFileContents(config_file,
                                               &content)) {
    LOG_ERROR() << "cannot config file " << config_file;
    return false;
  }
  if (google::protobuf::TextFormat::ParseFromString(
      content, &server_config_)) {
    LOG_ERROR() << "parse from file " << config_file << " error";
    return false;
  }
  uint64 server_id = 0;
  for (int i = 0; i < server_config_.server_info_size(); ++i) {
    const global::ServerInfo &server_info = server_config_.server_info(i);
    server_id = server_info.server_id();
    QuorumPeer *peer = new QuorumPeer(
        server_id,
        server_info.server_address(),
        server_info.leader_port(),
        server_info.election_port());
    ASSERT(peer);
    quorum_peer_map_[server_id] = peer;
  }
}

QuorumPeerManager::QuorumPeerManager()
  : impl_(new Impl) {
}

QuorumPeerManager::~QuorumPeerManager() {
  delete impl_;
}


QuorumPeer* QuorumPeerManager::FindQuorumPeerById(uint64 server_id) {
  return impl_->FindQuorumPeerById(server_id);
}

uint64 QuorumPeerManager::server_id() const {
  return impl_->server_id();
}

bool QuorumPeerManager::ParseConfigFile(const string &config_file) {
  impl_->ParseConfigFile(config_file);
}
};