/*=========================================================================

  Program:   Visocyte
  Module:    vtkSMVisocytePipelineController.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.visocyte.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMVisocytePipelineController.h"

#include "vtkCommand.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPVConfig.h"
#include "vtkPVInstantiator.h"
#include "vtkPVProxyDefinitionIterator.h"
#include "vtkPVXMLElement.h"
#include "vtkSMGlobalPropertiesProxy.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMPropertyIterator.h"
#include "vtkSMPropertyLink.h"
#include "vtkSMProxyDefinitionManager.h"
#include "vtkSMProxyInitializationHelper.h"
#include "vtkSMProxyIterator.h"
#include "vtkSMProxyListDomain.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMProxySelectionModel.h"
#include "vtkSMSession.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMSettings.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMTimeKeeperProxy.h"
#include "vtkSMTrace.h"
#include "vtkSmartPointer.h"
#include "vtkWeakPointer.h"

#include <cassert>
#include <map>
#include <set>
#include <sstream>
#include <vector>

class vtkSMVisocytePipelineController::vtkInternals
{
public:
  typedef std::map<void*, vtkTimeStamp> TimeStampsMap;
  TimeStampsMap InitializationTimeStamps;
  std::set<void*> ProxiesBeingUnRegistered;
};

namespace
{
// Used to monitor properties whose domains change.
class vtkDomainObserver
{
  std::vector<std::pair<vtkSMProperty*, unsigned long> > MonitoredProperties;
  std::set<vtkSMProperty*> PropertiesWithModifiedDomains;

  void DomainModified(vtkObject* sender, unsigned long, void*)
  {
    vtkSMProperty* prop = vtkSMProperty::SafeDownCast(sender);
    if (prop)
    {
      this->PropertiesWithModifiedDomains.insert(prop);
    }
  }

public:
  vtkDomainObserver() {}
  ~vtkDomainObserver()
  {
    for (size_t cc = 0; cc < this->MonitoredProperties.size(); cc++)
    {
      this->MonitoredProperties[cc].first->RemoveObserver(this->MonitoredProperties[cc].second);
    }
  }
  void Monitor(vtkSMProperty* prop)
  {
    assert(prop != NULL);
    unsigned long oid =
      prop->AddObserver(vtkCommand::DomainModifiedEvent, this, &vtkDomainObserver::DomainModified);
    this->MonitoredProperties.push_back(std::pair<vtkSMProperty*, unsigned long>(prop, oid));
  }

  const std::set<vtkSMProperty*>& GetPropertiesWithModifiedDomains() const
  {
    return this->PropertiesWithModifiedDomains;
  }
};

// method to create a new proxy safely i.e. not produce warnings if definition
// is not available.
inline vtkSMProxy* vtkSafeNewProxy(
  vtkSMSessionProxyManager* pxm, const char* group, const char* name)
{
  if (pxm && pxm->GetPrototypeProxy(group, name))
  {
    return pxm->NewProxy(group, name);
  }
  return NULL;
}

class vtkPrepareForUnregisteringScopedObj
{
  void* Proxy;
  std::set<void*>& ProxiesBeingUnRegistered;

public:
  vtkPrepareForUnregisteringScopedObj(void* proxy, std::set<void*>& theset)
    : Proxy(proxy)
    , ProxiesBeingUnRegistered(theset)
  {
    this->ProxiesBeingUnRegistered.insert(this->Proxy);
  }
  ~vtkPrepareForUnregisteringScopedObj() { this->ProxiesBeingUnRegistered.erase(this->Proxy); }
private:
  vtkPrepareForUnregisteringScopedObj(const vtkPrepareForUnregisteringScopedObj&);
  void operator=(const vtkPrepareForUnregisteringScopedObj&);
};
}

//---------------------------------------------------------------------------------------------------------
// This macros makes it easier to avoid running into unregistering loops due to dependency cycles
// when unregistering proxies. Any concrete method that unregisters proxies should simply call this
// macro.
#define PREPARE_FOR_UNREGISTERING(arg)                                                             \
  if (!arg)                                                                                        \
  {                                                                                                \
    return false;                                                                                  \
  }                                                                                                \
  if (this->Internals->ProxiesBeingUnRegistered.find(arg) !=                                       \
    this->Internals->ProxiesBeingUnRegistered.end())                                               \
  {                                                                                                \
    return true;                                                                                   \
  }                                                                                                \
  vtkPrepareForUnregisteringScopedObj __tmp(arg, this->Internals->ProxiesBeingUnRegistered);

vtkObjectFactoryNewMacro(vtkSMVisocytePipelineController);
//----------------------------------------------------------------------------
vtkSMVisocytePipelineController::vtkSMVisocytePipelineController()
  : Internals(new vtkSMVisocytePipelineController::vtkInternals())
{
}

//----------------------------------------------------------------------------
vtkSMVisocytePipelineController::~vtkSMVisocytePipelineController()
{
  delete this->Internals;
  this->Internals = NULL;
}

//----------------------------------------------------------------------------
vtkStdString vtkSMVisocytePipelineController::GetHelperProxyGroupName(vtkSMProxy* proxy)
{
  assert(proxy != NULL);
  std::ostringstream groupnamestr;
  groupnamestr << "pq_helper_proxies." << proxy->GetGlobalIDAsString();
  return groupnamestr.str();
}

//----------------------------------------------------------------------------
vtkSMProxy* vtkSMVisocytePipelineController::FindProxy(
  vtkSMSessionProxyManager* pxm, const char* reggroup, const char* xmlgroup, const char* xmltype)
{
  return pxm ? pxm->FindProxy(reggroup, xmlgroup, xmltype) : NULL;
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::CreateProxiesForProxyListDomains(vtkSMProxy* proxy)
{
  assert(proxy != NULL);
  vtkSmartPointer<vtkSMPropertyIterator> iter;
  iter.TakeReference(proxy->NewPropertyIterator());
  for (iter->Begin(); !iter->IsAtEnd(); iter->Next())
  {
    vtkSMProxyListDomain* pld = iter->GetProperty()
      ? vtkSMProxyListDomain::SafeDownCast(iter->GetProperty()->FindDomain("vtkSMProxyListDomain"))
      : NULL;
    if (pld)
    {
      pld->CreateProxies(proxy->GetSessionProxyManager());
      for (unsigned int cc = 0, max = pld->GetNumberOfProxies(); cc < max; cc++)
      {
        if (vtkSMProxy* dproxy = pld->GetProxy(cc))
        {
          // it makes sense to have all proxies in the ProxyListDomain have the
          // same location as the proxy to which the property belongs. Note this
          // may be different that proxy for cases where it's a property exposed
          // from a subproxy.
          vtkSMProxy* parentProxy = iter->GetProperty()->GetParent();
          dproxy->SetLocation(parentProxy->GetLocation());
          this->PreInitializeProxy(dproxy);
        }
      }
      // this is unnecessary here. only done for CompoundSourceProxy instances.
      // those proxies, we generally skip calling "reset" on in
      // PostInitializeProxy(). However, for properties that have proxy
      // list domains, we need to reset them (e.g ProbeLine filter).
      iter->GetProperty()->ResetToDomainDefaults();
    }
  }
  return true;
}

//----------------------------------------------------------------------------
void vtkSMVisocytePipelineController::RegisterProxiesForProxyListDomains(vtkSMProxy* proxy)
{
  assert(proxy != NULL);
  vtkSMSessionProxyManager* pxm = proxy->GetSessionProxyManager();

  std::string groupname = this->GetHelperProxyGroupName(proxy);

  vtkSmartPointer<vtkSMPropertyIterator> iter;
  iter.TakeReference(proxy->NewPropertyIterator());
  for (iter->Begin(); !iter->IsAtEnd(); iter->Next())
  {
    vtkSMProxyListDomain* pld = iter->GetProperty()
      ? vtkSMProxyListDomain::SafeDownCast(iter->GetProperty()->FindDomain("vtkSMProxyListDomain"))
      : NULL;
    if (!pld)
    {
      continue;
    }
    for (unsigned int cc = 0, max = pld->GetNumberOfProxies(); cc < max; cc++)
    {
      vtkSMProxy* plproxy = pld->GetProxy(cc);
      this->PostInitializeProxy(plproxy);
      pxm->RegisterProxy(groupname.c_str(), iter->GetKey(), plproxy);
    }
  }
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::SetupGlobalPropertiesLinks(vtkSMProxy* proxy)
{
  assert(proxy != NULL);

  vtkSMSessionProxyManager* pxm = proxy->GetSessionProxyManager();

  vtkSmartPointer<vtkSMPropertyIterator> iter;
  iter.TakeReference(proxy->NewPropertyIterator());
  for (iter->Begin(); !iter->IsAtEnd(); iter->Next())
  {
    vtkSMProperty* prop = iter->GetProperty();
    assert(prop);

    vtkPVXMLElement* linkHint =
      prop->GetHints() ? prop->GetHints()->FindNestedElementByName("GlobalPropertyLink") : NULL;
    if (linkHint)
    {
      std::string type = linkHint->GetAttributeOrEmpty("type");
      std::string property = linkHint->GetAttributeOrEmpty("property");
      if (type.empty() || property.empty())
      {
        vtkWarningMacro("Invalid GlobalPropertyLink hint.");
        continue;
      }

      vtkSMGlobalPropertiesProxy* gpproxy =
        vtkSMGlobalPropertiesProxy::SafeDownCast(pxm->GetProxy("global_properties", type.c_str()));
      if (!gpproxy)
      {
        continue;
      }
      if (!gpproxy->Link(property.c_str(), proxy, iter->GetKey()))
      {
        vtkWarningMacro("Failed to setup GlobalPropertyLink.");
      }
    }

    // Check for PropertyLinks
    linkHint = prop->GetHints() ? prop->GetHints()->FindNestedElementByName("PropertyLink") : NULL;
    if (linkHint)
    {
      const char* sourceGroupName = linkHint->GetAttributeOrEmpty("group");
      const char* sourceProxyName = linkHint->GetAttributeOrEmpty("proxy");
      const char* sourcePropertyName = linkHint->GetAttributeOrEmpty("property");
      if (!sourceGroupName || !sourceProxyName || !sourcePropertyName)
      {
        continue;
      }

      vtkSMProxy* sourceProxy = pxm->GetProxy(sourceGroupName, sourceProxyName);
      if (!sourceProxy)
      {
        continue;
      }

      vtkSMProperty* sourceProperty = sourceProxy->GetProperty(sourcePropertyName);
      if (sourceProperty)
      {
        sourceProperty->AddLinkedProperty(prop);
      }
      else
      {
        vtkWarningMacro(<< "No source property with group \"" << sourceGroupName << "\", proxy \""
                        << sourceProxyName << "\", property \"" << sourcePropertyName
                        << "\" exists. Linking not performed.")
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::CreateAnimationHelpers(vtkSMProxy* proxy)
{
  vtkSMSourceProxy* source = vtkSMSourceProxy::SafeDownCast(proxy);
  if (!source)
  {
    return false;
  }
  assert(proxy != NULL);
  vtkSMSessionProxyManager* pxm = proxy->GetSessionProxyManager();

  std::string groupname = this->GetHelperProxyGroupName(proxy);

  for (unsigned int cc = 0, max = source->GetNumberOfOutputPorts(); cc < max; cc++)
  {
    vtkSmartPointer<vtkSMProxy> helper;
    helper.TakeReference(vtkSafeNewProxy(pxm, "misc", "RepresentationAnimationHelper"));
    if (helper) // since this is optional
    {
      this->PreInitializeProxy(helper);
      vtkSMPropertyHelper(helper, "Source").Set(proxy);
      this->PostInitializeProxy(helper);
      helper->UpdateVTKObjects();

      // yup, all are registered under same name.
      pxm->RegisterProxy(groupname.c_str(), "RepresentationAnimationHelper", helper);
    }
  }

  return true;
}

//----------------------------------------------------------------------------
void vtkSMVisocytePipelineController::DoMaterialSetup(vtkSMProxy* vtkNotUsed(proxy))
{
  // expected to be overridden by rendering capable subclass
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::InitializeSession(vtkSMSession* session)
{
  assert(session != NULL);

  vtkSMSessionProxyManager* pxm = session->GetSessionProxyManager();
  assert(pxm);

  //---------------------------------------------------------------------------
  // If the session is a collaborative session, we need to fetch the state from
  // server before we start creating "essential" proxies. This is a no-op if not
  // a collaborative session.
  pxm->UpdateFromRemote();

  //---------------------------------------------------------------------------
  // Setup selection models used to track active view/active proxy.
  vtkSMProxySelectionModel* selmodel = pxm->GetSelectionModel("ActiveSources");
  if (selmodel == NULL)
  {
    selmodel = vtkSMProxySelectionModel::New();
    pxm->RegisterSelectionModel("ActiveSources", selmodel);
    selmodel->FastDelete();
  }

  selmodel = pxm->GetSelectionModel("ActiveView");
  if (selmodel == NULL)
  {
    selmodel = vtkSMProxySelectionModel::New();
    pxm->RegisterSelectionModel("ActiveView", selmodel);
    selmodel->FastDelete();
  }

  //---------------------------------------------------------------------------
  // Create the timekeeper if none exists.
  vtkSmartPointer<vtkSMProxy> timeKeeper = this->FindTimeKeeper(session);
  if (!timeKeeper)
  {
    timeKeeper.TakeReference(vtkSafeNewProxy(pxm, "misc", "TimeKeeper"));
    if (!timeKeeper)
    {
      vtkErrorMacro("Failed to create 'TimeKeeper' proxy. ");
      return false;
    }
    this->InitializeProxy(timeKeeper);
    timeKeeper->UpdateVTKObjects();

    pxm->RegisterProxy("timekeeper", timeKeeper);
  }

  //---------------------------------------------------------------------------
  // Create the materiallibrary
  vtkSmartPointer<vtkSMProxy> materialLib = this->FindMaterialLibrary(session);
  if (!materialLib)
  {
#if VTK_MODULE_ENABLE_VTK_RenderingOSPRay
    materialLib.TakeReference(vtkSafeNewProxy(pxm, "materials", "MaterialLibrary"));
    if (materialLib)
    {
      this->InitializeProxy(materialLib);
      materialLib->UpdateVTKObjects();
      this->DoMaterialSetup(materialLib.Get());
      pxm->RegisterProxy("materiallibrary", materialLib);
    }
#endif
  }

  //---------------------------------------------------------------------------
  // Create the animation-scene (optional)
  vtkSMProxy* animationScene = this->GetAnimationScene(session);
  if (animationScene)
  {
    // create time-animation track (optional)
    this->GetTimeAnimationTrack(animationScene);
  }

  //---------------------------------------------------------------------------
  // Setup global settings/state for the visualization state.
  this->UpdateSettingsProxies(session);

  //---------------------------------------------------------------------------
  // Setup color palette and proxies for other global property groups (optional)
  vtkSMProxy* proxy = pxm->GetProxy("global_properties", "ColorPalette");
  if (!proxy)
  {
    proxy = vtkSafeNewProxy(pxm, "misc", "ColorPalette");
    if (proxy)
    {
      this->InitializeProxy(proxy);
      pxm->RegisterProxy("global_properties", "ColorPalette", proxy);
      proxy->UpdateVTKObjects();
      proxy->Delete();
    }
  }
  return true;
}

//----------------------------------------------------------------------------
vtkSMProxy* vtkSMVisocytePipelineController::FindTimeKeeper(vtkSMSession* session)
{
  assert(session != NULL);

  vtkSMSessionProxyManager* pxm = session->GetSessionProxyManager();
  assert(pxm);

  return this->FindProxy(pxm, "timekeeper", "misc", "TimeKeeper");
}

//----------------------------------------------------------------------------
vtkSMProxy* vtkSMVisocytePipelineController::FindMaterialLibrary(vtkSMSession* session)
{
  assert(session != NULL);

  vtkSMSessionProxyManager* pxm = session->GetSessionProxyManager();
  assert(pxm);

  return this->FindProxy(pxm, "materiallibrary", "materials", "MaterialLibrary");
}

//----------------------------------------------------------------------------
vtkSMProxy* vtkSMVisocytePipelineController::FindAnimationScene(vtkSMSession* session)
{
  assert(session != NULL);

  vtkSMSessionProxyManager* pxm = session->GetSessionProxyManager();
  assert(pxm);

  return this->FindProxy(pxm, "animation", "animation", "AnimationScene");
}

//----------------------------------------------------------------------------
vtkSMProxy* vtkSMVisocytePipelineController::GetAnimationScene(vtkSMSession* session)
{
  assert(session != NULL);

  vtkSMSessionProxyManager* pxm = session->GetSessionProxyManager();
  assert(pxm);

  vtkSMProxy* timeKeeper = this->FindTimeKeeper(session);
  if (!timeKeeper)
  {
    return NULL;
  }

  vtkSmartPointer<vtkSMProxy> animationScene = this->FindAnimationScene(session);
  if (!animationScene)
  {
    animationScene.TakeReference(vtkSafeNewProxy(pxm, "animation", "AnimationScene"));
    if (animationScene)
    {
      this->PreInitializeProxy(animationScene);
      vtkSMPropertyHelper(animationScene, "TimeKeeper").Set(timeKeeper);
      this->PostInitializeProxy(animationScene);
      animationScene->UpdateVTKObjects();
      pxm->RegisterProxy("animation", animationScene);
    }
  }
  return animationScene.GetPointer();
}

//----------------------------------------------------------------------------
vtkSMProxy* vtkSMVisocytePipelineController::FindTimeAnimationTrack(vtkSMProxy* scene)
{
  if (!scene)
  {
    return NULL;
  }

  vtkSMProxy* timeKeeper = this->FindTimeKeeper(scene->GetSession());
  if (!timeKeeper)
  {
    return NULL;
  }

  vtkSMPropertyHelper helper(scene, "Cues", /*quiet*/ true);
  for (unsigned int cc = 0, max = helper.GetNumberOfElements(); cc < max; cc++)
  {
    vtkSMProxy* cue = helper.GetAsProxy(cc);
    if (cue && cue->GetXMLName() && strcmp(cue->GetXMLName(), "TimeAnimationCue") == 0 &&
      vtkSMPropertyHelper(cue, "AnimatedProxy").GetAsProxy() == timeKeeper)
    {
      vtkSMPropertyHelper pnameHelper(cue, "AnimatedPropertyName");
      if (pnameHelper.GetAsString(0) && strcmp(pnameHelper.GetAsString(0), "Time") == 0)
      {
        return cue;
      }
    }
  }
  return NULL;
}

//----------------------------------------------------------------------------
vtkSMProxy* vtkSMVisocytePipelineController::GetTimeAnimationTrack(vtkSMProxy* scene)
{
  vtkSmartPointer<vtkSMProxy> cue = this->FindTimeAnimationTrack(scene);
  if (cue || !scene)
  {
    return cue;
  }

  vtkSMProxy* timeKeeper = this->FindTimeKeeper(scene->GetSession());
  if (!timeKeeper)
  {
    return NULL;
  }

  vtkSMSessionProxyManager* pxm = scene->GetSessionProxyManager();
  assert(pxm);

  cue.TakeReference(vtkSafeNewProxy(pxm, "animation", "TimeAnimationCue"));
  if (!cue)
  {
    return NULL;
  }

  this->PreInitializeProxy(cue);
  vtkSMPropertyHelper(cue, "AnimatedProxy").Set(timeKeeper);
  vtkSMPropertyHelper(cue, "AnimatedPropertyName").Set("Time");
  this->PostInitializeProxy(cue);
  cue->UpdateVTKObjects();
  pxm->RegisterProxy("animation", cue);

  vtkSMPropertyHelper(scene, "Cues").Add(cue);
  scene->UpdateVTKObjects();

  return cue;
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::RegisterPipelineProxy(
  vtkSMProxy* proxy, const char* proxyname)
{
  if (!proxy)
  {
    return false;
  }

  SM_SCOPED_TRACE(RegisterPipelineProxy).arg("proxy", proxy);

  // Register proxies created for proxy list domains.
  this->RegisterProxiesForProxyListDomains(proxy);

  // Create animation helpers for this proxy.
  this->CreateAnimationHelpers(proxy);

  // Now register the proxy itself.
  // If proxyname is nullptr, the proxy manager makes up a name.
  if (proxyname == nullptr)
  {
    auto pname = proxy->GetSessionProxyManager()->RegisterProxy("sources", proxy);

    // assign a name for logging
    proxy->SetLogName(pname.c_str());
  }
  else
  {
    proxy->GetSessionProxyManager()->RegisterProxy("sources", proxyname, proxy);

    // assign a name for logging
    proxy->SetLogName(proxyname);
  }

  // Register proxy with TimeKeeper.
  vtkSMProxy* timeKeeper = this->FindTimeKeeper(proxy->GetSession());
  vtkSMTimeKeeperProxy::AddTimeSource(timeKeeper, proxy,
    /*suppress_input*/ (proxy->GetProperty("TimestepValues") != NULL ||
                                        proxy->GetProperty("TimeRange") != NULL));

  // Make the proxy active.
  vtkSMProxySelectionModel* selmodel =
    proxy->GetSessionProxyManager()->GetSelectionModel("ActiveSources");
  assert(selmodel != NULL);
  selmodel->SetCurrentProxy(proxy, vtkSMProxySelectionModel::CLEAR_AND_SELECT);

  return true;
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::UnRegisterPipelineProxy(vtkSMProxy* proxy)
{
  PREPARE_FOR_UNREGISTERING(proxy);

  vtkSMSessionProxyManager* pxm = proxy->GetSessionProxyManager();
  const char* _proxyname = pxm->GetProxyName("sources", proxy);
  if (_proxyname == NULL)
  {
    return false;
  }

  SM_SCOPED_TRACE(Delete).arg("proxy", proxy);

  const std::string proxyname(_proxyname);

  // ensure proxy is no longer active.
  vtkSMProxySelectionModel* selmodel = pxm->GetSelectionModel("ActiveSources");
  assert(selmodel != NULL);
  if (selmodel->GetCurrentProxy() == proxy)
  {
    selmodel->SetCurrentProxy(NULL, vtkSMProxySelectionModel::CLEAR_AND_SELECT);
  }

  // remove proxy from TimeKeeper.
  vtkSMProxy* timeKeeper = this->FindTimeKeeper(proxy->GetSession());
  vtkSMTimeKeeperProxy::RemoveTimeSource(timeKeeper, proxy,
    /*unsuppress_input*/ (proxy->GetProperty("TimestepValues") != NULL ||
                                           proxy->GetProperty("TimeRange") != NULL));

  // unregister dependencies.
  this->UnRegisterDependencies(proxy);

  // this will remove both proxy-list-domain helpers and animation helpers.
  this->FinalizeProxy(proxy);

  // unregister the proxy.
  pxm->UnRegisterProxy("sources", proxyname.c_str(), proxy);
  return true;
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::RegisterViewProxy(vtkSMProxy* proxy, const char* proxyname)
{
  if (!proxy)
  {
    return false;
  }

  SM_SCOPED_TRACE(RegisterViewProxy).arg("proxy", proxy);

  // Register proxies created for proxy list domains.
  this->RegisterProxiesForProxyListDomains(proxy);

  // Now register the proxy itself.
  proxy->GetSessionProxyManager()->RegisterProxy("views", proxyname, proxy);

  // Register proxy with TimeKeeper.
  vtkSMProxy* timeKeeper = this->FindTimeKeeper(proxy->GetSession());
  vtkSMPropertyHelper(timeKeeper, "Views").Add(proxy);
  timeKeeper->UpdateVTKObjects();

  // Register proxy with AnimationScene (optional)
  vtkSMProxy* scene = this->GetAnimationScene(proxy->GetSession());
  if (scene)
  {
    vtkSMPropertyHelper(scene, "ViewModules").Add(proxy);
    scene->UpdateVTKObjects();
  }

  vtkSmartPointer<vtkSMProxy> materialLib = this->FindMaterialLibrary(proxy->GetSession());
  if (materialLib)
  {
    // session has one, try and add it to the view
    // third argument 'true' stops complaints on incompatible view types
    vtkSMPropertyHelper(proxy, "OSPRayMaterialLibrary", true).Add(materialLib);
  }

  // Make the proxy active.
  vtkSMProxySelectionModel* selmodel =
    proxy->GetSessionProxyManager()->GetSelectionModel("ActiveView");
  assert(selmodel != NULL);
  selmodel->SetCurrentProxy(proxy, vtkSMProxySelectionModel::CLEAR_AND_SELECT);
  return true;
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::UnRegisterViewProxy(
  vtkSMProxy* proxy, bool unregister_representations /*=true*/)
{
  PREPARE_FOR_UNREGISTERING(proxy);

  vtkSMSessionProxyManager* pxm = proxy->GetSessionProxyManager();
  const char* _proxyname = pxm->GetProxyName("views", proxy);
  if (_proxyname == NULL)
  {
    return false;
  }

  SM_SCOPED_TRACE(Delete).arg("proxy", proxy);
  const std::string proxyname(_proxyname);

  // ensure proxy is no longer active.
  vtkSMProxySelectionModel* selmodel = pxm->GetSelectionModel("ActiveView");
  assert(selmodel != NULL);
  if (selmodel->GetCurrentProxy() == proxy)
  {
    selmodel->SetCurrentProxy(NULL, vtkSMProxySelectionModel::CLEAR_AND_SELECT);
  }

  // remove proxy from AnimationScene (optional)
  vtkSMProxy* scene = this->GetAnimationScene(proxy->GetSession());
  if (scene)
  {
    vtkSMPropertyHelper(scene, "ViewModules").Remove(proxy);
    scene->UpdateVTKObjects();
  }

  // remove proxy from TimeKeeper.
  vtkSMProxy* timeKeeper = this->FindTimeKeeper(proxy->GetSession());
  vtkSMPropertyHelper(timeKeeper, "Views").Remove(proxy);
  timeKeeper->UpdateVTKObjects();

  // remove all representation proxies. Always unregister lights.
  const char* pnames[] = { "AdditionalLights", "Representations", "HiddenRepresentations", "Props",
    "HiddenProps", NULL };
  for (int index = 0; index == 0 || (unregister_representations && (pnames[index] != NULL));
       ++index)
  {
    vtkSMProperty* prop = proxy->GetProperty(pnames[index]);
    if (prop == NULL)
    {
      continue;
    }

    typedef std::vector<vtkWeakPointer<vtkSMProxy> > proxyvectortype;
    proxyvectortype reprs;

    vtkSMPropertyHelper helper(prop);
    for (unsigned int cc = 0, max = helper.GetNumberOfElements(); cc < max; ++cc)
    {
      reprs.push_back(helper.GetAsProxy(cc));
    }

    for (proxyvectortype::iterator iter = reprs.begin(), end = reprs.end(); iter != end; ++iter)
    {
      if (iter->GetPointer())
      {
        this->UnRegisterProxy(iter->GetPointer());
      }
    }
  }

  // unregister dependencies.
  this->UnRegisterDependencies(proxy);

  // this will remove both proxy-list-domain helpers and animation helpers.
  this->FinalizeProxy(proxy);

  pxm->UnRegisterProxy("views", proxyname.c_str(), proxy);
  return true;
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::RegisterRepresentationProxy(vtkSMProxy* proxy)
{
  if (!proxy)
  {
    return false;
  }

  // Register proxies created for proxy list domains.
  this->RegisterProxiesForProxyListDomains(proxy);

  // Register the proxy itself.
  proxy->GetSessionProxyManager()->RegisterProxy("representations", proxy);
  return true;
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::UnRegisterRepresentationProxy(vtkSMProxy* proxy)
{
  PREPARE_FOR_UNREGISTERING(proxy);

  vtkSMSessionProxyManager* pxm = proxy->GetSessionProxyManager();
  std::string groupname("representations");
  const char* _proxyname = pxm->GetProxyName(groupname.c_str(), proxy);
  if (_proxyname == NULL)
  {
    groupname = "scalar_bars";
    _proxyname = pxm->GetProxyName(groupname.c_str(), proxy);
    if (_proxyname == NULL)
    {
      return false;
    }
  }
  SM_SCOPED_TRACE(Delete).arg("proxy", proxy);
  const std::string proxyname(_proxyname);

  //---------------------------------------------------------------------------
  // remove the representation from any views.
  typedef std::vector<std::pair<vtkSMProxy*, vtkSMProperty*> > viewsvector;
  viewsvector views;
  for (unsigned int cc = 0, max = proxy->GetNumberOfConsumers(); cc < max; cc++)
  {
    vtkSMProxy* consumer = proxy->GetConsumerProxy(cc);
    consumer = consumer ? consumer->GetTrueParentProxy() : NULL;
    if (consumer && consumer->IsA("vtkSMViewProxy") && proxy->GetConsumerProperty(cc))
    {
      views.push_back(
        std::pair<vtkSMProxy*, vtkSMProperty*>(consumer, proxy->GetConsumerProperty(cc)));
    }
  }
  for (viewsvector::iterator iter = views.begin(), max = views.end(); iter != max; ++iter)
  {
    if (iter->first && iter->second)
    {
      vtkSMPropertyHelper(iter->second).Remove(proxy);
      iter->first->UpdateVTKObjects();
    }
  }

  // unregister dependencies.
  this->UnRegisterDependencies(proxy);

  // this will remove both proxy-list-domain helpers and animation helpers.
  this->FinalizeProxy(proxy);

  proxy->GetSessionProxyManager()->UnRegisterProxy(groupname.c_str(), proxyname.c_str(), proxy);
  return true;
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::RegisterColorTransferFunctionProxy(
  vtkSMProxy* proxy, const char* proxyname)
{
  if (!proxy)
  {
    return false;
  }

  // Register proxies created for proxy list domains.
  this->RegisterProxiesForProxyListDomains(proxy);

  proxy->GetSessionProxyManager()->RegisterProxy("lookup_tables", proxyname, proxy);
  return true;
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::RegisterOpacityTransferFunction(
  vtkSMProxy* proxy, const char* proxyname)
{
  if (!proxy)
  {
    return false;
  }

  // Register proxies created for proxy list domains.
  this->RegisterProxiesForProxyListDomains(proxy);

  proxy->GetSessionProxyManager()->RegisterProxy("piecewise_functions", proxyname, proxy);
  return true;
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::RegisterAnimationProxy(vtkSMProxy* proxy)
{
  if (!proxy)
  {
    return false;
  }

  // Register proxies created for proxy list domains.
  this->RegisterProxiesForProxyListDomains(proxy);

  proxy->GetSessionProxyManager()->RegisterProxy("animation", proxy);
  return true;
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::UnRegisterAnimationProxy(vtkSMProxy* proxy)
{
  PREPARE_FOR_UNREGISTERING(proxy);

  vtkSMSessionProxyManager* pxm = proxy->GetSessionProxyManager();
  const char* _proxyname = pxm->GetProxyName("animation", proxy);
  if (_proxyname == NULL)
  {
    return false;
  }
  SM_SCOPED_TRACE(Delete).arg("proxy", proxy);
  const std::string proxyname(_proxyname);

  //---------------------------------------------------------------------------
  // Animation proxies are typically added to some other animation proxy. We
  // need to remove it from that proxy.
  typedef std::pair<vtkWeakPointer<vtkSMProxy>, vtkWeakPointer<vtkSMProperty> > proxypairitemtype;
  typedef std::vector<proxypairitemtype> proxypairvectortype;
  proxypairvectortype consumers;
  for (unsigned int cc = 0, max = proxy->GetNumberOfConsumers(); cc < max; ++cc)
  {
    vtkSMProxy* consumer = proxy->GetConsumerProxy(cc);
    consumer = consumer ? consumer->GetTrueParentProxy() : NULL;
    if (proxy->GetConsumerProperty(cc) && consumer && consumer->GetXMLGroup() &&
      strcmp(consumer->GetXMLGroup(), "animation") == 0)
    {
      consumers.push_back(proxypairitemtype(consumer, proxy->GetConsumerProperty(cc)));
    }
  }
  for (proxypairvectortype::iterator iter = consumers.begin(), max = consumers.end(); iter != max;
       ++iter)
  {
    if (iter->first && iter->second)
    {
      vtkSMPropertyHelper(iter->second).Remove(proxy);
      iter->first->UpdateVTKObjects();
    }
  }

  //---------------------------------------------------------------------------
  // destroy keyframes, if there are any.
  typedef std::vector<vtkWeakPointer<vtkSMProxy> > proxyvectortype;
  proxyvectortype keyframes;
  if (vtkSMProperty* kfProperty = proxy->GetProperty("KeyFrames"))
  {
    vtkSMPropertyHelper helper(kfProperty);
    for (unsigned int cc = 0, max = helper.GetNumberOfElements(); cc < max; ++cc)
    {
      keyframes.push_back(helper.GetAsProxy(cc));
    }
  }

  // unregister dependencies.
  this->UnRegisterDependencies(proxy);

  // this will remove both proxy-list-domain helpers and animation helpers.
  this->FinalizeProxy(proxy);

  pxm->UnRegisterProxy("animation", proxyname.c_str(), proxy);

  // now destroy all keyframe proxies.
  for (proxyvectortype::iterator iter = keyframes.begin(), max = keyframes.end(); iter != max;
       ++iter)
  {
    if (iter->GetPointer())
    {
      this->UnRegisterProxy(iter->GetPointer());
    }
  }
  return true;
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::RegisterLightProxy(
  vtkSMProxy* proxy, vtkSMProxy* view, const char* proxyname)
{
  if (!proxy)
  {
    return false;
  }

  SM_SCOPED_TRACE(RegisterLightProxy).arg("proxy", proxy).arg("view", view);

  // Register proxies created for proxy list domains.
  this->RegisterProxiesForProxyListDomains(proxy);

  // we would like the light to be a child of the view, but lights need to be created
  // and registered independently, before the view is created in python state files.
  // pxm->RegisterProxy(controller->GetHelperProxyGroupName(view), "lights", proxy);

  // Register the proxy itself.
  proxy->GetSessionProxyManager()->RegisterProxy("additional_lights", proxyname, proxy);
  return true;
}

//----------------------------------------------------------------------------
void vtkSMVisocytePipelineController::UpdateSettingsProxies(vtkSMSession* session)
{
  // Set up the settings proxies
  vtkSMSessionProxyManager* pxm = session->GetSessionProxyManager();
  vtkSMProxyDefinitionManager* pdm = pxm->GetProxyDefinitionManager();
  vtkPVProxyDefinitionIterator* iter = pdm->NewSingleGroupIterator("settings");
  for (iter->GoToFirstItem(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
  {
    vtkSMProxy* proxy = pxm->GetProxy(iter->GetGroupName(), iter->GetProxyName());
    if (!proxy)
    {
      proxy = vtkSafeNewProxy(pxm, iter->GetGroupName(), iter->GetProxyName());
      if (proxy)
      {
        this->InitializeProxy(proxy);
        pxm->RegisterProxy(iter->GetGroupName(), iter->GetProxyName(), proxy);
        proxy->UpdateVTKObjects();
        proxy->Delete();
      }
    }
  }
  iter->Delete();
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::PreInitializeProxy(vtkSMProxy* proxy)
{
  if (!proxy)
  {
    return false;
  }

  // 1. Load XML defaults
  //    (already done by NewProxy() call).

  // 2. Create proxies for proxy-list domains.
  if (!this->CreateProxiesForProxyListDomains(proxy))
  {
    return false;
  }

  // 3. Process global properties hints.
  if (!this->SetupGlobalPropertiesLinks(proxy))
  {
    return false;
  }

  // 4. Load property values from Settings.
  vtkSMSettings* settings = vtkSMSettings::GetInstance();
  settings->GetProxySettings(proxy);

  // Now, update the initialization time.
  this->Internals->InitializationTimeStamps[proxy].Modified();

  // Note: we need to be careful with *** vtkSMCompoundSourceProxy ***
  return true;
}

//----------------------------------------------------------------------------
vtkMTimeType vtkSMVisocytePipelineController::GetInitializationTime(vtkSMProxy* proxy)
{
  vtkInternals::TimeStampsMap::iterator titer =
    this->Internals->InitializationTimeStamps.find(proxy);
  return (titer == this->Internals->InitializationTimeStamps.end() ? 0 : titer->second);
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::PostInitializeProxy(vtkSMProxy* proxy)
{
  if (!proxy)
  {
    return false;
  }

  vtkInternals::TimeStampsMap::iterator titer =
    this->Internals->InitializationTimeStamps.find(proxy);
  if (titer == this->Internals->InitializationTimeStamps.end())
  {
    vtkErrorMacro("PostInitializeProxy must only be called after PreInitializeProxy");
    return false;
  }

  assert(titer != this->Internals->InitializationTimeStamps.end());

  vtkTimeStamp ts = titer->second;
  this->Internals->InitializationTimeStamps.erase(titer);

  // ensure everything is up-to-date.
  proxy->UpdateVTKObjects();

  // If InitializationHelper is specified for the proxy. Use it
  // for extra initialization.
  this->ProcessInitializationHelper(proxy, ts);

  // FIXME: need to figure out how we should really deal with this.
  // We don't reset properties on custom filter.
  if (!proxy->IsA("vtkSMCompoundSourceProxy"))
  {
    if (vtkSMSourceProxy* sourceProxy = vtkSMSourceProxy::SafeDownCast(proxy))
    {
      // Since domains depend on information properties, it's essential we update
      // property information first.
      if (sourceProxy->GetNumberOfOutputPorts() > 0)
      {
        // This is only done for non-writers, or non-sinks.
        sourceProxy->UpdatePipelineInformation();
      }
    }

    // Reset property values using domains. However, this should only be done for
    // properties that were not modified between the PreInitializePipelineProxy and
    // PostInitializePipelineProxy calls.

    vtkSmartPointer<vtkSMPropertyIterator> iter;
    iter.TakeReference(proxy->NewPropertyIterator());

    // iterate over properties and reset them to default. if any property says its
    // domain is modified after we reset it, we need to reset it again since its
    // default may have changed.
    vtkDomainObserver observer;

    for (iter->Begin(); !iter->IsAtEnd(); iter->Next())
    {
      vtkSMProperty* smproperty = iter->GetProperty();

      if ((smproperty->GetMTime() > ts) || smproperty->GetInformationOnly() ||
        // doesn't make sense to set property values for internal properties.
        smproperty->GetIsInternal())
      {
        // Property was modified between since the PreInitializeProxy() call. We
        // leave it untouched.
        continue;
      }

      vtkPVXMLElement* propHints = smproperty->GetHints();
      if (propHints && propHints->FindNestedElementByName("NoDefault"))
      {
        // Don't reset properties that request overriding of the default mechanism.
        continue;
      }

      observer.Monitor(iter->GetProperty());
      iter->GetProperty()->ResetToDomainDefaults();
    }

    const std::set<vtkSMProperty*>& props = observer.GetPropertiesWithModifiedDomains();
    for (std::set<vtkSMProperty*>::const_iterator siter = props.begin(); siter != props.end();
         ++siter)
    {
      (*siter)->ResetToDomainDefaults();
    }
    proxy->UpdateVTKObjects();
  }
  return true;
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::FinalizeProxy(vtkSMProxy* proxy)
{
  if (!proxy)
  {
    return false;
  }

  //---------------------------------------------------------------------------
  // Unregister all helper proxies. This will "unregister" all the helper
  // proxies which ensures that any "dependent" proxies for those helpers
  // also get removed.
  std::string groupname = this->GetHelperProxyGroupName(proxy);

  typedef std::pair<std::string, vtkWeakPointer<vtkSMProxy> > proxymapitemtype;
  typedef std::vector<proxymapitemtype> proxymaptype;
  proxymaptype proxymap;

  vtkSMSessionProxyManager* pxm = proxy->GetSessionProxyManager();
  {
    vtkNew<vtkSMProxyIterator> iter;
    iter->SetSessionProxyManager(pxm);
    iter->SetModeToOneGroup();
    for (iter->Begin(groupname.c_str()); !iter->IsAtEnd(); iter->Next())
    {
      proxymap.push_back(proxymapitemtype(iter->GetKey(), iter->GetProxy()));
    }
  }

  for (proxymaptype::iterator iter = proxymap.begin(), end = proxymap.end(); iter != end; ++iter)
  {
    if (iter->second)
    {
      // remove any dependencies for this proxy.
      this->UnRegisterDependencies(iter->second);
      // remove any helpers for this proxy.
      this->FinalizeProxy(iter->second);
      // unregister the proxy.
      pxm->UnRegisterProxy(groupname.c_str(), iter->first.c_str(), iter->second);
    }
  }

  return true;
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::UnRegisterDependencies(vtkSMProxy* proxy)
{
  assert(proxy != NULL);

  //---------------------------------------------------------------------------
  // Before going any further build a list of all consumer proxies
  // (not pointing to internal/sub-proxies).

  typedef std::vector<vtkWeakPointer<vtkSMProxy> > proxyvectortype;
  proxyvectortype consumers;

  for (unsigned int cc = 0, max = proxy->GetNumberOfConsumers(); cc < max; ++cc)
  {
    vtkSMProxy* consumer = proxy->GetConsumerProxy(cc);
    consumer = consumer ? consumer->GetTrueParentProxy() : NULL;
    if (consumer)
    {
      consumers.push_back(consumer);
    }
  }

  //---------------------------------------------------------------------------
  for (proxyvectortype::iterator iter = consumers.begin(), max = consumers.end(); iter != max;
       ++iter)
  {
    vtkSMProxy* consumer = iter->GetPointer();
    if (!consumer)
    {
      continue;
    }

    if (
      // Remove representations connected to this proxy, if any.
      consumer->IsA("vtkSMRepresentationProxy") ||

      // Remove any animation cues for this proxy
      strcmp(consumer->GetXMLGroup(), "animation") == 0 ||

      false)
    {
      this->UnRegisterProxy(consumer);
    }
  }
  //---------------------------------------------------------------------------

  // TODO: remove any property links/proxy link setup for this proxy.

  return true;
}

//----------------------------------------------------------------------------
bool vtkSMVisocytePipelineController::UnRegisterProxy(vtkSMProxy* proxy)
{
  if (!proxy)
  {
    return false;
  }

  SM_SCOPED_TRACE(CleanupAccessor).arg("proxy", proxy);

  // determine what type of proxy is this, based on that, we can finalize it.
  vtkSMSessionProxyManager* pxm = proxy->GetSessionProxyManager();
  if (pxm->GetProxyName("sources", proxy))
  {
    return this->UnRegisterPipelineProxy(proxy);
  }
  else if (pxm->GetProxyName("representations", proxy) || pxm->GetProxyName("scalar_bars", proxy))
  {
    return this->UnRegisterRepresentationProxy(proxy);
  }
  else if (pxm->GetProxyName("views", proxy))
  {
    return this->UnRegisterViewProxy(proxy);
  }
  else if (pxm->GetProxyName("animation", proxy))
  {
    if (proxy != this->GetAnimationScene(proxy->GetSession()))
    {
      return this->UnRegisterAnimationProxy(proxy);
    }
  }
  else
  {
    PREPARE_FOR_UNREGISTERING(proxy);

    const char* known_groups[] = { "lookup_tables", "piecewise_functions", "layouts",
      "additional_lights", NULL };
    for (int cc = 0; known_groups[cc] != NULL; ++cc)
    {
      if (const char* pname = pxm->GetProxyName(known_groups[cc], proxy))
      {
        SM_SCOPED_TRACE(Delete).arg("proxy", proxy);

        // unregister dependencies.
        if (!this->UnRegisterDependencies(proxy))
        {
          return false;
        }

        // this will remove both proxy-list-domain helpers and animation helpers.
        if (!this->FinalizeProxy(proxy))
        {
          return false;
        }
        pxm->UnRegisterProxy(known_groups[cc], pname, proxy);
        return true;
      }
    }
  }
  return false;
}

//----------------------------------------------------------------------------
void vtkSMVisocytePipelineController::ProcessInitializationHelper(
  vtkSMProxy* proxy, vtkMTimeType initializationTimeStamp)
{
  vtkPVXMLElement* hints = proxy->GetHints();
  for (unsigned int cc = 0, max = (hints ? hints->GetNumberOfNestedElements() : 0); cc < max; ++cc)
  {
    vtkPVXMLElement* child = hints->GetNestedElement(cc);
    if (child && strcmp(child->GetName(), "InitializationHelper") == 0 &&
      child->GetAttribute("class") != NULL)
    {
      const char* className = child->GetAttribute("class");
      vtkSmartPointer<vtkObject> obj;
      obj.TakeReference(vtkPVInstantiator::CreateInstance(className));
      if (vtkSMProxyInitializationHelper* helper =
            vtkSMProxyInitializationHelper::SafeDownCast(obj))
      {
        helper->PostInitializeProxy(proxy, child, initializationTimeStamp);
      }
    }
  }
}

//----------------------------------------------------------------------------
void vtkSMVisocytePipelineController::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
