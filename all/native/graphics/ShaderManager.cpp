#include "ShaderManager.h"
#include "graphics/Shader.h"
#include "graphics/shaders/ShaderSource.h"
#include "utils/AssetUtils.h"
#include "utils/Log.h"

namespace carto {

    ShaderManager::ShaderManager() :
        _glThreadId(),
        _shaderMap(),
        _createQueue(),
        _deleteProgIdQueue(),
        _deleteShaderIdQueue(),
        _mutex()
    {
    }
    
    ShaderManager::~ShaderManager() {
    }
    
    std::thread::id ShaderManager::getGLThreadId() const {
        std::lock_guard<std::mutex> lock(_mutex);

        return _glThreadId;
    }

    void ShaderManager::setGLThreadId(std::thread::id id) {
        std::lock_guard<std::mutex> lock(_mutex);

        _glThreadId = id;
    }

    std::shared_ptr<Shader> ShaderManager::createShader(const ShaderSource& source) {
        std::lock_guard<std::mutex> lock(_mutex);

        auto it = _shaderMap.find(&source);
        if (it != _shaderMap.end()) {
            if (auto shader = it->second.lock()) {
                return shader;
            }
        }

        std::shared_ptr<Shader> shader(new Shader(shared_from_this(), source), [this](Shader* shader) { deleteShader(shader); });
        _shaderMap[&source] = shader;

        _createQueue.push_back(shader);
        return shader;
    }
    
    void ShaderManager::processShaders() {
        std::vector<std::weak_ptr<Shader> > createQueue;
        {
            std::lock_guard<std::mutex> lock(_mutex);

            if (std::this_thread::get_id() != _glThreadId) {
                Log::Warn("ShaderManager::processShaders: Method called from wrong thread!");
                return;
            }

            for (GLuint shaderId : _deleteShaderIdQueue) {
                glDeleteShader(shaderId);
            }
            _deleteShaderIdQueue.clear();

            for (GLuint progId : _deleteProgIdQueue) {
                glDeleteProgram(progId);
            }
            _deleteProgIdQueue.clear();

            for (const std::weak_ptr<Shader>& shaderWeak : _createQueue) {
                if (auto shader = shaderWeak.lock()) {
                    shader->load();
                }
            }
            std::swap(createQueue, _createQueue); // release the shaders only after lock is released
        }
    }

    void ShaderManager::deleteShader(Shader* shader) {
        std::lock_guard<std::mutex> lock(_mutex);

        if (shader) {
            if (std::this_thread::get_id() == _glThreadId) {
                shader->unload();
            }
            else {
                if (shader->_vertShaderId != 0) {
                    _deleteShaderIdQueue.push_back(shader->_vertShaderId);
                }
                if (shader->_fragShaderId != 0) {
                    _deleteShaderIdQueue.push_back(shader->_fragShaderId);
                }
                if (shader->_progId != 0) {
                    _deleteProgIdQueue.push_back(shader->_progId);
                }
            }
            delete shader;
        }
    }

}
