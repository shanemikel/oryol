//------------------------------------------------------------------------------
//  resourceMgr.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Render/base/meshLoaderBase.h"
#include "Render/base/textureLoaderBase.h"
#include "resourceMgr.h"

namespace Oryol {
namespace Render {

using namespace Core;
using namespace Resource;
    
//------------------------------------------------------------------------------
resourceMgr::resourceMgr() :
isValid(false),
stateWrapper(nullptr),
displayMgr(nullptr) {
    // empty
}

//------------------------------------------------------------------------------
resourceMgr::~resourceMgr() {
    o_assert(!this->isValid);
}

//------------------------------------------------------------------------------
void
resourceMgr::AttachLoader(meshLoaderBase* loader) {
    this->meshFactory.AttachLoader(loader);
}

//------------------------------------------------------------------------------
void
resourceMgr::AttachLoader(textureLoaderBase* loader) {
    this->textureFactory.AttachLoader(loader);
}
    
//------------------------------------------------------------------------------
void
resourceMgr::Setup(const RenderSetup& setup, class stateWrapper* stWrapper, class displayMgr* dspMgr) {
    o_assert(!this->isValid);
    o_assert(nullptr != stWrapper);
    o_assert(nullptr != dspMgr);
    
    this->isValid = true;
    this->stateWrapper = stWrapper;
    this->displayMgr = dspMgr;

    this->meshFactory.Setup(this->stateWrapper);
    this->meshPool.Setup(&this->meshFactory, setup.GetPoolSize(ResourceType::Mesh), setup.GetThrottling(ResourceType::Mesh), 'MESH');
    this->shaderFactory.Setup();
    this->shaderPool.Setup(&this->shaderFactory, setup.GetPoolSize(ResourceType::Shader), 0, 'SHDR');
    this->programBundleFactory.Setup(this->stateWrapper, &this->shaderPool, &this->shaderFactory);
    this->programBundlePool.Setup(&this->programBundleFactory, setup.GetPoolSize(ResourceType::ProgramBundle), 0, 'PRGB');
    this->textureFactory.Setup(this->stateWrapper, this->displayMgr, &this->texturePool);
    this->texturePool.Setup(&this->textureFactory, setup.GetPoolSize(ResourceType::Texture), setup.GetThrottling(ResourceType::Texture), 'TXTR');
    this->stateBlockFactory.Setup();
    this->stateBlockPool.Setup(&this->stateBlockFactory, setup.GetPoolSize(ResourceType::StateBlock), 0, 'SBLK');
    this->depthStencilStateFactory.Setup();
    this->depthStencilStatePool.Setup(&this->depthStencilStateFactory, setup.GetPoolSize(ResourceType::DepthStencilState), 0, 'DDST');
    this->blendStateFactory.Setup();
    this->blendStatePool.Setup(&this->blendStateFactory, setup.GetPoolSize(ResourceType::BlendState), 0, 'BLST');
    
    this->resourceRegistry.Setup(setup.GetResourceRegistryCapacity());
}

//------------------------------------------------------------------------------
void
resourceMgr::Discard() {
    o_assert(this->isValid);
    this->isValid = false;
    this->resourceRegistry.Discard();
    this->blendStatePool.Discard();
    this->blendStateFactory.Discard();
    this->depthStencilStatePool.Discard();
    this->depthStencilStateFactory.Discard();
    this->stateBlockPool.Discard();
    this->stateBlockFactory.Discard();
    this->texturePool.Discard();
    this->textureFactory.Discard();
    this->programBundlePool.Discard();
    this->programBundleFactory.Discard();
    this->shaderPool.Discard();
    this->shaderFactory.Discard();
    this->meshPool.Discard();
    this->meshFactory.Discard();
    this->stateWrapper = nullptr;
}

//------------------------------------------------------------------------------
bool
resourceMgr::IsValid() const {
    return this->isValid;
}

//------------------------------------------------------------------------------
void
resourceMgr::Update() {
    o_assert(this->isValid);
    
    // only call Update on pools which support asynchronous resource loading
    this->meshPool.Update();
    this->texturePool.Update();
}

//------------------------------------------------------------------------------
template<> Id
resourceMgr::CreateResource(const MeshSetup& setup) {
    o_assert(this->isValid);
    const Locator& loc = setup.GetLocator();
    Id resId = this->resourceRegistry.LookupResource(loc);
    if (resId.IsValid()) {
        o_assert(resId.Type() == ResourceType::Mesh);
        return resId;
    }
    else {
        resId = this->meshPool.AllocId();
        this->resourceRegistry.AddResource(loc, resId);
        this->meshPool.Assign(resId, setup);
        return resId;
    }
}

//------------------------------------------------------------------------------
template<> Id
resourceMgr::CreateResource(const MeshSetup& setup, const Ptr<IO::Stream>& data) {
    o_assert(this->isValid);
    const Locator& loc = setup.GetLocator();
    Id resId = this->resourceRegistry.LookupResource(loc);
    if (resId.IsValid()) {
        o_assert(resId.Type() == ResourceType::Mesh);
        return resId;
    }
    else {
        resId = this->meshPool.AllocId();
        this->resourceRegistry.AddResource(loc, resId);
        this->meshPool.Assign(resId, setup, data);
        return resId;
    }
}

//------------------------------------------------------------------------------
template<> Id
resourceMgr::CreateResource(const TextureSetup& setup) {
    o_assert(this->isValid);
    const Locator& loc = setup.GetLocator();
    Id resId = this->resourceRegistry.LookupResource(loc);
    if (resId.IsValid()) {
        o_assert(resId.Type() == ResourceType::Texture);
        return resId;
    }
    else {
        resId = this->texturePool.AllocId();
        this->resourceRegistry.AddResource(loc, resId);
        this->texturePool.Assign(resId, setup);
        return resId;
    }
}

//------------------------------------------------------------------------------
template<> Id
resourceMgr::CreateResource(const TextureSetup& setup, const Ptr<IO::Stream>& data) {
    o_assert(this->isValid);
    const Locator& loc = setup.GetLocator();
    Id resId = this->resourceRegistry.LookupResource(loc);
    if (resId.IsValid()) {
        o_assert(resId.Type() == ResourceType::Texture);
        return resId;
    }
    else {
        resId = this->texturePool.AllocId();
        this->resourceRegistry.AddResource(loc, resId);
        this->texturePool.Assign(resId, setup, data);
        return resId;
    }
}

//------------------------------------------------------------------------------
template<> Id
resourceMgr::CreateResource(const ShaderSetup& setup) {
    o_assert(this->isValid);
    const Locator& loc = setup.GetLocator();
    Id resId = this->resourceRegistry.LookupResource(loc);
    if (resId.IsValid()) {
        o_assert(resId.Type() == ResourceType::Shader);
        return resId;
    }
    else {
        resId = this->shaderPool.AllocId();
        this->resourceRegistry.AddResource(loc, resId);
        this->shaderPool.Assign(resId, setup);
        return resId;
    }
}

//------------------------------------------------------------------------------
template<> Id
resourceMgr::CreateResource(const ProgramBundleSetup& setup) {
    o_assert(this->isValid);
    const Locator& loc = setup.GetLocator();
    Id resId = this->resourceRegistry.LookupResource(loc);
    if (resId.IsValid()) {
        o_assert(resId.Type() == ResourceType::ProgramBundle);
        return resId;
    }
    else {
        resId = this->programBundlePool.AllocId();
        // add vertex/fragment shaders as dependencies
        Array<Id> deps;
        const int32 numProgs = setup.GetNumPrograms();
        deps.Reserve(numProgs * 2);
        for (int32 i = 0; i < setup.GetNumPrograms(); i++) {
            if (setup.GetVertexShader(i).IsValid()) {
                deps.AddBack(setup.GetVertexShader(i));
            }
            if (setup.GetFragmentShader(i).IsValid()) {
                deps.AddBack(setup.GetFragmentShader(i));
            }
        }
        this->resourceRegistry.AddResource(loc, resId, deps);
        this->programBundlePool.Assign(resId, setup);
        return resId;
    }
}

//------------------------------------------------------------------------------
template<> Id
resourceMgr::CreateResource(const StateBlockSetup& setup) {
    o_assert(this->isValid);
    const Locator& loc = setup.GetLocator();
    Id resId = this->resourceRegistry.LookupResource(loc);
    if (resId.IsValid()) {
        o_assert(resId.Type() == ResourceType::StateBlock);
        return resId;
    }
    else {
        resId = this->stateBlockPool.AllocId();
        this->resourceRegistry.AddResource(loc, resId);
        this->stateBlockPool.Assign(resId, setup);
        return resId;
    }
}

//------------------------------------------------------------------------------
template<> Id
resourceMgr::CreateResource(const DepthStencilStateSetup& setup) {
    o_assert(this->isValid);
    const Locator& loc = setup.GetLocator();
    Id resId = this->resourceRegistry.LookupResource(loc);
    if (resId.IsValid()) {
        o_assert(resId.Type() == ResourceType::DepthStencilState);
        return resId;
    }
    else {
        resId = this->depthStencilStatePool.AllocId();
        this->resourceRegistry.AddResource(loc, resId);
        this->depthStencilStatePool.Assign(resId, setup);
        return resId;
    }
}

//------------------------------------------------------------------------------
template<> Id
resourceMgr::CreateResource(const BlendStateSetup& setup) {
    o_assert(this->isValid);
    const Locator& loc = setup.GetLocator();
    Id resId = this->resourceRegistry.LookupResource(loc);
    if (resId.IsValid()) {
        o_assert(resId.Type() == ResourceType::BlendState);
        return resId;
    }
    else {
        resId = this->blendStatePool.AllocId();
        this->resourceRegistry.AddResource(loc, resId);
        this->blendStatePool.Assign(resId, setup);
        return resId;
    }
}
    
//------------------------------------------------------------------------------
Id
resourceMgr::LookupResource(const Locator& loc) {
    o_assert(this->isValid);
    return this->resourceRegistry.LookupResource(loc);
}

//------------------------------------------------------------------------------
void
resourceMgr::DiscardResource(const Id& resId) {
    o_assert(this->isValid);
    if (this->resourceRegistry.ReleaseResource(resId, this->removedIds) > 0) {
        // removedIds now has the resources which need to be destroyed
        for (const Id& removeId : this->removedIds) {
            switch (removeId.Type()) {
                case ResourceType::Texture:
                    this->texturePool.Unassign(removeId);
                    break;
                case ResourceType::Mesh:
                    this->meshPool.Unassign(removeId);
                    break;
                case ResourceType::Shader:
                    this->shaderPool.Unassign(removeId);
                    break;
                case ResourceType::ProgramBundle:
                    this->programBundlePool.Unassign(removeId);
                    break;
                case ResourceType::StateBlock:
                    this->stateBlockPool.Unassign(removeId);
                    break;
                case ResourceType::ConstantBlock:
                    o_assert2(false, "FIXME!!!\n");
                    break;
                case ResourceType::DepthStencilState:
                    this->depthStencilStatePool.Unassign(removeId);
                    break;
                case ResourceType::BlendState:
                    this->blendStatePool.Unassign(removeId);
                    break;
                default:
                    o_assert(false);
                    break;
            }
        }
    }
}

//------------------------------------------------------------------------------
Resource::State::Code
resourceMgr::QueryResourceState(const Id& resId) {
    o_assert(this->isValid);
    switch (resId.Type()) {
        case ResourceType::Texture:
            return this->texturePool.QueryState(resId);
        case ResourceType::Mesh:
            return this->meshPool.QueryState(resId);
        case ResourceType::Shader:
            return this->shaderPool.QueryState(resId);
        case ResourceType::ProgramBundle:
            return this->programBundlePool.QueryState(resId);
        case ResourceType::StateBlock:
            return this->stateBlockPool.QueryState(resId);
        case ResourceType::ConstantBlock:
            o_assert2(false, "FIXME!!!\n");
            break;
        case ResourceType::DepthStencilState:
            return this->depthStencilStatePool.QueryState(resId);
        case ResourceType::BlendState:
            return this->blendStatePool.QueryState(resId);
        default:
            o_assert(false);
            break;
    }
    return Resource::State::InvalidState;
}

//------------------------------------------------------------------------------
void
resourceMgr::createFullscreenQuadMesh(mesh& mesh) {
    this->meshFactory.createFullscreenQuad(mesh);
}

//------------------------------------------------------------------------------
void
resourceMgr::discardFullscreenQuadMesh(mesh& mesh) {
    this->meshFactory.DestroyResource(mesh);
    mesh.setState(Resource::State::Initial);
}

} // namespace Render
} // namespace Oryol