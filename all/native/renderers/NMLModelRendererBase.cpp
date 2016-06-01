#include "NMLModelRendererBase.h"
#include "graphics/Shader.h"
#include "graphics/ShaderManager.h"
#include "graphics/shaders/NMLShaderSource.h"
#include "graphics/ViewState.h"
#include "utils/Log.h"
#include "utils/GLES2.h"
#include "utils/GLUtils.h"

namespace carto {

    NMLModelRendererBase::GLContext::GLContext(const Shader &shader) :
        _shader(shader),
        _mvpMatrix(cglib::mat4x4<float>::identity())
    {
    }
    
    NMLModelRendererBase::GLContext::~GLContext() {
    }
    
    const Shader& NMLModelRendererBase::GLContext::getShader() {
        return _shader;
    }
    
    void NMLModelRendererBase::GLContext::setModelviewProjectionMatrix(const cglib::mat4x4<float>& mvpMatrix) {
        _mvpMatrix = mvpMatrix;
        glUniformMatrix4fv(_shader.getUniformLoc("u_mvpMat"), 1, GL_FALSE, mvpMatrix.data());
    }
    
    GLuint NMLModelRendererBase::GLContext::getUniformLocation(const char *name) const {
        return _shader.getUniformLoc(std::string("u_") + name);
    }
    
    GLuint NMLModelRendererBase::GLContext::getAttribLocation(const char *name) const {
        return _shader.getAttribLoc(std::string("a_") + name);
    }
    
    void NMLModelRendererBase::GLContext::setLocalModelviewMatrix(const float matrix[]) {
        cglib::mat4x4<float> localMat;
        localMat.copy(matrix);
        cglib::mat4x4<float> lmvpMatrix = _mvpMatrix * localMat;
        glUniformMatrix4fv(_shader.getUniformLoc("u_mvpMat"), 1, GL_FALSE, lmvpMatrix.data());
    }
    
    NMLModelRendererBase::NMLModelRendererBase() :
        _glContext()
    {
    }
    
    NMLModelRendererBase::~NMLModelRendererBase() {
    }
    
    void NMLModelRendererBase::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        _shader = shaderManager->createShader(nml_shader_source);
        _glContext.reset(new GLContext(*_shader));
    }
    
    bool NMLModelRendererBase::onDrawFrame(float deltaSeconds, const ViewState& viewState) {
        // Bind program
        glUseProgram(_glContext->getShader().getProgId());
    
        // Draw models
        glEnable(GL_DEPTH_TEST);
        bool refresh = drawModels(viewState);
        glDepthMask(GL_TRUE);
        glDisable(GL_DEPTH_TEST);
        
        GLUtils::checkGLError("NMLModelRendererBase::onDrawFrame()");
        return refresh;
    }
    
    void NMLModelRendererBase::onSurfaceDestroyed() {
        _glContext.reset();
        _shader.reset();
    }
    
}
