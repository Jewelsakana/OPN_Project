#ifndef DIRECTORYSERVICE_H
#define DIRECTORYSERVICE_H

#include "Model.h"
#include "DataStructures.h"
#include <string>
#include <memory>

class DirectoryService : public Model {
public:
    DirectoryService() = default;
    std::string getName() const override { return "DirectoryService"; }

    std::string getDirectoryTree(const std::string& path = "");
    std::shared_ptr<TreeNode> getDirectoryTreeStructure(const std::string& path = "");

protected:
    void handleException(const std::exception& e) const override;

private:
    std::string buildDirectoryTree(const std::string& path, const std::string& prefix, bool isLast);
    std::shared_ptr<TreeNode> buildDirectoryTreeStructure(const std::string& path);
    bool isFilesystemAvailable() const;
};

#endif // DIRECTORYSERVICE_H
