/*=========================================================================

  Program:   Visocyte
  Module:    vtkSMVisocytePipelineControllerWithRendering.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.visocyte.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkSMVisocytePipelineControllerWithRendering
 *
 * vtkSMVisocytePipelineControllerWithRendering overrides
 * vtkSMVisocytePipelineController to add support for initializing rendering
 * proxies appropriately. vtkSMVisocytePipelineControllerWithRendering uses
 * vtkObjectFactory mechanisms to override vtkSMVisocytePipelineController's
 * creation. One should not need to create or use this class directly (excepting
 * when needing to subclass). Simply create vtkSMVisocytePipelineController. If
 * the application is linked with the rendering module, then this class will be
 * instantiated instead of vtkSMVisocytePipelineController automatically.
 *
 * vtkSMVisocytePipelineControllerWithRendering also adds new API to control
 * representation visibility and manage creation of views. To use that API
 * clients can instantiate vtkSMVisocytePipelineControllerWithRendering. Just
 * like vtkSMVisocytePipelineController, this class also uses vtkObjectFactory
 * mechanisms to enable overriding by clients at compile time.
*/

#ifndef vtkSMVisocytePipelineControllerWithRendering_h
#define vtkSMVisocytePipelineControllerWithRendering_h

#include "vtkPVServerManagerRenderingModule.h" //needed for exports
#include "vtkSMVisocytePipelineController.h"

class vtkSMSourceProxy;
class vtkSMViewLayoutProxy;
class vtkSMViewProxy;

class VTKPVSERVERMANAGERRENDERING_EXPORT vtkSMVisocytePipelineControllerWithRendering
  : public vtkSMVisocytePipelineController
{
public:
  static vtkSMVisocytePipelineControllerWithRendering* New();
  vtkTypeMacro(vtkSMVisocytePipelineControllerWithRendering, vtkSMVisocytePipelineController);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Show the output data in the view. If data cannot be shown in the view,
   * returns NULL. If \c view is NULL, this simply calls ShowInPreferredView().
   */
  virtual vtkSMProxy* Show(vtkSMSourceProxy* producer, int outputPort, vtkSMViewProxy* view);

  /**
   * Show all source output ports in provided view.
   */
  void ShowAll(vtkSMViewProxy* view);

  /**
   * Opposite of Show(). Locates the representation for the producer and then
   * hides it, if found. Returns that representation, if found.
   */
  virtual vtkSMProxy* Hide(vtkSMSourceProxy* producer, int outputPort, vtkSMViewProxy* view);

  /**
   * Same as above, except that when we already have the representation located.
   */
  virtual void Hide(vtkSMProxy* repr, vtkSMViewProxy* view);

  /**
   * Hide all visible sources output ports in provided view
   */
  virtual void HideAll(vtkSMViewProxy* view);

  /**
   * Alternative method to call Show and Hide using a visibility flag.
   */
  vtkSMProxy* SetVisibility(
    vtkSMSourceProxy* producer, int outputPort, vtkSMViewProxy* view, bool visible)
  {
    return (
      visible ? this->Show(producer, outputPort, view) : this->Hide(producer, outputPort, view));
  }

  /**
   * Returns whether the producer/port are shown in the given view.
   */
  virtual bool GetVisibility(vtkSMSourceProxy* producer, int outputPort, vtkSMViewProxy* view);

  /**
   * Same as Show() except that if the \c view is NULL or not the preferred
   * view for the producer's output, this method will create a new view and show
   * the data in that new view.
   *
   * There are several different strategies employed to determine the preferred
   * view for the producer's output. See
   * vtkSMVisocytePipelineControllerWithRendering::GetPreferredViewType() for
   * details.
   *
   * @note if the source's hint indicates so, the data may also be
   *       shown in the \c view passed in, in addition to the preferred view.
   *       This is done by using the `also_show_in_current_view` attribute to
   *       the `<View />` hint.
   *
   * @returns the view in which the data ends up being shown, if any.
   *          It may return nullptr if the \c view is not the preferred type
   *          or the preferred cannot be determined or created.
   */
  virtual vtkSMViewProxy* ShowInPreferredView(
    vtkSMSourceProxy* producer, int outputPort, vtkSMViewProxy* view);

  /**
   * Returns the name for the preferred view type, if there is any. There are
   * several strategies employed by the default implementation to determine the
   * preferred view type.
   *
   * -# Using XML hints.\n
   *    A producer proxy can provide XML hints to define the preferred view type
   *    of each (or all) of its output ports. This is done as follows:
   *
   *    @code{xml}
   *      <SourceProxy>
   *        <Hints>
   *          <View type="<view name>" port="<output port number" />
   *        </Hints>
   *      </SourceProxy>
   *    @endcode
   *
   *    Attribute `port` is optional and only needed to explicitly specify
   *    different view types for different output ports.
   *
   * -# Using data type.\n
   *    If the data type for the generated data is `vtkTable`, then the
   *    preferred view (if none provided) is assumed to be `SpreadSheetView`.
   *
   * @returns XML name for the preferred view proxy. It is assumed to be defined
   *          in the "views" group.
   */
  virtual const char* GetPreferredViewType(vtkSMSourceProxy* producer, int outputPort);

  /**
   * Return the pipeline icon to use with the provided producer.
   * It can be either a view type name, an existing icon resource or nullptr.
   * Here is the strategy to determine the pipeline icon.
   *
   * # Using XML hints.
   *    A producer proxy can provide XML hints to define the pipeline icon to use
   *    of each (or all) of its output ports. This is done as follows:
   *
   *    @code{xml}
   *      <SourceProxy>
   *        <Hints>
   *          <PipelineIcon name="<view name or icon resource>" port="<output port number>" />
   *        </Hints>
   *      </SourceProxy>
   *    @endcode
   *
   *    Attribute `port` is optional and only needed to explicitly specify
   *    different view types for different output ports.
   *
   * # Using GetPreferredViewType
   *    If no PipelineIcon as been provided, we fall back to using
   *    GetPreferredViewType.
   */
  virtual const char* GetPipelineIcon(vtkSMSourceProxy* producer, int outputPort);

  /**
   * Overridden to create color and opacity transfer functions if applicable.
   * While it is tempting to add any default property setup logic in such
   * overrides, we must keep such overrides to a minimal and opting for domains
   * that set appropriate defaults where as much as possible.
   */
  bool RegisterRepresentationProxy(vtkSMProxy* proxy) override;

  /**
   * Control how scalar bar visibility is updated by the Hide call.
   */
  static void SetHideScalarBarOnHide(bool);

  //@{
  /**
   * Control whether representations try to maintain properties from an input
   * representation, if present. e.g. if you "Transform" the representation for
   * a source, then any filter that you connect to it should be transformed as
   * well.
   */
  static void SetInheritRepresentationProperties(bool);
  static bool GetInheritRepresentationProperties();
  //@}

  /**
   * Overridden to handle default ColorArrayName for representations correctly.
   */
  bool PostInitializeProxy(vtkSMProxy* proxy) override;

  //@{
  /**
   * Overridden to place the view in a layout on creation.
   */
  bool RegisterViewProxy(vtkSMProxy* proxy, const char* proxyname) override;
  using Superclass::RegisterViewProxy;
  //@}

  /**
   * Register layout proxy.
   */
  virtual bool RegisterLayoutProxy(vtkSMProxy* proxy, const char* proxyname = NULL);

protected:
  vtkSMVisocytePipelineControllerWithRendering();
  ~vtkSMVisocytePipelineControllerWithRendering() override;

  virtual void UpdatePipelineBeforeDisplay(
    vtkSMSourceProxy* producer, int outputPort, vtkSMViewProxy* view);

  /**
   * Checks if the output from producer needs to be shown in the current view
   * also.
   */
  virtual bool AlsoShowInCurrentView(
    vtkSMSourceProxy* producer, int outputPort, vtkSMViewProxy* currentView);

  /**
   * Overridden here where the library has link access to rendering classes.
   */
  virtual void DoMaterialSetup(vtkSMProxy* proxy) override;

private:
  vtkSMVisocytePipelineControllerWithRendering(
    const vtkSMVisocytePipelineControllerWithRendering&) = delete;
  void operator=(const vtkSMVisocytePipelineControllerWithRendering&) = delete;
  static bool HideScalarBarOnHide;
  static bool InheritRepresentationProperties;
};

#endif
