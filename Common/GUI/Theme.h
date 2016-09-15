/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <D3DHeaders.h>
#include <string>
#include <map>
#include <Exception.h>
#include <GUI/Font.h>

namespace GUI
{

DECLARE_EXCEPTION(ThemeException);
DECLARE_CHILD_EXCEPTION(ControlViewElementNotFoundException, ThemeException)
DECLARE_CHILD_EXCEPTION(ControlViewParameterNotFoundException, ThemeException)
DECLARE_CHILD_EXCEPTION(ControlViewNotFoundException, ThemeException)
DECLARE_CHILD_EXCEPTION(FontNotFoundException, ThemeException)
DECLARE_CHILD_EXCEPTION(ImageDataNotFoundException, ThemeException)

class Theme;
class ControlView
{
friend class Theme;
public:
    struct ElementData
    {
        std::string textureFileName;
        D3DXVECTOR4 textureBounds;
        D3DXVECTOR2 screenSize;
    };
    typedef std::map<std::string, ElementData> ElementsStorage;
    typedef std::map<std::string, std::string> ParamsStorage;
private:
    ParamsStorage params;
    ElementsStorage elements;
    std::string name;
public:
    ControlView(){}
    const ElementsStorage &GetElements() const {return elements;}
    const ParamsStorage &GetParameters() const {return params;}
    const std::string &GetName() const {return name;}
    const ElementData &GetElement(const std::string &ElementName) const throw (Exception);
    BOOL FindElement(const std::string &ElementName, ElementData &FoundElement) const;
    const std::string &GetParameter(const std::string &ParameterName) const throw (Exception);
    void SetParameter(const std::string &ParameterName, const std::string &ParameterValue);
    BOOL FindParameter(const std::string &ParameterName, std::string &value) const;
    void RemoveParameter(const std::string &ParameterName);
};

struct ImageData
{
    std::string textureFileName;
    D3DXVECTOR4 textureBounds;
    D3DXVECTOR2 screenSize;
};

class Theme
{
private:
    typedef std::map<std::string, ControlView> ControlViewsStorage;
    ControlViewsStorage controlViews;
    std::string name;
    typedef std::map<std::string, Font> FontsStorage;
    FontsStorage fonts;
    std::string defaultFontName;
    typedef std::map<std::string, ImageData> ImagesStorage;
    ImagesStorage images;
public:
    typedef ControlViewsStorage::const_iterator ConstIterator;
    Theme(){}
    ConstIterator BeginControlViews() const {return controlViews.begin();}
    ConstIterator EndControlViews() const {return controlViews.end();}
    const Font &GetFont(const std::string &Name = "") const throw (Exception);
    const std::string &GetName() const {return name;}
    const std::string &GetDefaultFontName() const {return defaultFontName;}
    void SetDefaultFont(const std::string &FontName) throw (Exception);
    const ControlView &GetControlView(const std::string &Name) const throw (Exception);
    BOOL FindControlView(const std::string &Name, ControlView &FindedControlView);    
    const ImageData &GetImageData(const std::string &ImageName) const throw (Exception);
    BOOL FindImageData(const std::string &ImageName, ImageData &FoundImageData) const; 
    void Load(const std::string &Name) throw (Exception);
};

}