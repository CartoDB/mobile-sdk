#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

#include "PackageManager.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "utils/Log.h"

#include <cstdint>
#include <memory>
#include <utility>
#include <algorithm>
#include <limits>
#include <time.h>

#include <stdext/utf8_filesystem.h>

#include <sqlite3pp.h>
#include <sqlite3ppext.h>

#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>

#include <rc5.h>
#include <sha.h>
#include <modes.h>
#include <filters.h>
#include <hex.h>

#define MINIZ_HEADER_FILE_ONLY
#include <miniz.c>

namespace {
    std::shared_ptr<carto::PackageMetaInfo> createPackageMetaInfo(const rapidjson::Value& value) {
        rapidjson::StringBuffer metaInfo;
        rapidjson::Writer<rapidjson::StringBuffer> writer(metaInfo);
        value.Accept(writer);
        carto::Variant var = carto::Variant::FromString(metaInfo.GetString());
        return std::make_shared<carto::PackageMetaInfo>(var);
    }
}

namespace carto {
    
    PackageManager::PackageManager(const std::string& packageListURL, const std::string& dataFolder, const std::string& serverEncKey, const std::string& localEncKey) :
        _packageListURL(packageListURL), _packageListFileName("serverpackages.json"), _dataFolder(dataFolder), _serverEncKey(serverEncKey), _localEncKey(localEncKey)
    {
        std::string taskDbFileName = "tasks_v1.sqlite";
        try {
            _taskQueue = std::make_shared<PersistentTaskQueue>(createLocalFilePath(taskDbFileName));
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager: Error while constructing PackageManager::PersistentTaskQueue: %s, trying to remove...", ex.what());
            _taskQueue.reset();
            utf8_filesystem::unlink(taskDbFileName.c_str());
            try {
                _taskQueue = std::make_shared<PersistentTaskQueue>(createLocalFilePath(taskDbFileName));
            }
            catch (const std::exception& ex) {
                throw FileException("Failed to create/open package manager task queue database", taskDbFileName);
            }
        }

        std::string packageDbFileName = "packages_v1.sqlite";
        try {
            _localDb = std::make_shared<sqlite3pp::database>(createLocalFilePath(packageDbFileName).c_str());
            InitializeDb(*_localDb, _serverEncKey + _localEncKey);
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager: Error while constructing PackageManager: %s, trying to remove", ex.what());
            _localDb.reset();
            utf8_filesystem::unlink(packageDbFileName.c_str());
            try {
                _localDb = std::make_shared<sqlite3pp::database>(createLocalFilePath(packageDbFileName).c_str());
                InitializeDb(*_localDb, _serverEncKey + _localEncKey);
            }
            catch (const std::exception& ex) {
                throw FileException("Failed to create/open package manager database", packageDbFileName);
            }
        }

        syncLocalPackages();
    }

    PackageManager::~PackageManager() {
        stop(true);
    }

    std::shared_ptr<PackageManagerListener> PackageManager::getPackageManagerListener() const {
        return _packageManagerListener.get();
    }

    void PackageManager::setPackageManagerListener(const std::shared_ptr<PackageManagerListener>& listener) {
        _packageManagerListener.set(listener);
    }

    void PackageManager::registerOnChangeListener(const std::shared_ptr<OnChangeListener>& listener) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _onChangeListeners.push_back(listener);
    }

    void PackageManager::unregisterOnChangeListener(const std::shared_ptr<OnChangeListener>& listener) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _onChangeListeners.erase(std::remove(_onChangeListeners.begin(), _onChangeListeners.end(), listener), _onChangeListeners.end());
    }

    bool PackageManager::start() {
        if (!_localDb) {
            return false;
        }

        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            if (!_stopped && _packageManagerThread) {
                return true;
            }
        }

        stop(true);

        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _stopped = false;
        _packageManagerThread = std::make_shared<std::thread>(std::bind(&PackageManager::run, this));
        Log::Info("PackageManager: Package manager started");
        return true;
    }

    void PackageManager::stop(bool wait) {
        if (!_localDb) {
            return;
        }

        std::shared_ptr<std::thread> packageManagerThread;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            if (!_stopped) {
                _stopped = true;
                _taskQueueCondition.notify_all();
                Log::Info("PackageManager: Stopping package manager");
            }
            packageManagerThread = _packageManagerThread;
        }

        if (packageManagerThread && wait) {
            packageManagerThread->join();

            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _packageManagerThread.reset();
            Log::Info("PackageManager: Package manager stopped");
        }
    }

    std::shared_ptr<BinaryData> PackageManager::loadTile(const MapTile& mapTile) const {
        try {
            // Try all packages, check if the tile is in the package tile mask. Start with the last package (the most recently downloaded)
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            for (auto it = _localPackages.rbegin(); it != _localPackages.rend(); it++) {
                const std::shared_ptr<PackageInfo>& packageInfo = *it;
                if (packageInfo->getPackageType() != PackageType::PACKAGE_TYPE_MAP) {
                    continue;
                }
                if (std::shared_ptr<PackageTileMask> tileMask = packageInfo->getTileMask()) {
                    if (tileMask->getTileStatus(mapTile) != PackageTileStatus::PACKAGE_TILE_STATUS_MISSING) {
                        if (std::shared_ptr<sqlite3pp::database> packageDb = getLocalPackageDb(packageInfo)) {
                            // Try to load the tile (this could fail, as tile masks may not be complete to the last zoom level)
                            sqlite3pp::query query(*packageDb, "SELECT tile_decrypt(tile_data, zoom_level, tile_column, tile_row) FROM tiles WHERE zoom_level=:zoom AND tile_column=:x AND tile_row=:y");
                            query.bind(":zoom", mapTile.getZoom());
                            query.bind(":x", mapTile.getX());
                            query.bind(":y", mapTile.getY());
                            for (auto qit = query.begin(); qit != query.end(); qit++) {
                                Log::Infof("PackageManager::loadTile: Using package %s", packageInfo->getPackageId().c_str());
                                const unsigned char* dataPtr = reinterpret_cast<const unsigned char*>(qit->get<const void*>(0));
                                std::size_t dataSize = qit->column_bytes(0);
                                return std::make_shared<BinaryData>(dataPtr, dataSize);
                            }
                        }
                    }
                }
            }
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager::loadTile: Exception %s", ex.what());
        }
        return std::shared_ptr<BinaryData>();
    }

    void PackageManager::accessPackageFiles(const std::vector<std::string>& packageIds, std::function<void(const std::map<std::string, std::shared_ptr<std::ifstream> >&)> callback) const {
        try {
            std::lock_guard<std::recursive_mutex> lock(_mutex);

            // Create instances to all open files
            std::map<std::string, std::shared_ptr<std::ifstream> > packageFileMap;
            for (const std::string& packageId : packageIds) {
                std::shared_ptr<std::ifstream> file;
                auto fileIt = _localPackageFileCache.find(packageId);
                if (fileIt == _localPackageFileCache.end()) {
                    for (auto it = _localPackages.rbegin(); it != _localPackages.rend(); it++) {
                        const std::shared_ptr<PackageInfo>& packageInfo = *it;
                        if (packageInfo->getPackageId() == packageId) {
                            std::string fileName = createLocalFilePath(createPackageFileName(packageInfo->getPackageId(), packageInfo->getPackageType(), packageInfo->getVersion()));
                            file = std::make_shared<std::ifstream>();
                            file->exceptions(std::ifstream::failbit | std::ifstream::badbit);
                            file->rdbuf()->pubsetbuf(0, 0);
                            file->open(fileName, std::ios::binary);
                            _localPackageFileCache[packageId] = file;
                            break;
                        }
                    }
                }
                else {
                    file = fileIt->second;
                }

                if (file) {
                    packageFileMap[packageId] = file;
                }
                else {
                    Log::Warnf("PackageManager::accessLocalPackages: Could not find package %s", packageId.c_str());
                }
            }

            // Use the callback
            callback(packageFileMap);
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager::accessLocalPackages: Exception %s", ex.what());
        }
    }
    
    std::vector<std::shared_ptr<PackageInfo> > PackageManager::getServerPackages() const {
        try {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            
            if (_serverPackageCache.empty()) {
                // Load package list and parse
                std::string packageListJson = loadPackageListJson(_packageListFileName);
                if (packageListJson.empty()) {
                    return std::vector<std::shared_ptr<PackageInfo> >();
                }
                rapidjson::Document packageListDoc;
                if (packageListDoc.Parse<rapidjson::kParseDefaultFlags>(packageListJson.c_str()).HasParseError()) {
                    throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, "Error while parsing package list");
                }

                // Create packages
                for (rapidjson::Value::ValueIterator jit = packageListDoc["packages"].Begin(); jit != packageListDoc["packages"].End(); jit++) {
                    rapidjson::Value& jsonPackageInfo = *jit;
                    std::string packageId = jsonPackageInfo["id"].GetString();
                    std::string packageURL = jsonPackageInfo["url"].GetString();
                    PackageType::PackageType packageType = PackageType::PACKAGE_TYPE_MAP;
                    if (packageURL.find(".nutigraph") != std::string::npos) {
                        packageType = PackageType::PACKAGE_TYPE_ROUTING;
                    }
                    std::shared_ptr<PackageMetaInfo> metaInfo;
                    if (jsonPackageInfo.HasMember("metainfo")) {
                        metaInfo = createPackageMetaInfo(jsonPackageInfo["metainfo"]);
                    }
                    std::shared_ptr<PackageTileMask> tileMask;
                    if (jsonPackageInfo.HasMember("tile_mask")) {
                        tileMask = std::make_shared<PackageTileMask>(jsonPackageInfo["tile_mask"].GetString());
                    }
                    auto packageInfo = std::make_shared<PackageInfo>(
                        packageId,
                        packageType,
                        jsonPackageInfo["version"].GetInt(),
                        jsonPackageInfo["size"].GetInt64(),
                        packageURL,
                        tileMask,
                        metaInfo
                    );
                    _serverPackageCache.push_back(packageInfo);
                }
            }
            return _serverPackageCache;
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager::getServerPackages: %s", ex.what());
        }
        return std::vector<std::shared_ptr<PackageInfo> >();
    }

    std::vector<std::shared_ptr<PackageInfo> > PackageManager::getLocalPackages() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _localPackages;
    }

    std::shared_ptr<PackageMetaInfo> PackageManager::getServerPackageListMetaInfo() const {
        try {
            // Parse package list file, load meta info
            std::string packageListJson = loadPackageListJson(_packageListFileName);
            if (packageListJson.empty()) {
                return std::shared_ptr<PackageMetaInfo>();
            }
            rapidjson::Document packageListDoc;
            if (packageListDoc.Parse<rapidjson::kParseDefaultFlags>(packageListJson.c_str()).HasParseError()) {
                throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, "Error while parsing package list");
            }
            if (packageListDoc.HasMember("metainfo")) {
                return createPackageMetaInfo(packageListDoc["metainfo"]);
            }
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager::getServerPackageListMetaInfo: %s", ex.what());
        }
        return std::shared_ptr<PackageMetaInfo>();
    }

    int PackageManager::getServerPackageListAge() const {
        // Use last modification time of package list file
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        utf8_filesystem::stat st;
        memset(&st, 0, sizeof(st));
        if (utf8_filesystem::fstat(createLocalFilePath(_packageListFileName).c_str(), &st) == 0) {
            return static_cast<int>(time(NULL) - st.st_mtime);
        }
        return std::numeric_limits<int>::max();
    }

    std::shared_ptr<PackageInfo> PackageManager::getServerPackage(const std::string& packageId) const {
        std::vector<std::shared_ptr<PackageInfo> > packages = getServerPackages();
        auto it = std::find_if(packages.begin(), packages.end(), [&packageId](const std::shared_ptr<PackageInfo>& packageInfo) {
            return packageInfo->getPackageId() == packageId;
        });
        if (it == packages.end()) {
            return std::shared_ptr<PackageInfo>();
        }
        return *it;
    }

    std::shared_ptr<PackageInfo> PackageManager::getLocalPackage(const std::string& packageId) const {
        std::vector<std::shared_ptr<PackageInfo> > packages = getLocalPackages();
        auto it = std::find_if(packages.begin(), packages.end(), [&packageId](const std::shared_ptr<PackageInfo>& packageInfo) {
            return packageInfo->getPackageId() == packageId;
        });
        if (it == packages.end()) {
            return std::shared_ptr<PackageInfo>();
        }
        return *it;
    }

    std::shared_ptr<PackageStatus> PackageManager::getLocalPackageStatus(const std::string& packageId, int version) const {
        if (!_localDb) {
            return std::shared_ptr<PackageStatus>();
        }

        try {
            std::lock_guard<std::recursive_mutex> lock(_mutex);

            // Check task queue first for the package
            for (int taskId : _taskQueue->getTaskIds()) {
                Task task = _taskQueue->getTask(taskId);
                if (task.packageId == packageId && (version == -1 || task.packageVersion == version)) {
                    return std::make_shared<PackageStatus>(task.action, isTaskPaused(taskId), static_cast<float>(task.progress));
                }
            }

            // Check if the package is ready
            for (const std::shared_ptr<PackageInfo>& localPackage : getLocalPackages()) {
                if (localPackage->getPackageId() == packageId && (version == -1 || localPackage->getVersion() == version)) {
                    return std::make_shared<PackageStatus>(PackageAction::PACKAGE_ACTION_READY, false, 100.0f);
                }
            }
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager::getLocalPackageStatus: %s", ex.what());
        }

        // No local package, no task => no status
        return std::shared_ptr<PackageStatus>();
    }

    bool PackageManager::startPackageListDownload() {
        if (!_localDb) {
            return false;
        }

        try {
            Task downloadTask;
            downloadTask.command = Task::DOWNLOAD_PACKAGE_LIST;
            int taskId = _taskQueue->scheduleTask(downloadTask);
            _taskQueue->setTaskPriority(taskId, std::numeric_limits<int>::max());
            updateTaskStatus(taskId, PackageAction::PACKAGE_ACTION_WAITING, 0);
            _taskQueueCondition.notify_one();
            return true;
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager::startPackageListDownload: %s", ex.what());
        }
        return false;
    }

    bool PackageManager::startPackageImport(const std::string& packageId, int version, const std::string& packageFileName) {
        if (!_localDb) {
            return false;
        }

        try {
            Task importTask;
            importTask.command = Task::IMPORT_PACKAGE;
            importTask.packageId = packageId;
            importTask.packageType = PackageType::PACKAGE_TYPE_MAP;
            importTask.packageVersion = version;
            importTask.packageLocation = packageFileName;
            int taskId = _taskQueue->scheduleTask(importTask);
            updateTaskStatus(taskId, PackageAction::PACKAGE_ACTION_WAITING, 0);
            _taskQueueCondition.notify_one();
            return true;
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager::startPackageImport: %s", ex.what());
        }
        return false;
    }

    bool PackageManager::startPackageDownload(const std::string& packageId) {
        if (!_localDb) {
            return false;
        }

        try {
            // Find server package
            std::shared_ptr<PackageInfo> package;
            for (const std::shared_ptr<PackageInfo>& serverPackage : getServerPackages()) {
                if (serverPackage->getPackageId() == packageId) {
                    package = serverPackage;
                }
            }
            if (!package) {
                int currentVersion = 0;
                std::shared_ptr<PackageInfo> localPackage = getLocalPackage(packageId);
                if (localPackage) {
                    currentVersion = localPackage->getVersion();
                }
                package = getCustomPackage(packageId, currentVersion + 1);
                if (!package) {
                    return false;
                }
            }

            // Create download task
            Task downloadTask;
            downloadTask.command = Task::DOWNLOAD_PACKAGE;
            downloadTask.packageId = package->getPackageId();
            downloadTask.packageType = package->getPackageType();
            downloadTask.packageVersion = package->getVersion();
            downloadTask.packageLocation = package->getServerURL();
            int taskId = _taskQueue->scheduleTask(downloadTask);
            updateTaskStatus(taskId, PackageAction::PACKAGE_ACTION_WAITING, 0);
            _taskQueueCondition.notify_one();
            return true;
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager::startPackageDownload: %s", ex.what());
        }
        return false;
    }

    void PackageManager::cancelPackageTasks(const std::string& packageId) {
        if (!_localDb) {
            return;
        }

        try {
            for (int taskId : _taskQueue->getTaskIds()) {
                Task task = _taskQueue->getTask(taskId);
                if (task.packageId == packageId) {
                    _taskQueue->cancelTask(taskId);
                    updateTaskStatus(taskId, task.action, task.progress / 100.0f);
                    _taskQueueCondition.notify_one();
                }
            }
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager::cancelPackageTasks: %s", ex.what());
        }
    }

    bool PackageManager::startPackageRemove(const std::string& packageId) {
        if (!_localDb) {
            return false;
        }

        try {
            // Find local package
            std::shared_ptr<PackageInfo> package;
            for (const std::shared_ptr<PackageInfo>& localPackage : getLocalPackages()) {
                if (localPackage->getPackageId() == packageId) {
                    package = localPackage;
                }
            }
            if (!package) {
                return false;
            }

            // Cancel all previous tasks, as their results will be erased anyway
            for (int taskId : _taskQueue->getTaskIds()) {
                Task task = _taskQueue->getTask(taskId);
                if (task.packageId == packageId) {
                    _taskQueue->cancelTask(taskId);
                    updateTaskStatus(taskId, task.action, task.progress / 100.0f);
                }
            }

            // Add remove task
            Task removeTask;
            removeTask.command = Task::REMOVE_PACKAGE;
            removeTask.packageId = package->getPackageId();
            removeTask.packageType = package->getPackageType();
            removeTask.packageVersion = package->getVersion();
            int taskId = _taskQueue->scheduleTask(removeTask);
            updateTaskStatus(taskId, PackageAction::PACKAGE_ACTION_WAITING, 0);
            _taskQueueCondition.notify_one();
            return true;
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager::startPackageRemove: %s", ex.what());
        }
        return false;
    }

    void PackageManager::setPackagePriority(const std::string& packageId, int priority) {
        if (!_localDb) {
            return;
        }

        try {
            for (int taskId : _taskQueue->getTaskIds()) {
                Task task = _taskQueue->getTask(taskId);
                if (task.packageId == packageId) {
                    _taskQueue->setTaskPriority(taskId, priority);
                    updateTaskStatus(taskId, task.action, task.progress / 100.0f);
                    _taskQueueCondition.notify_one();
                }
            }
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager::setPackagePriority: %s", ex.what());
        }
    }

    void PackageManager::run() {
        try {
            while (true) {
                int taskId = -1;
                {
                    std::unique_lock<std::recursive_mutex> lock(_mutex);
                    if (_stopped) {
                        break;
                    }
                    taskId = _taskQueue->getActiveTaskId();
                    if (taskId == -1) {
                        _taskQueueCondition.wait(lock);
                        continue;
                    }
                }
                try {
                    Task::Command command = _taskQueue->getTask(taskId).command;
                    bool success = false;
                    switch (command) {
                    case Task::NOP:
                        success = true;
                        break;
                    case Task::DOWNLOAD_PACKAGE_LIST:
                        success = downloadPackageList(taskId);
                        break;
                    case Task::DOWNLOAD_PACKAGE:
                        success = downloadPackage(taskId);
                        break;
                    case Task::IMPORT_PACKAGE:
                        success = importPackage(taskId);
                        break;
                    case Task::REMOVE_PACKAGE:
                        success = removePackage(taskId);
                        break;
                    }
                    if (success) {
                        setTaskFinished(taskId);
                    }
                    else {
                        setTaskFailed(taskId, PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM);
                    }
                }
                catch (const PauseException&) {
                    setTaskPaused(taskId);
                    Log::Info("PackageManager: Paused task");
                }
                catch (const CancelException&) {
                    setTaskCancelled(taskId);
                    Log::Info("PackageManager: Cancelled task");
                }
                catch (const PackageException& ex) {
                    setTaskFailed(taskId, ex.getErrorType());
                    Log::Errorf("PackageManager: Exception while executing task: %s", ex.what());
                }
                catch (const std::exception& ex) {
                    setTaskFailed(taskId, PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM);
                    Log::Errorf("PackageManager: Exception while executing task: %s", ex.what());
                }
            }
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager: Unexpected exception while handling tasks, shutting down: %s", ex.what());
        }
    }

    bool PackageManager::downloadPackageList(int taskId) {
        // Download package list data
        std::vector<unsigned char> packageListData;
        for (int retry = 0; true; retry++) {
            if (retry > 0) {
                packageListData.clear();
                Log::Info("PackageManager: Retrying package list download");
            }

            int errorCode = DownloadFile(createPackageListURL(_packageListURL), [this, &packageListData, taskId](std::uint64_t offset, std::uint64_t length, const unsigned char* buf, std::size_t size) {
                if (isTaskCancelled(taskId)) {
                    return false;
                }
                if (isTaskPaused(taskId)) {
                    return false;
                }

                if (packageListData.size() != offset) {
                    packageListData.resize(static_cast<size_t>(offset));
                }
                packageListData.insert(packageListData.end(), buf, buf + size);
                return true;
            });

            if (errorCode == 0) {
                break;
            }
            if (isTaskCancelled(taskId)) {
                throw CancelException();
            }
            if (isTaskPaused(taskId)) {
                throw PauseException();
            }
            if (retry > 0) {
                throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_CONNECTION, "Failed to download package list");
            }
        }

        // Test if the data is gzipped, in that case inflate
        std::vector<unsigned char> packageListDataTemp;
        if (InflateData(packageListData, packageListDataTemp)) {
            std::swap(packageListData, packageListDataTemp);
        }

        // Create JSON string
        std::string packageListJson;
        if (!packageListData.empty()) {
            packageListJson.assign(reinterpret_cast<const char*>(&packageListData[0]), reinterpret_cast<const char*>(&packageListData[0] + packageListData.size()));
        }

        // Parse JSON, check that it is valid
        rapidjson::Document packageListDoc;
        if (packageListDoc.Parse<rapidjson::kParseDefaultFlags>(packageListJson.c_str()).HasParseError()) {
            throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, "Error while parsing package list");
        }
        if (!packageListDoc.HasMember("packages")) {
            throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, "Package list does not contain package definitions");
        }

        // Update package list file
        savePackageListJson(_packageListFileName, packageListJson);

        Log::Info("PackageManager: Package list updated");
        return true;
    }

    bool PackageManager::importPackage(int taskId) {
        Task task = _taskQueue->getTask(taskId);

        // Check if the package is already imported
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            sqlite3pp::query query(*_localDb, "SELECT id FROM packages WHERE package_id=:package_id AND version=:version AND valid=1");
            query.bind(":package_id", task.packageId.c_str());
            query.bind(":version", task.packageVersion);
            for (auto qit = query.begin(); qit != query.end(); qit++) {
                Log::Infof("PackageManager: Package %s already imported", task.packageId.c_str());
                return true;
            }
        }

        std::string packageFileName = createLocalFilePath(createPackageFileName(task.packageId, task.packageType, task.packageVersion));
        try {
            // Determine file size, copy file
            std::uint64_t fileSize = 0;
            {
                FILE* fpSrcRaw = utf8_filesystem::fopen(task.packageLocation.c_str(), "rb");
                if (!fpSrcRaw) {
                    throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, std::string("Could not open package file ") + task.packageLocation);
                }
                std::shared_ptr<FILE> fpSrc(fpSrcRaw, fclose);
                utf8_filesystem::fseek64(fpSrc.get(), 0, SEEK_END);
                fileSize = utf8_filesystem::ftell64(fpSrc.get());
                utf8_filesystem::fseek64(fpSrc.get(), 0, SEEK_SET);
                FILE* fpDestRaw = utf8_filesystem::fopen(packageFileName.c_str(), "wb");
                if (!fpDestRaw) {
                    throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, std::string("Could not create file ") + packageFileName);
                }
                std::shared_ptr<FILE> fpDest(fpDestRaw, fclose);
                unsigned char buf[4096];
                std::uint64_t fileOffset = 0;
                while (!feof(fpSrc.get())) {
                    if (isTaskCancelled(taskId)) {
                        throw CancelException();
                    }
                    if (isTaskPaused(taskId)) {
                        throw PauseException();
                    }

                    std::size_t n = fread(buf, sizeof(unsigned char), sizeof(buf) / sizeof(unsigned char), fpSrc.get());
                    if (fwrite(buf, sizeof(unsigned char), n, fpDest.get()) != n) {
                        throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, std::string("Could not write to file ") + packageFileName);
                    }
                    fileOffset += n;
                    updateTaskStatus(taskId, PackageAction::PACKAGE_ACTION_COPYING, static_cast<float>(fileOffset) / static_cast<float>(fileSize));
                }
            }

            // Find package tiles and calculate tile mask
            std::shared_ptr<PackageTileMask> tileMask;
            if (task.packageType == PackageType::PACKAGE_TYPE_MAP) {
                sqlite3pp::database packageDb(packageFileName.c_str());
                sqlite3pp::query query(packageDb, "SELECT zoom_level, tile_column, tile_row FROM tiles");
                std::vector<PackageTileMask::Tile> tiles;
                for (auto qit = query.begin(); qit != query.end(); qit++) {
                    PackageTileMask::Tile tile(qit->get<int>(0), qit->get<int>(1), qit->get<int>(2));
                    tiles.push_back(tile);
                }
                tileMask = std::make_shared<PackageTileMask>(tiles);
            }

            // Get package id
            int id = -1;
            {
                std::lock_guard<std::recursive_mutex> lock(_mutex);
                sqlite3pp::query query1(*_localDb, "SELECT id FROM packages WHERE package_id=:package_id AND version=:version");
                query1.bind(":package_id", task.packageId.c_str());
                query1.bind(":version", task.packageVersion);
                for (auto qit = query1.begin(); qit != query1.end(); qit++) {
                    id = qit->get<int>(0);
                }
                if (id == -1) {
                    sqlite3pp::command command(*_localDb, "INSERT INTO packages(package_id, package_type, version, size, server_url, tile_mask, metainfo, valid) VALUES(:package_id, :package_type, :version, :size, '', :tile_mask, '', 0)");
                    command.bind(":package_id", task.packageId.c_str());
                    command.bind(":package_type", static_cast<int>(task.packageType));
                    command.bind(":version", task.packageVersion);
                    command.bind(":size", fileSize);
                    command.bind(":tile_mask", tileMask ? tileMask->getStringValue().c_str() : "");
                    command.execute();
                    id = static_cast<int>(_localDb->last_insert_rowid());
                }
            }

            // Import package
            importLocalPackage(id, taskId, task.packageId, task.packageType, packageFileName);
        }
        catch (...) {
            utf8_filesystem::unlink(packageFileName.c_str());
            throw;
        }

        Log::Infof("PackageManager: Package %s imported", task.packageId.c_str());
        return true;
    }

    bool PackageManager::downloadPackage(int taskId) {
        Task task = _taskQueue->getTask(taskId);

        // Find the package info
        std::shared_ptr<PackageInfo> package;
        for (const std::shared_ptr<PackageInfo>& serverPackage : getServerPackages()) {
            if (serverPackage->getPackageId() == task.packageId && serverPackage->getVersion() == task.packageVersion) {
                package = serverPackage;
                break;
            }
        }
        if (!package) {
            package = getCustomPackage(task.packageId, task.packageVersion);
            if (!package) {
                Log::Errorf("PackageManager: Failed to find package %s to download", task.packageId.c_str());
                return false;
            }
        }

        // Check if the package is already downloaded
        bool downloaded = false;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            sqlite3pp::query query(*_localDb, "SELECT version FROM packages WHERE package_id=:package_id AND valid=1");
            query.bind(":package_id", task.packageId.c_str());
            for (auto qit = query.begin(); qit != query.end(); qit++) {
                if (qit->get<int>(0) == task.packageVersion && task.packageVersion != -1) {
                    Log::Infof("PackageManager: Package %s already downloaded", task.packageId.c_str());
                    return true;
                }
                downloaded = true;
            }
        }

        // Create new package file or reuse partly downloaded file
        bool packageSizeIndeterminate = package->getSize() == 0;
        std::string packageFileName = createLocalFilePath(createPackageFileName(task.packageId, task.packageType, task.packageVersion));
        try {
            // Try to download the package
            for (int retry = 0; true; retry++) {
                if (retry > 0) {
                    utf8_filesystem::unlink(packageFileName.c_str());
                    Log::Infof("PackageManager: Retrying package %s download", task.packageId.c_str());
                }
                FILE* fpRaw = utf8_filesystem::fopen(packageFileName.c_str(), "ab");
                if (!fpRaw) {
                    throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, std::string("Could not create download package file ") + packageFileName);
                }
                std::shared_ptr<FILE> fp(fpRaw, fclose);
                utf8_filesystem::fseek64(fp.get(), 0, SEEK_END);
                std::uint64_t fileOffset = utf8_filesystem::ftell64(fp.get());
                std::uint64_t fileSize = package->getSize();
                if (!packageSizeIndeterminate && fileOffset == fileSize) {
                    break;
                }
                if (fileSize > 0) {
                    updateTaskStatus(taskId, PackageAction::PACKAGE_ACTION_DOWNLOADING, static_cast<float>(fileOffset) / static_cast<float>(fileSize));
                }

                std::string packageURL = createPackageURL(task.packageId, task.packageVersion, task.packageLocation, downloaded);
                if (packageURL.empty()) {
                    throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_NO_OFFLINE_PLAN, "Offline packages not available");
                }
                int errorCode = DownloadFile(packageURL, [this, fp, taskId, packageFileName, &fileOffset, fileSize](std::uint64_t offset, std::uint64_t length, const unsigned char* buf, std::size_t size) {
                    if (isTaskCancelled(taskId)) {
                        return false;
                    }
                    if (isTaskPaused(taskId)) {
                        return false;
                    }

                    if (offset != fileOffset) {
                        Log::Infof("PackageManager: Truncating file");
                        utf8_filesystem::fseek64(fp.get(), offset, SEEK_SET);
                        utf8_filesystem::ftruncate64(fp.get(), offset);
                    }
                    if (fwrite(buf, sizeof(unsigned char), size, fp.get()) != size) {
                        Log::Errorf("PackageManager: Storage full? Could not write to package file %s", packageFileName.c_str());
                        return false;
                    }
                    fileOffset = offset + size;
                    std::uint64_t realSize = fileSize;
                    if (fileSize == 0 && length != std::numeric_limits<std::uint64_t>::max()) {
                        realSize = length;
                    }
                    if (realSize > 0) {
                        updateTaskStatus(taskId, PackageAction::PACKAGE_ACTION_DOWNLOADING, static_cast<float>(fileOffset) / static_cast<float>(realSize));
                    }
                    return true;
                }, fileOffset);

                if (errorCode == 0) {
                    if (packageSizeIndeterminate || fileOffset == fileSize) {
                        updateTaskStatus(taskId, PackageAction::PACKAGE_ACTION_DOWNLOADING, 1.0f);
                        break;
                    }
                    Log::Errorf("PackageManager: File size mismatch for package %s (expected %lld, actual %lld)", task.packageId.c_str(), static_cast<long long>(fileSize), static_cast<long long>(fileOffset));
                }
                if (isTaskCancelled(taskId)) {
                    throw CancelException();
                }
                if (isTaskPaused(taskId)) {
                    throw PauseException();
                }
                if (retry > 0) {
                    switch (errorCode) {
                    case 402: // Payment required
                        throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_DOWNLOAD_LIMIT_EXCEEDED, "Subscription limit exceeded while downloading: " + task.packageId);
                    case 403: // Forbidden
                        throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_NO_OFFLINE_PLAN, "Offline packages not available");
                        break;
                    case 406: // Not acceptable
                        throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_PACKAGE_TOO_BIG, "Package contains too many tiles: " + task.packageId);
                        break;
                    default:
                        throw PackageException(errorCode < 0 ? PackageErrorType::PACKAGE_ERROR_TYPE_CONNECTION : PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, "Failed to download package " + task.packageId);
                    }
                }
            }

            // Get package id, create package record
            int id = -1;
            {
                std::lock_guard<std::recursive_mutex> lock(_mutex);
                sqlite3pp::query query(*_localDb, "SELECT id FROM packages WHERE package_id=:package_id AND version=:version");
                query.bind(":package_id", task.packageId.c_str());
                query.bind(":version", task.packageVersion);
                for (auto qit = query.begin(); qit != query.end(); qit++) {
                    id = qit->get<int>(0);
                }
                if (id == -1) {
                    std::string metaInfo;
                    if (package->getMetaInfo()) {
                        metaInfo = package->getMetaInfo()->getVariant().toString();
                    }
                    std::string tileMask;
                    if (package->getTileMask()) {
                        tileMask = package->getTileMask()->getStringValue();
                    }
                    else if (package->getPackageType() == PackageType::PACKAGE_TYPE_MAP) {
                        sqlite3pp::database packageDb(packageFileName.c_str());
                        sqlite3pp::query query2(packageDb, "SELECT zoom_level, tile_column, tile_row FROM tiles");
                        std::vector<PackageTileMask::Tile> tiles;
                        for (auto qit2 = query2.begin(); qit2 != query2.end(); qit2++) {
                            PackageTileMask::Tile tile(qit2->get<int>(0), qit2->get<int>(1), qit2->get<int>(2));
                            tiles.push_back(tile);
                        }
                        tileMask = PackageTileMask(tiles).getStringValue();
                    }
                    std::uint64_t fileSize = package->getSize();
                    if (packageSizeIndeterminate) {
                        FILE* fpRaw = utf8_filesystem::fopen(packageFileName.c_str(), "rb");
                        if (fpRaw) {
                            std::shared_ptr<FILE> fp(fpRaw, fclose);
                            utf8_filesystem::fseek64(fp.get(), 0, SEEK_END);
                            fileSize = utf8_filesystem::ftell64(fp.get());
                        }
                    }
                    sqlite3pp::command command(*_localDb, "INSERT INTO packages(package_id, package_type, version, size, server_url, tile_mask, metainfo, valid) VALUES(:package_id, :package_type, :version, :size, :server_url, :tile_mask, :metainfo, 0)");
                    command.bind(":package_id", package->getPackageId().c_str());
                    command.bind(":package_type", static_cast<int>(package->getPackageType()));
                    command.bind(":version", package->getVersion());
                    command.bind(":size", fileSize);
                    command.bind(":server_url", package->getServerURL().c_str());
                    command.bind(":tile_mask", tileMask.c_str());
                    command.bind(":metainfo", metaInfo.c_str());
                    command.execute();
                    id = static_cast<int>(_localDb->last_insert_rowid());
                }
            }

            // Import download package
            importLocalPackage(id, taskId, task.packageId, task.packageType, packageFileName);
        }
        catch (const PauseException&) {
            throw;
        }
        catch (const CancelException&) {
            utf8_filesystem::unlink(packageFileName.c_str());
            throw;
        }
        catch (...) {
            utf8_filesystem::unlink(packageFileName.c_str());
            throw;
        }

        Log::Infof("PackageManager: Package %s downloaded", task.packageId.c_str());
        return true;
    }

    bool PackageManager::removePackage(int taskId) {
        Task task = _taskQueue->getTask(taskId);

        if (_taskQueue->isTaskCancelled(taskId)) {
            throw CancelException();
        }

        // Find package id
        int id = -1;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            sqlite3pp::query query(*_localDb, "SELECT id FROM packages WHERE package_id=:package_id AND version=:version");
            query.bind(":package_id", task.packageId.c_str());
            query.bind(":version", task.packageVersion);
            for (auto qit = query.begin(); qit != query.end(); qit++) {
                id = qit->get<int>(0);
            }
        }
        if (id == -1) {
            Log::Error("PackageManager: Failed to find package to remove");
            return false;
        }

        // Delete package
        updateTaskStatus(taskId, PackageAction::PACKAGE_ACTION_REMOVING, 0);
        deleteLocalPackage(id);
        updateTaskStatus(taskId, PackageAction::PACKAGE_ACTION_REMOVING, 100);

        Log::Infof("PackageManager: Package %s removed", task.packageId.c_str());
        return true;
    }

    void PackageManager::syncLocalPackages() {
        if (!_localDb) {
            return;
        }

        try {
            std::lock_guard<std::recursive_mutex> lock(_mutex);

            // Find all valid packages
            std::vector<std::shared_ptr<PackageInfo> > packages;
            sqlite3pp::query query(*_localDb, "SELECT package_id, package_type, version, size, server_url, tile_mask, metainfo FROM packages WHERE valid=1 ORDER BY id ASC");
            for (auto qit = query.begin(); qit != query.end(); qit++) {
                std::shared_ptr<PackageTileMask> tileMask;
                if (strlen(qit->get<const char*>(5)) != 0) {
                    tileMask = std::make_shared<PackageTileMask>(qit->get<const char*>(5));
                }
                std::shared_ptr<PackageMetaInfo> metaInfo;
                if (strlen(qit->get<const char*>(6)) != 0) {
                    rapidjson::Document metaInfoDoc;
                    if (metaInfoDoc.Parse<rapidjson::kParseDefaultFlags>(qit->get<const char*>(6)).HasParseError()) {
                        throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, "Error while parsing meta info");
                    }
                    metaInfo = createPackageMetaInfo(metaInfoDoc);
                }
                auto packageInfo = std::make_shared<PackageInfo>(
                    qit->get<const char*>(0),
                    static_cast<PackageType::PackageType>(qit->get<int>(1)),
                    qit->get<int>(2),
                    qit->get<std::uint64_t>(3),
                    qit->get<const char*>(4),
                    tileMask,
                    metaInfo
                    );
                packages.push_back(packageInfo);
            }

            // Update packages, clear db cache
            std::swap(_localPackages, packages);
            _localPackageDbCache.clear();
            for (auto it = _localPackageFileCache.begin(); it != _localPackageFileCache.end(); it++) {
                it->second->close();
            }
            _localPackageFileCache.clear();
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager::syncLocalPackages: %s", ex.what());
        }
    }

    std::shared_ptr<sqlite3pp::database> PackageManager::getLocalPackageDb(const std::shared_ptr<PackageInfo>& packageInfo) const {
        const int MAX_OPEN_PACKAGES = 4;

        // Try to find already open package from the cache
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        for (std::size_t i = 0; i < _localPackageDbCache.size(); i++) {
            if (_localPackageDbCache[i].packageId == packageInfo->getPackageId()) {
                PackageDatabase packageDatabase = _localPackageDbCache[i];
                if (i > 0) {
                    _localPackageDbCache.erase(_localPackageDbCache.begin() + i);
                    _localPackageDbCache.insert(_localPackageDbCache.begin(), packageDatabase);
                }
                return packageDatabase.packageDb;
            }
        }
        
        // Must open new database instance
        std::string packageFileName = createLocalFilePath(createPackageFileName(packageInfo->getPackageId(), packageInfo->getPackageType(), packageInfo->getVersion()).c_str());
        try {
            std::shared_ptr<sqlite3pp::database> packageDb = std::make_shared<sqlite3pp::database>(packageFileName.c_str());

            // Create new sqlite decryption function. First check if the database is crypted.
            std::string encKey = _serverEncKey;
            bool encrypted = CheckDbEncryption(*packageDb, _serverEncKey + _localEncKey); // NOTE: this is a hack - though tiles are actually encrypted with server key only, with check that local key is included in the hash also
            std::shared_ptr<sqlite3pp::ext::function> decryptFunc = std::make_shared<sqlite3pp::ext::function>(*packageDb);
            decryptFunc->create("tile_decrypt", [encrypted, encKey](sqlite3pp::ext::context& ctx) {
                const unsigned char* encData = reinterpret_cast<const unsigned char*>(ctx.get<const void*>(0));
                std::size_t encSize = ctx.args_bytes(0);
                int zoom = ctx.get<int>(1);
                int x = ctx.get<int>(2);
                int y = ctx.get<int>(3);
                std::vector<unsigned char> encVector(encData, encData + encSize);
                if (encrypted) {
                    DecryptTile(encVector, zoom, x, y, encKey);
                }
                ctx.result(encVector.empty() ? nullptr : &encVector[0], static_cast<int>(encVector.size()), false);
            }, 4);
            if (_localPackageDbCache.size() >= MAX_OPEN_PACKAGES) {
                _localPackageDbCache.pop_back();
            }

            // Save new package database instance
            PackageDatabase packageDatabase;
            packageDatabase.packageId = packageInfo->getPackageId();
            packageDatabase.packageDb = packageDb;
            packageDatabase.decryptFunc = decryptFunc;
            _localPackageDbCache.insert(_localPackageDbCache.begin(), packageDatabase);
            return packageDb;
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager::getLocalPackageDb: %s", ex.what());
        }
        return std::shared_ptr<sqlite3pp::database>();
    }

    void PackageManager::importLocalPackage(int id, int taskId, const std::string& packageId, PackageType::PackageType packageType, const std::string& packageFileName) {
        // Check if the package is encrypted.
        // We do a hack here - to make system more hack-proof, we should transcrypt already crypted packages, but this is a slow process.
        if (packageType == PackageType::PACKAGE_TYPE_MAP) {
            // Thus we will simply include local encryption key in the hash and keep original tiles.
            sqlite3pp::database packageDb(packageFileName.c_str());
            bool encrypted = CheckDbEncryption(packageDb, _serverEncKey);
            if (encrypted) {
                UpdateDbEncryption(packageDb, _serverEncKey + _localEncKey);
            }
        } else if (packageType == PackageType::PACKAGE_TYPE_ROUTING) {
            // We do not currently encrypt routing packages.
        }

        // Mark downloaded package as valid and older packages as invalid.
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        sqlite3pp::command command2(*_localDb, "UPDATE packages SET valid=(id=:id) WHERE package_id=:package_id");
        command2.bind(":id", id);
        command2.bind(":package_id", packageId.c_str());
        command2.execute();

        // Delete older invalid packages
        sqlite3pp::query query(*_localDb, "SELECT id FROM packages WHERE package_id=:package_id AND valid=0");
        query.bind(":package_id", packageId.c_str());
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            int otherId = qit->get<int>(0);
            deleteLocalPackage(otherId);
        }

        // Sync
        syncLocalPackages();
    }

    void PackageManager::deleteLocalPackage(int id) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);

            // Get package file name
            sqlite3pp::query query(*_localDb, "SELECT package_id, package_type, version FROM packages WHERE id=:id");
            query.bind(":id", id);
            std::string packageFileName;
            PackageType::PackageType packageType = PackageType::PACKAGE_TYPE_MAP;
            for (auto qit = query.begin(); qit != query.end(); qit++) {
                std::string packageId = qit->get<const char*>(0);
                packageType = static_cast<PackageType::PackageType>(qit->get<int>(1));
                int version = qit->get<int>(2);
                packageFileName = createLocalFilePath(createPackageFileName(packageId, packageType, version));
            }

            if (packageFileName.empty()) {
                return;
            }

            // Delete package from package list
            sqlite3pp::command command(*_localDb, "DELETE FROM packages WHERE id=:id");
            command.bind(":id", id);
            command.execute();
            
            // Delete file
            utf8_filesystem::unlink(packageFileName.c_str());
        }

        // Sync
        syncLocalPackages();
    }

    bool PackageManager::isTaskCancelled(int taskId) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _taskQueue->isTaskCancelled(taskId);
    }

    bool PackageManager::isTaskPaused(int taskId) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        if (_stopped) {
            return true;
        }
        return _taskQueue->getActiveTaskId(taskId) != taskId;
    }

    void PackageManager::updateTaskStatus(int taskId, PackageAction::PackageAction action, float progress) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            int roundedProgress = std::min(100, std::max(0, static_cast<int>(progress * 100.0f)));
            if (taskId == _prevTaskId && action == _prevAction && roundedProgress == _prevRoundedProgress) {
                return;
            }

            _taskQueue->updateTaskStatus(taskId, action, progress);
            _prevTaskId = taskId;
            _prevAction = action;
            _prevRoundedProgress = roundedProgress;
        }

        DirectorPtr<PackageManagerListener> packageManagerListener = _packageManagerListener;

        if (packageManagerListener) {
            Task task = _taskQueue->getTask(taskId);
            if (!task.packageId.empty()) {
                std::shared_ptr<PackageStatus> status = getLocalPackageStatus(task.packageId, task.packageVersion);
                if (status) {
                    packageManagerListener->onPackageStatusChanged(task.packageId, task.packageVersion, status);
                }
            }
        }
    }

    void PackageManager::setTaskFinished(int taskId) {
        std::vector<std::shared_ptr<OnChangeListener> > onChangeListeners;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            onChangeListeners = _onChangeListeners;
        }

        Task task = _taskQueue->getTask(taskId);
        _taskQueue->deleteTask(taskId);

        for (const std::shared_ptr<OnChangeListener>& onChangeListener : onChangeListeners) {
            onChangeListener->onTilesChanged();
        }

        DirectorPtr<PackageManagerListener> packageManagerListener = _packageManagerListener;

        if (packageManagerListener) {
            if (task.packageId.empty()) {
                packageManagerListener->onPackageListUpdated();
            }
            else {
                packageManagerListener->onPackageUpdated(task.packageId, task.packageVersion);
            }
        }
    }

    void PackageManager::setTaskPaused(int taskId) {
        Task task = _taskQueue->getTask(taskId);

        DirectorPtr<PackageManagerListener> packageManagerListener = _packageManagerListener;

        if (packageManagerListener) {
            if (!task.packageId.empty()) {
                std::shared_ptr<PackageStatus> status = getLocalPackageStatus(task.packageId, task.packageVersion);
                if (status) {
                    packageManagerListener->onPackageStatusChanged(task.packageId, task.packageVersion, status);
                }
            }
        }
    }

    void PackageManager::setTaskCancelled(int taskId) {
        Task task = _taskQueue->getTask(taskId);
        _taskQueue->deleteTask(taskId);

        DirectorPtr<PackageManagerListener> packageManagerListener = _packageManagerListener;

        if (packageManagerListener) {
            if (!task.packageId.empty()) {
                packageManagerListener->onPackageCancelled(task.packageId, task.packageVersion);
            }
        }
    }

    void PackageManager::setTaskFailed(int taskId, PackageErrorType::PackageErrorType errorType) {
        Task task = _taskQueue->getTask(taskId);
        _taskQueue->deleteTask(taskId);

        DirectorPtr<PackageManagerListener> packageManagerListener = _packageManagerListener;

        if (packageManagerListener) {
            if (task.packageId.empty()) {
                packageManagerListener->onPackageListFailed();
            }
            else {
                packageManagerListener->onPackageFailed(task.packageId, task.packageVersion, errorType);
            }
        }
    }

    std::string PackageManager::createLocalFilePath(const std::string& name) const {
        std::string fileName = _dataFolder;
        if (!fileName.empty()) {
            char c = fileName[fileName.size() - 1];
            if (c != '/' && c != '\\') {
                fileName.append("/");
            }
        }
        fileName.append("__Nuti_pkgmgr_" + name);
        return fileName;
    }

    std::string PackageManager::createPackageFileName(const std::string& packageId, PackageType::PackageType packageType, int version) const {
        std::stringstream ss;
        ss << packageId << "." << version;
        if (packageType == PackageType::PACKAGE_TYPE_MAP) {
            ss << ".mbtiles";
        } else if (packageType == PackageType::PACKAGE_TYPE_ROUTING) {
            ss << ".nutigraph";
        }
        return ss.str();
    }

    std::string PackageManager::createPackageListURL(const std::string& baseURL) const {
        return baseURL;
    }

    std::string PackageManager::createPackageURL(const std::string& packageId, int version, const std::string& baseURL, bool downloaded) const {
        return baseURL;
    }

    std::shared_ptr<PackageInfo> PackageManager::getCustomPackage(const std::string& packageId, int version) const {
        return std::shared_ptr<PackageInfo>();
    }

    std::string PackageManager::loadPackageListJson(const std::string& jsonFileName) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        std::string packageListFileName = createLocalFilePath(jsonFileName);
        FILE* fpRaw = utf8_filesystem::fopen(packageListFileName.c_str(), "rb");
        if (!fpRaw) {
            return std::string();
        }
        std::shared_ptr<FILE> fp(fpRaw, fclose);
        std::string json;
        while (!feof(fp.get())) {
            char buf[4096];
            std::size_t n = fread(buf, sizeof(char), sizeof(buf) / sizeof(char), fp.get());
            if (n == 0) {
                throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, std::string("Could not open read package list file ") + packageListFileName);
            }
            json.append(buf, n);
        }
        return json;
    }

    void PackageManager::savePackageListJson(const std::string& jsonFileName, const std::string& json) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        std::string packageListFileName = createLocalFilePath(jsonFileName);
        std::string tempPackageListFileName = createLocalFilePath(jsonFileName + ".tmp");
        FILE* fpRaw = utf8_filesystem::fopen(tempPackageListFileName.c_str(), "wb");
        if (!fpRaw) {
            throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, std::string("Could not create package list file ") + tempPackageListFileName);
        }
        std::shared_ptr<FILE> fp(fpRaw, fclose);
        if (fwrite(json.data(), sizeof(char), json.size(), fp.get()) != json.size()) {
            throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, std::string("Could not write to package list file ") + tempPackageListFileName);
        }
        fp.reset();
        utf8_filesystem::unlink(packageListFileName.c_str());
        if (utf8_filesystem::rename(tempPackageListFileName.c_str(), packageListFileName.c_str()) != 0) {
            throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, std::string("Could not rename package list file ") + tempPackageListFileName);
        }
        _serverPackageCache.clear();
    }

    void PackageManager::InitializeDb(sqlite3pp::database& db, const std::string& encKey) {
        db.execute("PRAGMA encoding='UTF-8'");
        db.execute(R"SQL(
                CREATE TABLE IF NOT EXISTS packages (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    package_id TEXT NOT NULL,
                    version INTEGER NOT NULL,
                    size INTEGER NOT NULL,
                    server_url TEXT NOT NULL,
                    tile_mask TEXT NOT NULL,
                    metainfo TEXT NOT NULL,
                    valid INTEGER NOT NULL DEFAULT 0
                    ))SQL");
        AddDbField(db, "packages", "package_type", "INTEGER NOT NULL DEFAULT 0");
        db.execute(R"SQL(
                CREATE TABLE IF NOT EXISTS metadata(
                    name TEXT,
                    value TEXT
                ))SQL");
        db.execute("CREATE INDEX IF NOT EXISTS packages_package_id ON packages(package_id)");
        
        std::string dbHash;
        sqlite3pp::query query(db, "SELECT value FROM metadata WHERE name='nutikeysha1'");
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            dbHash = qit->get<const char*>(0);
        }
        if (dbHash.empty()) {
            if (!encKey.empty()) {
                std::string sha1 = CalculateKeyHash(encKey);
                sqlite3pp::command command(db, "INSERT INTO metadata(name, value) VALUES('nutikeysha1', :hash)");
                command.bind(":hash", sha1.c_str());
                command.execute();
            }
        }
        else {
            CheckDbEncryption(db, encKey);
        }
    }

    bool PackageManager::AddDbField(sqlite3pp::database& db, const std::string& table, const std::string& field, const std::string& def) {
        sqlite3pp::query query(db, ("PRAGMA table_info(" + table + ")").c_str());
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            if (qit->get<const char *>(1) == field) {
                return false;
            }
        }
        db.execute(("ALTER TABLE " + table + " ADD COLUMN " + field + " " + def).c_str());
        return true;
    }

    bool PackageManager::CheckDbEncryption(sqlite3pp::database& db, const std::string& encKey) {
        sqlite3pp::query query(db, "SELECT value FROM metadata WHERE name='nutikeysha1'");
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            if (encKey.empty()) {
                throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, "Package database is encrypted and needs encryption key");
            }
            std::string sha1 = qit->get<const char*>(0);
            if (sha1 != CalculateKeyHash(encKey)) {
                throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, "Package encryption keys do not match");
            }
            return true;
        }
        return false;
    }

    void PackageManager::UpdateDbEncryption(sqlite3pp::database& db, const std::string& encKey) {
        sqlite3pp::transaction xct(db);
        {
            sqlite3pp::command command(db, "DELETE FROM metadata WHERE name='nutikeysha1'");
            command.execute();
            if (!encKey.empty()) {
                std::string sha1 = CalculateKeyHash(encKey);
                sqlite3pp::command command2(db, "INSERT INTO metadata(name, value) VALUES('nutikeysha1', :hash)");
                command2.bind(":hash", sha1.c_str());
                command2.execute();
            }
            xct.commit();
        }
    }
    
    std::string PackageManager::CalculateKeyHash(const std::string& encKey) {
        CryptoPP::SHA1 hash;
        unsigned char digest[CryptoPP::SHA1::DIGESTSIZE];
        hash.CalculateDigest(digest, reinterpret_cast<const unsigned char*>(encKey.c_str()), encKey.size());
        std::string sha1;
        CryptoPP::HexEncoder encoder;
        encoder.Attach(new CryptoPP::StringSink(sha1));
        encoder.Put(digest, sizeof(digest));
        encoder.MessageEnd();
        return sha1;
    }

    void PackageManager::EncryptTile(std::vector<unsigned char>& data, int zoom, int x, int y, const std::string& encKey) {
        if (data.empty()) {
            return;
        }
        
        unsigned char iv[CryptoPP::RC5::BLOCKSIZE];
        unsigned char k[CryptoPP::RC5::DEFAULT_KEYLENGTH];
        SetCipherKeyIV(k, iv, zoom, x, y, encKey);
        CryptoPP::CBC_Mode<CryptoPP::RC5>::Encryption enc;
        enc.SetKeyWithIV(k, sizeof(k), iv);
        std::string cipherText;
        cipherText.reserve(data.size() + 1);
        CryptoPP::StreamTransformationFilter stfEncryptor(enc, new CryptoPP::StringSink(cipherText), CryptoPP::StreamTransformationFilter::PKCS_PADDING); // NOTE: stfEncryptor will delete sink itself
        stfEncryptor.Put(&data[0], data.size());
        stfEncryptor.MessageEnd();
        data.assign(reinterpret_cast<const unsigned char*>(cipherText.data()), reinterpret_cast<const unsigned char*>(cipherText.data() + cipherText.size()));
    }
    
    void PackageManager::DecryptTile(std::vector<unsigned char>& data, int zoom, int x, int y, const std::string& encKey) {
        if (data.empty()) {
            return;
        }
        
        unsigned char iv[CryptoPP::RC5::BLOCKSIZE];
        unsigned char k[CryptoPP::RC5::DEFAULT_KEYLENGTH];
        SetCipherKeyIV(k, iv, zoom, x, y, encKey);
        CryptoPP::CBC_Mode<CryptoPP::RC5>::Decryption dec;
        dec.SetKeyWithIV(k, sizeof(k), iv);
        std::string plainText;
        plainText.reserve(data.size() + 1);
        CryptoPP::StreamTransformationFilter stfEncryptor(dec, new CryptoPP::StringSink(plainText), CryptoPP::StreamTransformationFilter::PKCS_PADDING); // NOTE: stfEncryptor will delete sink itself
        stfEncryptor.Put(&data[0], data.size());
        stfEncryptor.MessageEnd();
        data.assign(reinterpret_cast<const unsigned char*>(plainText.data()), reinterpret_cast<const unsigned char*>(plainText.data() + plainText.size()));
    }

    void PackageManager::SetCipherKeyIV(unsigned char* k, unsigned char* iv, int zoom, int x, int y, const std::string& encKey) {
        std::fill(iv, iv + CryptoPP::RC5::BLOCKSIZE, 0);
        iv[0 % CryptoPP::RC5::BLOCKSIZE] ^= static_cast<unsigned char>(zoom);
        iv[1 % CryptoPP::RC5::BLOCKSIZE] ^= 0;
        iv[2 % CryptoPP::RC5::BLOCKSIZE] ^= static_cast<unsigned char>((x >> 0)  & 255);
        iv[3 % CryptoPP::RC5::BLOCKSIZE] ^= static_cast<unsigned char>((x >> 8)  & 255);
        iv[4 % CryptoPP::RC5::BLOCKSIZE] ^= static_cast<unsigned char>((x >> 16) & 255);
        iv[5 % CryptoPP::RC5::BLOCKSIZE] ^= static_cast<unsigned char>((y >> 0)  & 255);
        iv[6 % CryptoPP::RC5::BLOCKSIZE] ^= static_cast<unsigned char>((y >> 8)  & 255);
        iv[7 % CryptoPP::RC5::BLOCKSIZE] ^= static_cast<unsigned char>((y >> 16) & 255);
        std::fill(k, k + CryptoPP::RC5::DEFAULT_KEYLENGTH, 0);
        std::copy(encKey.begin(), encKey.begin() + std::min(encKey.size(), static_cast<std::size_t>(CryptoPP::RC5::DEFAULT_KEYLENGTH)), k);
    }

    int PackageManager::DownloadFile(const std::string& url, NetworkUtils::HandlerFn handler, std::uint64_t offset) {
        std::map<std::string, std::string> requestHeaders;
        std::map<std::string, std::string> responseHeaders;
        return NetworkUtils::GetHTTP(url, requestHeaders, responseHeaders, handler, offset, Log::IsShowDebug());
    }

    bool PackageManager::InflateData(const std::vector<unsigned char>& in, std::vector<unsigned char>& out) {
        if (in.size() < 14) {
            return false;
        }

        std::size_t offset = 0;
        if (in[0] != 0x1f || in[1] != 0x8b) {
            return false;
        }
        if (in[2] != 8) {
            return false;
        }
        int flags = in[3];
        offset += 10;
        if (flags & (1 << 2)) { // FEXTRA
            int n = static_cast<int>(in[offset + 0]) | (static_cast<int>(in[offset + 1]) << 8);
            offset += n + 2;
        }
        if (flags & (1 << 3)) { // FNAME
            while (offset < in.size()) {
                if (in[offset++] == 0) {
                    break;
                }
            }
        }
        if (flags & (1 << 4)) { // FCOMMENT
            while (offset < in.size()) {
                if (in[offset++] == 0) {
                    break;
                }
            }
        }
        if (flags & (1 << 1)) { // FCRC
            offset += 2;
        }

        unsigned char buf[4096];
        ::mz_stream infstream;
        std::memset(&infstream, 0, sizeof(infstream));
        infstream.zalloc = Z_NULL;
        infstream.zfree = Z_NULL;
        infstream.opaque = Z_NULL;
        int err = Z_OK;
        infstream.avail_in = static_cast<unsigned int>(in.size() - offset - 4); // size of input
        infstream.next_in = reinterpret_cast<const unsigned char*>(&in[offset]); // input char array
        infstream.avail_out = sizeof(buf); // size of output
        infstream.next_out = buf; // output char array
        ::mz_inflateInit2(&infstream, -MZ_DEFAULT_WINDOW_BITS);
        do {
            infstream.avail_out = sizeof(buf); // size of output
            infstream.next_out = buf; // output char array
            err = ::mz_inflate(&infstream, infstream.avail_in > 0 ? MZ_NO_FLUSH : MZ_FINISH);
            if (err != MZ_OK && err != MZ_STREAM_END) {
                break;
            }
            out.insert(out.end(), buf, buf + sizeof(buf) - infstream.avail_out);
        } while (err != MZ_STREAM_END);
        ::mz_inflateEnd(&infstream);
        return err == MZ_OK || err == MZ_STREAM_END;
    }

    PackageManager::PersistentTaskQueue::PersistentTaskQueue(const std::string& dbFileName) {
        _localDb = std::make_shared<sqlite3pp::database>(dbFileName.c_str());
        _localDb->execute("PRAGMA encoding='UTF-8'");

        _localDb->execute(R"SQL(
                CREATE TABLE IF NOT EXISTS manager_tasks (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    command INTEGER NOT NULL,
                    priority INTEGER NOT NULL DEFAULT 0,
                    cancelled INTEGER NOT NULL DEFAULT 0,
                    action INTEGER NOT NULL DEFAULT 0,
                    progress INTEGER NOT NULL DEFAULT 0,
                    package_id TEXT,
                    package_version INTEGER,
                    package_location TEXT
                ))SQL");
        AddDbField(*_localDb, "manager_tasks", "package_type", "INTEGER DEFAULT 0");
        _localDb->execute("CREATE INDEX IF NOT EXISTS manager_tasks_package_id ON manager_tasks(package_id)");
    }

    int PackageManager::PersistentTaskQueue::getActiveTaskId(int currentActiveTaskId) const {
        // Find task with highest priority. Do not process paused tasks (priority < 0) unless they are cancelled. Cancelled tasks should be always processed.
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        sqlite3pp::query query(*_localDb, "SELECT id, package_id, (CASE WHEN id=:active_id THEN -1 ELSE id END) as ordering FROM manager_tasks WHERE priority>=0 OR cancelled=1 ORDER BY priority DESC, ordering ASC LIMIT 1");
        query.bind(":active_id", currentActiveTaskId);
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            int taskId = qit->get<int>(0);
            const char* packageId = qit->get<const char*>(1);
            if (!packageId) {
                return taskId;
            }

            // This is a package task - package tasks have to be processed in-order, so take the first task with the same package (even if it is paused).
            sqlite3pp::query query2(*_localDb, "SELECT id FROM manager_tasks WHERE package_id=:package_id ORDER BY id ASC LIMIT 1");
            query2.bind(":package_id", packageId);
            for (auto qit2 = query2.begin(); qit2 != query2.end(); qit2++) {
                return qit2->get<int>(0);
            }
        }
        return -1;
    }

    std::vector<int> PackageManager::PersistentTaskQueue::getTaskIds() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        sqlite3pp::query query(*_localDb, "SELECT id FROM manager_tasks");
        std::vector<int> taskIds;
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            taskIds.push_back(qit->get<int>(0));
        }
        return taskIds;
    }

    PackageManager::Task PackageManager::PersistentTaskQueue::getTask(int taskId) const {
        Task task;
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        sqlite3pp::query query(*_localDb, "SELECT command, priority, action, progress, package_id, package_type, package_version, package_location FROM manager_tasks WHERE id=:task_id");
        query.bind(":task_id", taskId);
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            task.command = static_cast<Task::Command>(qit->get<int>(0));
            task.priority = qit->get<int>(1);
            task.action = static_cast<PackageAction::PackageAction>(qit->get<int>(2));
            task.progress = qit->get<int>(3);
            task.packageId = qit->get<const char*>(4);
            task.packageType = static_cast<PackageType::PackageType>(qit->get<int>(5));
            task.packageVersion = qit->get<int>(6);
            task.packageLocation = qit->get<const char*>(7);
        }
        return task;
    }

    int PackageManager::PersistentTaskQueue::scheduleTask(const Task& task) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        sqlite3pp::command command(*_localDb, "INSERT INTO manager_tasks(command, action, progress, package_id, package_type, package_version, package_location) VALUES(:command, :action, :progress, :package_id, :package_type, :package_version, :package_location)");
        command.bind(":command", static_cast<int>(task.command));
        command.bind(":action", static_cast<int>(task.action));
        command.bind(":progress", task.progress);
        command.bind(":package_id", task.packageId.c_str());
        command.bind(":package_type", static_cast<int>(task.packageType));
        command.bind(":package_version", task.packageVersion);
        command.bind(":package_location", task.packageLocation.c_str());
        command.execute();
        int taskId = static_cast<int>(_localDb->last_insert_rowid());
        return taskId;
    }

    bool PackageManager::PersistentTaskQueue::isTaskCancelled(int taskId) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        sqlite3pp::query query(*_localDb, "SELECT cancelled FROM manager_tasks WHERE id=:task_id");
        query.bind(":task_id", taskId);
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            return qit->get<bool>(0);
        }
        return true;
    }

    void PackageManager::PersistentTaskQueue::cancelTask(int taskId) {
        // NOTE: cancelled tasks still need to be processed, do not delete them
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        sqlite3pp::command command(*_localDb, "UPDATE manager_tasks SET cancelled=1, priority=1000000 WHERE id=:task_id");
        command.bind(":task_id", taskId);
        command.execute();
    }

    void PackageManager::PersistentTaskQueue::setTaskPriority(int taskId, int priority) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        sqlite3pp::command command(*_localDb, "UPDATE manager_tasks SET priority=:priority WHERE id=:task_id");
        command.bind(":task_id", taskId);
        command.bind(":priority", priority);
        command.execute();
    }

    void PackageManager::PersistentTaskQueue::updateTaskStatus(int taskId, PackageAction::PackageAction action, float progress) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        sqlite3pp::command command(*_localDb, "UPDATE manager_tasks SET action=:action, progress=:progress WHERE id=:task_id");
        command.bind(":task_id", taskId);
        command.bind(":action", static_cast<int>(action));
        command.bind(":progress", std::min(100, std::max(0, static_cast<int>(progress * 100.0f))));
        command.execute();
    }

    void PackageManager::PersistentTaskQueue::deleteTask(int taskId) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        sqlite3pp::command command(*_localDb, "DELETE FROM manager_tasks WHERE id=:task_id");
        command.bind(":task_id", taskId);
        command.execute();
    }

}

#endif
