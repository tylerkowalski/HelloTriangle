#pragma once

#include <string>
#include <vector>

namespace ht {
class HtPipeline {
public:
  HtPipeline(const std::string &vertFilePath, const std::string &fragFilePath);

private:
  static std::vector<char> readFile(const std::string &filePath);

  void createGraphicsPipeline(const std::string &vertFilePath,
                              const std::string &fragFilePath);
};
} // namespace ht