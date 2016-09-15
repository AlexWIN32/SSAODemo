/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <GUI/Theme.h>
#include <GUI/Control.h>
#include <CommonParams.h>
#include <Xml.h>
#include <sstream>
#include <Serializing.h>

namespace GUI
{

class Cursor
{
private:
    std::string themeName;
    std::string controlName;
    std::string elementName;
public:
    void SetThemeName(const std::string &ThemeName){themeName = ThemeName;}
    void SetControlName(const std::string &ControlName){controlName = ControlName;}
    void SetElementName(const std::string &ElementName){elementName = ElementName;}
    ThemeException CreateException(const std::string &Text)
    {
        std::stringstream sstrm;
        sstrm << "theme " << themeName;
        if(controlName != "")
            sstrm << ", control " << controlName;
        if(elementName != "")
            sstrm << ", element " << elementName;

        sstrm << ": " << Text;

        return ThemeException(sstrm.str());
    }
};

struct TextureSize
{
    FLOAT widthOverOne, heightOverOne;
};

typedef std::map<std::string, TextureSize> TextureSizesStorage;

template<class TElement>
TElement ParseElement(const XML::Node &DataNode, const TextureSizesStorage &TextureSizes, Cursor &cursor)
{
    const std::string &texFileName = DataNode.GetProperty("textureFile");

    TextureSizesStorage::const_iterator tsCi = TextureSizes.find(texFileName);
    if(tsCi == TextureSizes.end())
        throw cursor.CreateException(texFileName + " texture not declared");

    const TextureSize &texSize = tsCi->second;

    SizeF screenSize = SizeFParser::FromString(DataNode.GetProperty("screenSize"));

    Point2F upperLeftPos = Point2FParser::FromString(DataNode.GetProperty("upperLeft"));
    Point2F lowerRightPos = (upperLeftPos + Cast<Point2F>(screenSize));

    TElement newElement;
    newElement.textureFileName = texFileName;
    newElement.textureBounds.x = upperLeftPos.x * texSize.widthOverOne;
    newElement.textureBounds.y = upperLeftPos.y * texSize.heightOverOne;
    newElement.textureBounds.z = lowerRightPos.x * texSize.widthOverOne;
    newElement.textureBounds.w = lowerRightPos.y * texSize.heightOverOne;
    newElement.screenSize.x = Control::XToWigth(screenSize.width / CommonParams::GetScreenWidth());
    newElement.screenSize.y = screenSize.height / CommonParams::GetScreenHeight();
    return newElement;
}


const ControlView::ElementData &ControlView::GetElement(const std::string &ElementName) const throw (Exception)
{
    ElementsStorage::const_iterator ci = elements.find(ElementName);
    if(ci == elements.end())
        throw new ControlViewElementNotFoundException("element " + ElementName + " not found in " + name + " control view");

    return ci->second;
}

BOOL ControlView::FindElement(const std::string &ElementName, ControlView::ElementData &FoundElement) const
{
    ElementsStorage::const_iterator ci = elements.find(ElementName);
    if(ci == elements.end())
        return false;

    FoundElement = ci->second;
    return true;
}

const std::string &ControlView::GetParameter(const std::string &ParameterName) const throw (Exception)
{
    ParamsStorage::const_iterator ci = params.find(ParameterName);
    if(ci == params.end())
        throw new ControlViewElementNotFoundException("parameter " + ParameterName + " not found in " + name + " control view");

    return ci->second;
}

void ControlView::SetParameter(const std::string &ParameterName, const std::string &ParameterValue)
{
    params[ParameterName] = ParameterValue;
}

void ControlView::RemoveParameter(const std::string &ParameterName)
{
    ParamsStorage::iterator it = params.begin();
    if(it != params.end())
        params.erase(ParameterName);
}

BOOL ControlView::FindParameter(const std::string &ParameterName, std::string &value) const
{
    ParamsStorage::const_iterator ci = params.find(ParameterName);
    if(ci == params.end())
        return false;

    value = ci->second;
    return true;
}

const ControlView &Theme::GetControlView(const std::string &Name) const throw (Exception)
{
    ControlViewsStorage::const_iterator ci = controlViews.find(Name);
    if(ci == controlViews.end())
        throw ControlViewNotFoundException(Name +" not found in " + name + " theme");

    return ci->second;
}

const Font &Theme::GetFont(const std::string &Name) const throw (Exception)
{
    std::string fontName = Name;
    if(Name == ""){
        if(defaultFontName == "")
            throw FontNotFoundException("default font not set");
        fontName = defaultFontName;
    }

    auto ci = fonts.find(fontName);
    if(ci != fonts.end())
        return ci->second;
        
    throw FontNotFoundException("font "+ fontName +" not found in " + name + " theme");
}

BOOL Theme::FindControlView(const std::string &Name, ControlView &FindedControlView)
{
    ControlViewsStorage::const_iterator ci = controlViews.find(Name);
    if(ci == controlViews.end())
        return false;

    FindedControlView = ci->second;
    return true;
}

void Theme::SetDefaultFont(const std::string &FontName) throw (Exception)
{    
    if(fonts.find(FontName) == fonts.end())
        throw FontNotFoundException("font "+ FontName +" not found in " + name + " theme");

    defaultFontName = FontName;
}

void Theme::Load(const std::string &Name) throw (Exception)
{
    TextureSizesStorage textureSizes;

    Cursor cursor;

    name = Name.substr(0, Name.find('.'));

    cursor.SetThemeName(Name);

    XML::XmlData data;
    data.LoadFromFile("../Resources/GuiThemes/" + Name);

    const XML::Node &guiNode = data.GetRoot();
    
    const XML::Node &texturesNode = guiNode.GetNode("textures");

    for(const XML::Node &textureNode : texturesNode){

        SizeF texSize = SizeFParser::FromString(textureNode.GetProperty("size"));

        TextureSize newTexSize;
        newTexSize.widthOverOne = 1.0f / texSize.width;
        newTexSize.heightOverOne = 1.0f / texSize.height;

        const std::string &texName = textureNode.GetProperty("fileName");

        textureSizes.insert(std::make_pair(texName, newTexSize));
    }

    const XML::Node &controlsNode = guiNode.GetNode("controls");

    for(const XML::NodesNamesData &controlNodesData : controlsNode.GetNodesNames()){
        cursor.SetControlName(controlNodesData.name);

        if(controlNodesData.count != 1)
            throw cursor.CreateException("control redifinition");

        ControlView newControlView;

        const XML::Node &controlNode = controlsNode.GetNode(controlNodesData.name);
        for(const XML::Node &dataNode : controlNode){
            cursor.SetElementName(dataNode.GetName());

            if(controlsNode.GetNodesCount(dataNode.GetName()) > 1)
                throw cursor.CreateException("element redifinition");

             if(dataNode.GetName() == "params"){
                for(const XML::Node &paramDataNode : dataNode){
                    cursor.SetElementName(dataNode.GetName());

                    if(dataNode.GetNodesCount(paramDataNode.GetName()) > 1)
                        throw cursor.CreateException("element redifinition");

                    newControlView.params.insert({paramDataNode.GetName(), paramDataNode.GetValue()});
                }
             }else
                newControlView.elements.insert({dataNode.GetName(), ParseElement<ControlView::ElementData>(dataNode, textureSizes, cursor)});
        }

        cursor.SetElementName("");

        controlViews.insert(std::make_pair(controlNode.GetName(), newControlView));
    }

    const XML::Node &fntsNode = guiNode.GetNode("fonts");
    
    for(const XML::Node &fntNode : fntsNode){

        Font newFont;
        newFont.Init(fntNode.GetProperty("name"));
        fonts.insert({newFont.GetName(), newFont});
    }

    if(fonts.size() > 1)
        defaultFontName = fntsNode.GetNode("defaultFont").GetProperty("name");
    else
        defaultFontName = fonts.begin()->first;

    XML::ConstNodesSet imagesNodes;
    if(guiNode.FindNode("images", imagesNodes, false)){
        if(imagesNodes.size() > 1)
            throw cursor.CreateException("images node redifinition");

        for(const XML::Node &node : **imagesNodes.begin())
            images.insert({node.GetName(), ParseElement<ImageData>(node, textureSizes, cursor)});
    }


}

const ImageData &Theme::GetImageData(const std::string &ImageName) const throw (Exception)
{
    ImagesStorage::const_iterator ci = images.find(ImageName);
    if(ci == images.end())
        throw ImageDataNotFoundException("Image data " + ImageName + " not found in " + name + " theme");

    return ci->second;
}

BOOL Theme::FindImageData(const std::string &ImageName, ImageData &FoundImageData) const
{
    ImagesStorage::const_iterator ci = images.find(ImageName);
    if(ci == images.end())
        return false;

    FoundImageData = ci->second;
    return true;
}

}