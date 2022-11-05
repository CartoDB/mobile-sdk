#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

#include "PackageManager.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "projections/Projection.h"
#include "projections/EPSG3857.h"
#include "packagemanager/handlers/PackageHandler.h"
#include "packagemanager/handlers/PackageHandlerFactory.h"
#include "utils/URLFileLoader.h"
#include "utils/GeneralUtils.h"
#include "utils/Log.h"

#include <cstdint>
#include <memory>
#include <utility>
#include <algorithm>
#include <limits>
#include <time.h>

#include <boost/lexical_cast.hpp>

#include <stdext/utf8_filesystem.h>
#include <stdext/zlib.h>

#include <sqlite3pp.h>
#include <sqlite3ppext.h>

#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>

#include <botan/botan_all.h>

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
        _packageListURL(packageListURL),
        _packageListFileName("serverpackages.json"),
        _dataFolder(dataFolder),
        _serverEncKey(serverEncKey),
        _localEncKey(localEncKey),
        _localPackages(),
        _localDb(),
        _taskQueue(),
        _taskQueueCondition(),
        _packageManagerThread(),
        _onChangeListeners(),
        _stopped(true),
        _prevTaskId(-1),
        _prevAction(PackageAction::PACKAGE_ACTION_WAITING),
        _prevRoundedProgress(0),
        _packageManagerListener(),
        _serverPackageCache(),
        _packageHandlerCache(),
        _packageFileMutex(),
        _mutex()
    {
        if (_packageListURL.empty()) {
            throw GenericException("Package list URL not defined");
        }

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
                Log::Errorf("PackageManager: Second error while constructing PackageManager::PersistentTaskQueue: %s", ex.what());
                throw FileException("Failed to create/open package manager task queue database", taskDbFileName);
            }
        }

        std::string packageDbFileName = "packages_v1.sqlite";
        try {
            _localDb = std::make_shared<sqlite3pp::database>(createLocalFilePath(packageDbFileName).c_str());
            InitializeDb(*_localDb, _serverEncKey + _localEncKey);
            syncLocalPackages();
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager: Error while constructing PackageManager: %s, trying to remove", ex.what());
            _localDb.reset();
            try {
                utf8_filesystem::unlink(packageDbFileName.c_str());
                _localDb = std::make_shared<sqlite3pp::database>(createLocalFilePath(packageDbFileName).c_str());
                InitializeDb(*_localDb, _serverEncKey + _localEncKey);
            }
            catch (const std::exception& ex) {
                Log::Errorf("PackageManager: Second attempt at constructing PackageManager failed: %s", ex.what());
                throw FileException("Failed to create/open package manager database", packageDbFileName);
            }
        }
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

    std::string PackageManager::getSchema() const {
        if (!_localDb) {
            return std::string();
        }

        try {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            sqlite3pp::query query(*_localDb, "SELECT value FROM metadata WHERE name='schema'");
            for (auto qit = query.begin(); qit != query.end(); qit++) {
                return qit->get<const char*>(0);
            }
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager::getSchema: %s", ex.what());
        }
        return std::string();
    }

    std::vector<std::shared_ptr<PackageInfo> > PackageManager::getServerPackages() const {
        try {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            
            if (!_serverPackageCache) {
                _serverPackageCache = std::make_shared<std::vector<std::shared_ptr<PackageInfo> > >();

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
                    PackageType::PackageType packageType = PackageHandlerFactory::DetectPackageType(packageURL);
                    std::shared_ptr<PackageMetaInfo> metaInfo;
                    if (jsonPackageInfo.HasMember("metainfo")) {
                        metaInfo = createPackageMetaInfo(jsonPackageInfo["metainfo"]);
                    }
                    std::shared_ptr<PackageTileMask> tileMask;
                    if (jsonPackageInfo.HasMember("tile_mask")) {
                        tileMask = DecodeTileMask(jsonPackageInfo["tile_mask"].GetString());
                    }
                    auto packageInfo = std::make_shared<PackageInfo>(
                        packageId,
                        packageType,
                        jsonPackageInfo["version"].IsString() ? boost::lexical_cast<int>(jsonPackageInfo["version"].GetString()) : jsonPackageInfo["version"].GetInt(),
                        jsonPackageInfo["size"].IsString() ? boost::lexical_cast<std::uint64_t>(jsonPackageInfo["size"].GetString()) : jsonPackageInfo["size"].GetInt64(),
                        packageURL,
                        tileMask,
                        metaInfo
                    );
                    _serverPackageCache->push_back(packageInfo);
                }
            }
            return *_serverPackageCache;
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

    void PackageManager::accessLocalPackages(const std::function<void(const std::map<std::shared_ptr<PackageInfo>, std::shared_ptr<PackageHandler> >&)>& callback) const {
        // NOTE: should use shared_lock here, but iOS 9 does not support it
        std::unique_lock<std::mutex> packageLock(_packageFileMutex);

        // Find all package handlers
        std::map<std::shared_ptr<PackageInfo>, std::shared_ptr<PackageHandler> > packageHandlerMap;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);

            for (const std::shared_ptr<PackageInfo>& packageInfo : _localPackages) {
                auto it = _packageHandlerCache.find(packageInfo);
                if (it == _packageHandlerCache.end()) {
                    std::string fileName = createLocalFilePath(createPackageFileName(packageInfo->getPackageId(), packageInfo->getPackageType(), packageInfo->getVersion()));
                    auto handler = PackageHandlerFactory(_serverEncKey, _localEncKey).createPackageHandler(packageInfo->getPackageType(), fileName);
                    if (!handler) {
                        continue;
                    }
                    it = _packageHandlerCache.insert(std::make_pair(packageInfo, handler)).first;
                }
                packageHandlerMap[packageInfo] = it->second;
            }
        }

        // Use the callback
        callback(packageHandlerMap);
    }
    
    std::vector<std::shared_ptr<PackageInfo> > PackageManager::suggestPackages(const MapPos& mapPos, const std::shared_ptr<Projection>& projection) const {
        if (!_localDb) {
            return std::vector<std::shared_ptr<PackageInfo> >();
        }

        if (!projection) {
            throw NullArgumentException("Null projection");
        }

        // Detect zoom level from tile masks
        int zoom = 0;
        for (const std::shared_ptr<PackageInfo>& packageInfo : getServerPackages()) {
            zoom = std::max(zoom, packageInfo->getTileMask()->getMaxZoomLevel());
        }

        // Calculate map tile from the map position
        MapTile mapTile = CalculateMapTile(mapPos, zoom, projection, false);

        // Find tile statuses from all packages. Keep only packages where the tile exists
        std::vector<std::pair<std::shared_ptr<PackageInfo>, PackageTileStatus::PackageTileStatus> > packageTileStatuses;
        while (true) {
            for (const std::shared_ptr<PackageInfo>& packageInfo : getServerPackages()) {
                PackageTileStatus::PackageTileStatus status = packageInfo->getTileMask()->getTileStatus(mapTile);
                if (status != PackageTileStatus::PACKAGE_TILE_STATUS_MISSING) {
                    packageTileStatuses.emplace_back(packageInfo, packageInfo->getTileMask()->getTileStatus(mapTile));
                }
            }
            if (!packageTileStatuses.empty() || mapTile.getZoom() == 0) {
                break;
            }

            mapTile = mapTile.getParent();
        }

        // Sort packages based on tile status and package size
        std::sort(packageTileStatuses.begin(), packageTileStatuses.end(), [](const std::pair<std::shared_ptr<PackageInfo>, PackageTileStatus::PackageTileStatus>& package1, const std::pair<std::shared_ptr<PackageInfo>, PackageTileStatus::PackageTileStatus>& package2) {
            if (package1.second != package2.second) {
                return package1.second == PackageTileStatus::PACKAGE_TILE_STATUS_FULL;
            }
            return package1.first->getSize() < package2.first->getSize();
        });

        // Return the packages
        std::vector<std::shared_ptr<PackageInfo> > packageInfos;
        std::transform(packageTileStatuses.begin(), packageTileStatuses.end(), std::back_inserter(packageInfos), [](const std::pair<std::shared_ptr<PackageInfo>, PackageTileStatus::PackageTileStatus>& package) {
            return package.first;
        });
        return packageInfos;
    }

    bool PackageManager::isAreaDownloaded(const MapBounds& mapBounds, int zoom, const std::shared_ptr<Projection>& projection) const {
        if (!_localDb) {
            return false;
        }

        if (!projection) {
            throw NullArgumentException("Null projection");
        }

        // Get local packages
        std::vector<std::shared_ptr<PackageInfo> > localPackages = getLocalPackages();

        // Calculate tile extents
        MapTile mapTile1 = CalculateMapTile(mapBounds.getMin(), zoom, projection, true);
        MapTile mapTile2 = CalculateMapTile(mapBounds.getMax(), zoom, projection, true);
        for (int y = std::min(mapTile1.getY(), mapTile2.getY()); y <= std::max(mapTile1.getY(), mapTile2.getY()); y++) {
            for (int x = std::min(mapTile1.getX(), mapTile2.getX()); x <= std::max(mapTile1.getX(), mapTile2.getX()); x++) {
                bool found = false;
                for (std::size_t i = 0; i < localPackages.size(); i++) {
                    if (localPackages[i]->getTileMask()->getTileStatus(MapTile(x, y, zoom, 0)) == PackageTileStatus::PACKAGE_TILE_STATUS_FULL) {
                        std::rotate(localPackages.begin(), localPackages.begin() + i, localPackages.begin() + i + 1);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    return false;
                }
            }
        }
        return true;
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
            importTask.packageType = PackageHandlerFactory::DetectPackageType(packageFileName);
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

    bool PackageManager::startStyleDownload(const std::string& styleName) {
        if (!_localDb) {
            return false;
        }

        try {
            Task downloadTask;
            downloadTask.command = Task::DOWNLOAD_STYLE;
            downloadTask.packageId = styleName;
            int taskId = _taskQueue->scheduleTask(downloadTask);
            _taskQueue->setTaskPriority(taskId, std::numeric_limits<int>::max());
            updateTaskStatus(taskId, PackageAction::PACKAGE_ACTION_WAITING, 0);
            _taskQueueCondition.notify_one();
            return true;
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager::startStyleDownload: %s", ex.what());
        }
        return false;
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
                    case Task::DOWNLOAD_STYLE:
                        success = downloadStyle(taskId);
                        break;
                    }
                    if (success) {
                        setTaskFinished(taskId);
                    } else {
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
                    packageListData.resize(static_cast<std::size_t>(offset));
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
        if (zlib::inflate_gzip(packageListData.data(), packageListData.size(), packageListDataTemp)) {
            packageListData = std::move(packageListDataTemp);
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

        // Check optional schema value. Always keep the schema up-to-date
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            sqlite3pp::command delCmd(*_localDb, "DELETE FROM metadata WHERE name='schema'");
            delCmd.execute();
            if (packageListDoc.HasMember("schema")) {
                sqlite3pp::command insCmd(*_localDb, "INSERT INTO metadata(name, value) VALUES('schema', :schema)");
                insCmd.bind(":schema", packageListDoc["schema"].GetString());
                insCmd.execute();
            }
        }

        // Update package list file and reset package cache
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            savePackageListJson(_packageListFileName, packageListJson);
            _serverPackageCache.reset();
        }

        // Done
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
                FILE* fpDestRaw = utf8_filesystem::fopen(packageFileName.c_str(), "wb");
                if (!fpDestRaw) {
                    throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, std::string("Could not create file ") + packageFileName);
                }
                std::shared_ptr<FILE> fpDest(fpDestRaw, fclose);

                updateTaskStatus(taskId, PackageAction::PACKAGE_ACTION_COPYING, 0.0f);

                std::string sourceURL = task.packageLocation;
                if (sourceURL.find("://") == std::string::npos) {
                    sourceURL = "file://" + sourceURL;
                }

                URLFileLoader loader;
                loader.setLocalFiles(true);
                bool result = loader.stream(sourceURL, [&](std::uint64_t length, const unsigned char* buf, std::size_t size) {
                    if (isTaskCancelled(taskId)) {
                        return false;
                    }

                    if (fwrite(buf, sizeof(unsigned char), size, fpDest.get()) != size) {
                        Log::Errorf("PackageManager: Storage full? Could not write to package file %s", packageFileName.c_str());
                        return false;
                    }
                    fileSize += size;
                    if (length > 0) {
                        updateTaskStatus(taskId, PackageAction::PACKAGE_ACTION_COPYING, static_cast<float>(fileSize) / static_cast<float>(length));
                    }
                    return true;
                });

                if (isTaskCancelled(taskId)) {
                    throw CancelException();
                }
                if (!result) {
                    throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, "Failed to import package " + task.packageId);
                }
                updateTaskStatus(taskId, PackageAction::PACKAGE_ACTION_COPYING, 1.0f);
            }

            // Find package tiles and calculate tile mask
            std::string tileMaskValue;
            if (auto handler = PackageHandlerFactory(_serverEncKey, _localEncKey).createPackageHandler(task.packageType, packageFileName)) {
                tileMaskValue = EncodeTileMask(handler->calculateTileMask());
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
                    command.bind(":tile_mask", tileMaskValue.c_str());
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

            // Read/calculate tile mask
            std::string tileMaskValue;
            if (package->getTileMask()) {
                tileMaskValue = EncodeTileMask(package->getTileMask());
            } else if (auto handler = PackageHandlerFactory(_serverEncKey, _localEncKey).createPackageHandler(task.packageType, packageFileName)) {
                tileMaskValue = EncodeTileMask(handler->calculateTileMask());
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
                    command.bind(":tile_mask", tileMaskValue.c_str());
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

    bool PackageManager::downloadStyle(int taskId) {
        Task task = _taskQueue->getTask(taskId);

        if (_taskQueue->isTaskCancelled(taskId)) {
            throw CancelException();
        }

        if (updateStyle(task.packageId)) {
            notifyStylesChanged();
        }

        return true;
    }

    void PackageManager::syncLocalPackages() {
        // Find all valid packages
        std::vector<std::shared_ptr<PackageInfo> > packages;
        sqlite3pp::query query(*_localDb, "SELECT package_id, package_type, version, size, server_url, tile_mask, metainfo FROM packages WHERE valid=1 ORDER BY id ASC");
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            std::shared_ptr<PackageTileMask> tileMask;
            if (strlen(qit->get<const char*>(5)) != 0) {
                tileMask = DecodeTileMask(qit->get<const char*>(5));
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

        // Update packages, sync caches
        _localPackages = std::move(packages);
        _packageHandlerCache.clear();
    }

    void PackageManager::importLocalPackage(int id, int taskId, const std::string& packageId, PackageType::PackageType packageType, const std::string& packageFileName) {
        // Invoke handler callback
        if (auto handler = PackageHandlerFactory(_serverEncKey, _localEncKey).createPackageHandler(packageType, packageFileName)) {
            handler->onImportPackage();
        }

        // Mark downloaded package as valid and older packages as invalid.
        std::vector<int> otherPackageIds;
        try {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            sqlite3pp::command command2(*_localDb, "UPDATE packages SET valid=(id=:id) WHERE package_id=:package_id");
            command2.bind(":id", id);
            command2.bind(":package_id", packageId.c_str());
            command2.execute();

            // Find older invalid packages
            sqlite3pp::query query(*_localDb, "SELECT id FROM packages WHERE package_id=:package_id AND valid=0");
            query.bind(":package_id", packageId.c_str());
            for (auto qit = query.begin(); qit != query.end(); qit++) {
                int otherId = qit->get<int>(0);
                otherPackageIds.push_back(otherId);
            }

            // Sync
            syncLocalPackages();
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager::importLocalPackage: %s", ex.what());
            return;
        }

        // Notify listeners
        notifyPackagesChanged(OnChangeListener::PACKAGES_ADDED);

        // Delete older invalid packages
        for (int otherId : otherPackageIds) {
            deleteLocalPackage(otherId);
        }
    }

    void PackageManager::deleteLocalPackage(int id) {
        std::unique_lock<std::mutex> packageLock(_packageFileMutex);

        // Find package info and if successful, remove the package
        std::string packageFileName;
        PackageType::PackageType packageType = PackageType::PACKAGE_TYPE_MAP;
        try {
            std::lock_guard<std::recursive_mutex> lock(_mutex);

            // Get package file name
            sqlite3pp::query query(*_localDb, "SELECT package_id, package_type, version FROM packages WHERE id=:id");
            query.bind(":id", id);
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

            // Sync
            syncLocalPackages();
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManager::deleteLocalPackage: %s", ex.what());
            return;
        }

        // Notify packages were deleted
        notifyPackagesChanged(OnChangeListener::PACKAGES_DELETED);

        // Invoke handler callback, delete file
        if (auto handler = PackageHandlerFactory(_serverEncKey, _localEncKey).createPackageHandler(packageType, packageFileName)) {
            handler->onDeletePackage();
        }
        utf8_filesystem::unlink(packageFileName.c_str());
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
            switch (task.command) {
            case Task::Command::DOWNLOAD_PACKAGE:
            case Task::Command::IMPORT_PACKAGE:
                if (std::shared_ptr<PackageStatus> status = getLocalPackageStatus(task.packageId, task.packageVersion)) {
                    packageManagerListener->onPackageStatusChanged(task.packageId, task.packageVersion, status);
                }
                break;
            default:
                break;
            }
        }
    }

    void PackageManager::setTaskFinished(int taskId) {
        Task task = _taskQueue->getTask(taskId);
        _taskQueue->deleteTask(taskId);

        DirectorPtr<PackageManagerListener> packageManagerListener = _packageManagerListener;

        if (packageManagerListener) {
            switch (task.command) {
            case Task::Command::DOWNLOAD_PACKAGE_LIST:
                packageManagerListener->onPackageListUpdated();
                break;
            case Task::Command::DOWNLOAD_PACKAGE:
            case Task::Command::IMPORT_PACKAGE:
            case Task::Command::REMOVE_PACKAGE:
                packageManagerListener->onPackageUpdated(task.packageId, task.packageVersion);
                break;
            case Task::Command::DOWNLOAD_STYLE:
                packageManagerListener->onStyleUpdated(task.packageId);
                break;
            default:
                break;
            }
        }
    }

    void PackageManager::setTaskPaused(int taskId) {
        Task task = _taskQueue->getTask(taskId);

        DirectorPtr<PackageManagerListener> packageManagerListener = _packageManagerListener;

        if (packageManagerListener) {
            switch (task.command) {
            case Task::Command::DOWNLOAD_PACKAGE:
            case Task::Command::IMPORT_PACKAGE:
            case Task::Command::REMOVE_PACKAGE:
                if (std::shared_ptr<PackageStatus> status = getLocalPackageStatus(task.packageId, task.packageVersion)) {
                    packageManagerListener->onPackageStatusChanged(task.packageId, task.packageVersion, status);
                }
                break;
            default:
                break;
            }
        }
    }

    void PackageManager::setTaskCancelled(int taskId) {
        Task task = _taskQueue->getTask(taskId);
        _taskQueue->deleteTask(taskId);

        DirectorPtr<PackageManagerListener> packageManagerListener = _packageManagerListener;

        if (packageManagerListener) {
            switch (task.command) {
            case Task::Command::DOWNLOAD_PACKAGE:
            case Task::Command::IMPORT_PACKAGE:
            case Task::Command::REMOVE_PACKAGE:
                packageManagerListener->onPackageCancelled(task.packageId, task.packageVersion);
                break;
            default:
                break;
            }
        }
    }

    void PackageManager::setTaskFailed(int taskId, PackageErrorType::PackageErrorType errorType) {
        Task task = _taskQueue->getTask(taskId);
        _taskQueue->deleteTask(taskId);

        DirectorPtr<PackageManagerListener> packageManagerListener = _packageManagerListener;

        if (packageManagerListener) {
            switch (task.command) {
            case Task::Command::DOWNLOAD_PACKAGE_LIST:
                packageManagerListener->onPackageListFailed();
                break;
            case Task::Command::DOWNLOAD_PACKAGE:
            case Task::Command::IMPORT_PACKAGE:
            case Task::Command::REMOVE_PACKAGE:
                packageManagerListener->onPackageFailed(task.packageId, task.packageVersion, errorType);
                break;
            case Task::Command::DOWNLOAD_STYLE:
                packageManagerListener->onStyleFailed(task.packageId);
                break;
            default:
                break;
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
        return packageId + "." + boost::lexical_cast<std::string>(version) + PackageHandlerFactory::GetPackageTypeExtension(packageType);
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

    bool PackageManager::updateStyle(const std::string& styleName) {
        return false;
    }

    void PackageManager::notifyPackagesChanged(OnChangeListener::PackageChangeType changeType) {
        std::vector<std::shared_ptr<OnChangeListener> > onChangeListeners;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            onChangeListeners = _onChangeListeners;
        }

        for (const std::shared_ptr<OnChangeListener>& onChangeListener : onChangeListeners) {
            onChangeListener->onPackagesChanged(changeType);
        }
    }

    void PackageManager::notifyStylesChanged() {
        std::vector<std::shared_ptr<OnChangeListener> > onChangeListeners;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            onChangeListeners = _onChangeListeners;
        }

        for (const std::shared_ptr<OnChangeListener>& onChangeListener : onChangeListeners) {
            onChangeListener->onStylesChanged();
        }
    }

    std::string PackageManager::loadPackageListJson(const std::string& jsonFileName) const {
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
    }

    void PackageManager::InitializeDb(sqlite3pp::database& db, const std::string& encKey) {
        db.execute("PRAGMA encoding='UTF-8'");
        db.execute(R"SQL(
                CREATE TABLE IF NOT EXISTS packages (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    package_id TEXT NOT NULL,
                    package_type INTEGER NOT NULL DEFAULT 0,
                    version INTEGER NOT NULL,
                    size INTEGER NOT NULL,
                    server_url TEXT NOT NULL,
                    tile_mask TEXT NOT NULL,
                    metainfo TEXT NOT NULL,
                    valid INTEGER NOT NULL DEFAULT 0
                ))SQL");
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
        } else {
            CheckDbEncryption(db, encKey);
        }
    }

    bool PackageManager::CheckDbEncryption(sqlite3pp::database& db, const std::string& encKey) {
        sqlite3pp::query query(db, "SELECT value FROM metadata WHERE name='nutikeysha1'");
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            if (encKey.empty()) {
                throw PackageException(PackageErrorType::PACKAGE_ERROR_TYPE_SYSTEM, "Package database is encrypted and needs encryption key");
            }
            std::string sha1 = qit->get<const char*>(0);
            if (sha1 != CalculateKeyHash(encKey)) {
                Log::Warn("Package encryption keys do not match");
            }
            return true;
        }
        return false;
    }

    std::string PackageManager::CalculateKeyHash(const std::string& encKey) {
        std::unique_ptr<Botan::HashFunction> hash(new Botan::SHA_160);
        hash->update(reinterpret_cast<const std::uint8_t*>(encKey.data()), encKey.size());
        return Botan::hex_encode(hash->final(), true);
    }

    MapTile PackageManager::CalculateMapTile(const MapPos& mapPos, int zoom, const std::shared_ptr<Projection>& proj, bool clip) {
        EPSG3857 epsg3857;
        double tileWidth = epsg3857.getBounds().getDelta().getX() / (1 << zoom);
        double tileHeight = epsg3857.getBounds().getDelta().getY() / (1 << zoom);
        MapVec mapVec = epsg3857.fromWgs84(proj->toWgs84(mapPos)) - epsg3857.getBounds().getMin();
        int x = static_cast<int>(std::floor(mapVec.getX() / tileWidth));
        int y = static_cast<int>(std::floor(mapVec.getY() / tileHeight));
        if (clip) {
            int maxExtent = (1 << zoom) - 1;
            x = std::max(0, std::min(maxExtent, x));
            y = std::max(0, std::min(maxExtent, y));
        }
        return MapTile(x, y, zoom, 0);
    }

    std::shared_ptr<PackageTileMask> PackageManager::DecodeTileMask(const std::string& tileMaskStr) {
        std::vector<std::string> parts = GeneralUtils::Split(tileMaskStr, ':');
        if (parts.empty()) {
            return std::shared_ptr<PackageTileMask>();
        }
        int zoomLevel = DEFAULT_TILEMASK_ZOOMLEVEL;
        if (parts.size() > 1) {
            zoomLevel = boost::lexical_cast<int>(parts[1]);
        }
        return std::make_shared<PackageTileMask>(parts[0], zoomLevel);
    }

    std::string PackageManager::EncodeTileMask(const std::shared_ptr<PackageTileMask>& tileMask) {
        if (!tileMask) {
            return std::string();
        }
        return tileMask->getStringValue() + ":" + boost::lexical_cast<std::string>(tileMask->getMaxZoomLevel());
    }

    int PackageManager::DownloadFile(const std::string& url, NetworkUtils::HandlerFunc handler, std::uint64_t offset) {
        Log::Debugf("PackageManager::DownloadFile: %s", url.c_str());
        std::map<std::string, std::string> requestHeaders = NetworkUtils::CreateAppRefererHeader();
        std::map<std::string, std::string> responseHeaders;
        return NetworkUtils::StreamHTTPResponse("GET", url, requestHeaders, responseHeaders, handler, offset, Log::IsShowDebug());
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
                    package_type INTEGER DEFAULT 0,
                    package_location TEXT
                ))SQL");
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

    const int PackageManager::DEFAULT_TILEMASK_ZOOMLEVEL = 14;

}

#endif
