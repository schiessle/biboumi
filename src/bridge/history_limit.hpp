#pragma once

// Default values means no limit
struct HistoryLimit
{
  int stanzas{-1};
  std::string since{};
};
