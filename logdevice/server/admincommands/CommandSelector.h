/**
 * Copyright (c) 2017-present, Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#pragma once

#include <map>
#include <string>
#include <vector>

#include <folly/Memory.h>
#include <folly/io/Cursor.h>

namespace facebook { namespace logdevice {

class AdminCommand;

/**
 * @file Selects admin command based on command line.
 */

class CommandSelector {
 public:
  typedef std::function<std::unique_ptr<AdminCommand>(folly::io::Appender&)>
      CommandFactory;

  CommandSelector() {}

  void add(const char* prefix, CommandFactory factory);

  template <typename C, typename... Args>
  void add(const char* prefix, Args&&... args) {
    // Capturing parameter packs doesn't work with gcc older than 4.9.
    add(prefix,
        std::bind([](folly::io::Appender& out,
                     Args... a) { return std::make_unique<C>(out, a...); },
                  std::placeholders::_1,
                  std::forward<Args>(args)...));
  }

  // Returns matching command and removes from inout_command the prefix that
  // was used to select command.
  // If there's no matching command returns nullptr and writes error to output.
  std::unique_ptr<AdminCommand>
  selectCommand(std::vector<std::string>& inout_args,
                folly::io::Appender& output);

 private:
  struct TrieNode {
    CommandFactory factory;
    std::map<std::string, std::unique_ptr<TrieNode>> children;
  };

  TrieNode root_;
};

}} // namespace facebook::logdevice
